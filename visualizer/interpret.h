#pragma once

#include "ast.h"
#include "symbol_linking.h"
#include "translate.h"

#include <functional>
#include <cstring>
#include <queue>
#include <unordered_set>
#include <utility>

enum class ValueKind {
	NUMBER,
	PROCEDURE
};

struct Value {
	ValueKind kind;
	union {
		number_t number;
		AProcedure proc;
	};

	explicit Value(AProcedure proc, bool is_procedure) : kind(ValueKind::PROCEDURE), proc(proc) {}
	explicit Value(number_t number) : kind(ValueKind::NUMBER), number(number) {}
	Value() : Value(0) {}
	Value(const Value& value) {
		memcpy(this, &value, sizeof(Value));
	}
	const Value& operator=(const Value& value) {
		return * new (this) Value(value);
	}
	~Value() {}
};

struct State {
	AProcedure proc;
	number_t time;
	number_t x,y;
	number_t size;
	number_t direction;
	number_t tint;
	number_t seed;
	std::vector<Value> stack;

	State() {}
	State(AProcedure proc, State& parent, std::vector<Value> stack) : proc(proc), stack(std::move(stack)) {
		time = parent.time;
		x = parent.x;
		y = parent.y;
		size = parent.size;
		direction = parent.direction;
		tint = parent.tint;
		seed = parent.seed;
	}

	State(State&& state) = default;
	State& operator=(State&& state) = default;
};

class Interpreter : private ReturningAdapter<Value> {
	SymbolLinking& sym;
	const char *filename;
	State state;
	std::queue<State> pending;
	std::vector<Plot> output;
	RoseStatistics *stats;
	bool forked_in_frame;

public:
	Interpreter(SymbolLinking& sym, const char *filename)
		: sym(sym), filename(filename), stats(nullptr) {}

	std::vector<Plot> interpret(AProcedure main, RoseStatistics *stats) {
		this->stats = stats;

		State initial;
		initial.proc = main;
		initial.time = MAKE_NUMBER(0);
		initial.x = MAKE_NUMBER(0);
		initial.y = MAKE_NUMBER(0);
		initial.size = MAKE_NUMBER(2);
		initial.direction = MAKE_NUMBER(0);
		initial.tint = MAKE_NUMBER(1);
		initial.seed = 0xBABEFEED;
		pending.push(std::move(initial));

		while (!pending.empty()) {
			state = std::move(pending.front());
			pending.pop();
			short f = NUMBER_TO_INT(state.time);
			if (f >= 0 && f < stats->frames) {
				cpu(140);
				forked_in_frame = false;
				state.proc.getBody().apply(*this);
				if (!forked_in_frame) {
					stats->frame[f].turtles_died++;
					cpu(40);
				}
			} else {
				int overwait = f - stats->frames;
				if (overwait > stats->max_overwait) stats->max_overwait = overwait;
			}
		}

		this->stats = nullptr;
		return output;
	}

	std::vector<TintColor> get_colors(AProgram program) {
		std::vector<TintColor> colors;
		number_t time = MAKE_NUMBER(0);
		for (PEvent event : program.getEvent()) {
			if (event.is<AColorEvent>()) {
				std::string color = event.cast<AColorEvent>().getColor().getText();
				short tint = 0;
				int i = 0;
				while (color[i] >= '0' && color[i] <= '9') {
					tint = tint * 10 + (color[i] - '0');
					i++;
				}
				short rgb = 0;
				for (int j = 0 ; j < 3 ; j++) {
					rgb *= 16;
					char h = color[i + 1 + j];
					if (h >= '0' && h <= '9') {
						rgb += h - '0';
					} else {
						rgb += 10 + (h & 0x5F) - 'A';
					}
				}
				colors.push_back({NUMBER_TO_INT(time), tint, rgb});
			} else if (event.is<AWaitEvent>()) {
				PExpression waitexp = event.cast<AWaitEvent>().getExpression();
				Value wait = apply(waitexp);
				time += wait.number;
			}
		}
		return colors;
	}

	bool get_resolution(AProgram program, int *width_out, int *height_out) {
		PResolution res = program.getResolution();
		if (res) {
			PExpression width_exp = res.cast<AResolution>().getWidth();
			Value width_value = apply(width_exp);
			PExpression height_exp = res.cast<AResolution>().getHeight();
			Value height_value = apply(height_exp);
			*width_out = NUMBER_TO_INT(width_value.number);
			*height_out = NUMBER_TO_INT(height_value.number);
			return true;
		} else {
			return false;
		}
	}

	bool get_layers(AProgram program, int *count_out, int *depth_out) {
		PLayers lay = program.getLayers();
		if (lay) {
			PExpression count_exp = lay.cast<ALayers>().getCount();
			Value count_value = apply(count_exp);
			PExpression depth_exp = lay.cast<ALayers>().getDepth();
			Value depth_value = apply(depth_exp);
			*count_out = NUMBER_TO_INT(count_value.number);
			*depth_out = NUMBER_TO_INT(depth_value.number);
			if (*count_out < 1) {
				throw new CompileException(lay.cast<ALayers>().getToken(), "Layer count must be at least 1");
			}
			if (*depth_out < 1) {
				throw new CompileException(lay.cast<ALayers>().getToken(), "Layer depth must be at least 1");
			}
			return true;
		} else {
			return false;
		}
	}

private:
	// Count CPU cycles
	void cpu(int cycles) {
		if (stats != nullptr) {
			short f = NUMBER_TO_INT(state.time);
			if (f >= 0 && f < stats->frames) {
				stats->frame[f].cpu_compute_cycles += cycles;
			}
		}
	}

	// Util
	int sin(int a) {
		int na = a & 8191;
		if (na == 4096) {
			return a & 8192 ? -16384 : 16384;
		}
		if (na > 4096) {
			na = 8192 - na;
		}
		int na2 = (na * na) >> 8;
		int r = (((((((2373 * na2) >> 16) - 21073) * na2) >> 16) + 51469) * na) >> 13;
		return a & 8192 ? -r : r;
	}

	number_t random_iteration(number_t v) {
		return ((v & 0xFFFF) * 0x9D3D) + ((v << 16) | ((v >> 16) & 0xFFFF));
	}

	// Expressions

	void caseABinaryExpression(ABinaryExpression exp) override {
		Value left = apply(exp.getLeft());
		Value right = apply(exp.getRight());
		std::function<number_t(number_t,number_t)> eval;
		Token token;
		PBinop op = exp.getOp();
		cpu(20);
		if (op.is<APlusBinop>()) {
			eval = [&](number_t a, number_t b) { return a + b; };
			token = op.cast<APlusBinop>().getPlus();
		} else if (op.is<AMinusBinop>()) {
			eval = [&](number_t a, number_t b) { return a - b; };
			token = op.cast<AMinusBinop>().getMinus();
		} else if (op.is<AMultiplyBinop>()) {
			token = op.cast<AMultiplyBinop>().getMul();
			eval = [&](number_t a, number_t b) {
				if (a >= 128 << 16 || a < -128 << 16) {
					sym.warning(token, "Left operand overflows");
				}
				if (b >= 128 << 16 || b < -128 << 16) {
					sym.warning(token, "Right operand overflows");
				}
				return (a << 8 >> 16) * (b << 8 >> 16);
			};
			cpu(126 - 20);
		} else if (op.is<ADivideBinop>()) {
			token = op.cast<ADivideBinop>().getDiv();
			eval = [&](number_t a, number_t b) {
				if (b >= 128 << 16 || b < -128 << 16) {
					sym.warning(token, "Right operand overflows");
				}
				int divisor = b << 8 >> 16;
				if (divisor == 0) {
					throw CompileException(token, "Division by zero");
				}
				int div_result = a / divisor;
				if (b >= 128 << 16 || b < -128 << 16) {
					sym.warning(token, "Result overflows");
				}
				return div_result << 8;
			};
			cpu(218 - 20);
		} else if (op.is<AEqBinop>()) {
			eval = [&](number_t a, number_t b) { return a == b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<AEqBinop>().getEq();
		} else if (op.is<ANeBinop>()) {
			eval = [&](number_t a, number_t b) { return a != b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<ANeBinop>().getNe();
		} else if (op.is<ALtBinop>()) {
			eval = [&](number_t a, number_t b) { return a < b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<ALtBinop>().getLt();
		} else if (op.is<ALeBinop>()) {
			eval = [&](number_t a, number_t b) { return a <= b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<ALeBinop>().getLe();
		} else if (op.is<AGtBinop>()) {
			eval = [&](number_t a, number_t b) { return a > b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<AGtBinop>().getGt();
		} else if (op.is<AGeBinop>()) {
			eval = [&](number_t a, number_t b) { return a >= b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
			token = op.cast<AGeBinop>().getGe();
		} else if (op.is<AAndBinop>()) {
			eval = [&](number_t a, number_t b) { return a & b; };
			token = op.cast<AAndBinop>().getAnd();
		} else if (op.is<AOrBinop>()) {
			eval = [&](number_t a, number_t b) { return a | b; };
			token = op.cast<AOrBinop>().getOr();
		}

		if (left.kind != ValueKind::NUMBER) {
			throw CompileException(token, "Left side of operation is not a number");
		}
		if (right.kind != ValueKind::NUMBER) {
			throw CompileException(token, "Right side of operation is not a number");
		}

		result = Value(eval(left.number, right.number));
	}

	void caseANegExpression(ANegExpression exp) override {
		Value inner = apply(exp.getExpression());
		if (inner.kind != ValueKind::NUMBER) {
			throw CompileException(exp.getToken(), "Operand of negation is not a number");
		}
		result = Value(-inner.number);
		cpu(4);
	}

	void caseASineExpression(ASineExpression exp) override {
		Value inner = apply(exp.getExpression());
		if (inner.kind != ValueKind::NUMBER) {
			throw CompileException(exp.getToken(), "Operand of sine is not a number");
		}
		result = Value(sin((inner.number & 0xffff) >> 2) << 2);
		cpu(42);
	}

	void caseARandExpression(ARandExpression exp) override {
		state.seed = random_iteration(state.seed);
		result = Value((state.seed >> 16) & 0xFFFF);
		cpu(12 + 144);
	}

	void caseAVarExpression(AVarExpression exp) override {
		VarRef ref = sym.var_ref[exp];
		switch (ref.kind) {
		case VarKind::GLOBAL:
			switch (static_cast<GlobalKind>(ref.index)) {
			case GlobalKind::X:
				result = Value(state.x);
				break;
			case GlobalKind::Y:
				result = Value(state.y);
				break;
			case GlobalKind::DIRECTION:
				result = Value(state.direction);
				break;
			}
			break;
		case VarKind::LOCAL:
			if (state.stack.size() <= ref.index) {
				throw CompileException(exp.getName(), "Internal error: Local index out of range");
			}
			result = state.stack[ref.index];
			break;
		case VarKind::PROCEDURE:
			result = Value(sym.procs[ref.index], true);
			break;
		}
		cpu(12 + 16);
	}

	void caseANumberExpression(ANumberExpression exp) override {
		result = Value(sym.literal_number[exp]);
		cpu(12 + 16);
	}

	void caseACondExpression(ACondExpression exp) override {
		Value cond = apply(exp.getCond());
		if (cond.kind != ValueKind::NUMBER) {
			throw CompileException(exp.getToken(), "Condition is not a number");
		}
		if (cond.number != 0) {
			result = apply(exp.getWhen());
			cpu(12 + 10);
		} else {
			result = apply(exp.getElse());
			cpu(10);
		}
	}

	// Statements

	void caseAWhenStatement(AWhenStatement s) override {
		Value cond = apply(s.getCond());
		if (cond.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Condition is not a number");
		}
		if (cond.number != 0) {
			s.getWhen().apply(*this);
			state.stack.resize(state.stack.size() - sym.when_pop[s]);
			cpu(12 + 10);
			if (sym.when_pop[s] != 0) cpu(8);
		} else {
			s.getElse().apply(*this);
			state.stack.resize(state.stack.size() - sym.else_pop[s]);
			cpu(10);
			if (sym.else_pop[s] != 0) cpu(8);
		}
	}

	void caseAForkStatement(AForkStatement s) override {
		Value proc = apply(s.getProc());
		if (proc.kind != ValueKind::PROCEDURE) {
			throw CompileException(s.getToken(), "Target is not a procedure");
		}
		int n_args = s.getArgs().size();
		int n_params = proc.proc.getParams().size();
		if (n_args != n_params) {
			throw CompileException(s.getToken(), "Wrong number of arguments for procedure " + proc.proc.getName().getText() + ": "
				+ std::to_string(n_args) + " given, " + std::to_string(n_params) + " expected");
		}
		std::vector<Value> args;
		for (auto a : s.getArgs()) {
			args.push_back(apply(a));
		}
		pending.emplace(proc.proc, state, std::move(args));
		forked_in_frame = true;
		if (proc.proc == state.proc) {
			// Assume tail fork. Negate dispatch overhead.
			cpu(20 + n_args * 28 - 140);
		} else {
			cpu(344 + n_args * 34); // TODO: Add 20 per wire
		}
	}

	void caseATempStatement(ATempStatement s) override {
		state.stack.push_back(apply(s.getExpression()));
	}

	void caseAWaitStatement(AWaitStatement s) override {
		Value wait = apply(s.getExpression());
		if (wait.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Wait value is not a number");
		}
		if (wait.number < 0) {
			sym.warning(s.getToken(), "Negative wait");
			return;
		}
		int frame = NUMBER_TO_INT(state.time);
		int new_frame = NUMBER_TO_INT(state.time + wait.number);
		while (frame < stats->frames && frame < new_frame) {
			stats->frame[frame++].turtles_survived++;
			forked_in_frame = false;
		}
		state.time += wait.number;
		cpu(146);
	}

	void caseATurnStatement(ATurnStatement s) override {
		Value turn = apply(s.getExpression());
		if (turn.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Turn value is not a number");
		}
		state.direction += turn.number;
		cpu(12 + 16 + 20 + 16);
	}

	void caseAFaceStatement(AFaceStatement s) override {
		Value face = apply(s.getExpression());
		if (face.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Face value is not a number");
		}
		state.direction = face.number;
		cpu(16);
	}

	void caseASizeStatement(ASizeStatement s) override {
		Value size = apply(s.getExpression());
		if (size.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Size is not a number");
		}
		state.size = size.number;
		cpu(16);
	}

	void caseATintStatement(ATintStatement s) override {
		Value tint = apply(s.getExpression());
		if (tint.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Tint is not a number");
		}
		state.tint = tint.number;
		cpu(16);
		short tint_int = NUMBER_TO_INT(tint.number);
		if (tint_int < 0) {
			sym.warning(s.getToken(), "Negative tint");
		} else if (tint_int >= stats->layer_count * stats->layer_depth) {
			sym.warning(s.getToken(), "Tint value outside range");
		}
	}

	void caseASeedStatement(ASeedStatement s) override {
		Value seed = apply(s.getExpression());
		if (seed.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Seed is not a number");
		}
		state.seed = random_iteration(random_iteration(seed.number));
		cpu(204);
	}

	void caseAMoveStatement(AMoveStatement s) override {
		Value move = apply(s.getExpression());
		if (move.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Move distance is not a number");
		}
		number_t m = move.number;
		int sa = sin(state.direction >> 10);
		int ca = sin((state.direction >> 10) + 4096);
		if (m < MAKE_NUMBER(32)) {
			// High precision move
			state.x += ((m << 10 >> 16) * ca) >> 8;
			state.y += ((m << 10 >> 16) * sa) >> 8;
			cpu(402);
		} else {
			// High distance move
			state.x += (m << 2 >> 16) * ca;
			state.y += (m << 2 >> 16) * sa;
			cpu(346);
		}
	}

	void caseAJumpStatement(AJumpStatement s) override {
		Value x = apply(s.getX());
		Value y = apply(s.getY());
		if (x.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "X is not a number");
		}
		if (y.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Y is not a number");
		}
		state.x = x.number;
		state.y = y.number;
		cpu(32);
	}

	void draw(short tint) {
		short f = NUMBER_TO_INT(state.time);
		if (f >= 0 && f < stats->frames) {
			short x = NUMBER_TO_INT(state.x);
			short y = NUMBER_TO_INT(state.y);
			short size = NUMBER_TO_INT(state.size);
			output.push_back({f, x, y, size, tint});
			stats->draw(f, x, y, size);
		}
	}

	void caseADrawStatement(ADrawStatement s) override {
		draw(NUMBER_TO_INT(state.tint));
	}

	void caseAPlotStatement(APlotStatement s) override {
		draw(~NUMBER_TO_INT(state.tint));
	}

};
