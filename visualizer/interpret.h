#pragma once

#include "ast.h"
#include "symbol_linking.h"
#include "translate.h"

#include <functional>
#include <cstring>
#include <queue>
#include <unordered_set>
#include <utility>

typedef uint64_t wire_mask_t;

enum class ValueKind {
	NUMBER,
	PROCEDURE
};

struct Value {
	ValueKind kind;
	union {
		number_t number;
		AProcDecl proc;
	};

	explicit Value(AProcDecl proc, bool is_procedure) : kind(ValueKind::PROCEDURE), proc(proc) {}
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
	AProcDecl proc;
	number_t time;
	number_t x,y;
	number_t size;
	number_t direction;
	number_t tint;
	number_t seed;
	std::vector<Value> stack;
	std::vector<Value> wire_values;
	wire_mask_t wires_set;
	std::vector<wire_mask_t> wires_written_since;

	State() {}
	State(AProcDecl proc, State& parent, std::vector<Value> stack)
	: proc(proc), stack(std::move(stack)), wire_values(parent.wire_values) {
		time = parent.time;
		x = parent.x;
		y = parent.y;
		size = parent.size;
		direction = parent.direction;
		tint = parent.tint;
		seed = parent.seed;
		wires_set = parent.wires_set;
		wires_written_since = parent.wires_written_since;
	}

	State(State&& state) = default;
	State& operator=(State&& state) = default;
};

class Interpreter : private ReturningAdapter<Value> {
	Reporter& rep;
	SymbolLinking& sym;
	State state;
	std::queue<State> pending;
	std::vector<Plot> output;
	RoseStatistics *stats;
	bool forked_in_frame;
	bool procedure_phase;

	// Temp state for color script calculation
	std::vector<TintColor> colors;
	number_t time;
	nodemap<bool> look_seen;
	std::vector<short> current_palette;
	std::vector<short> fade_palette;
	number_t fade_time;
	bool is_fading;

	TintColor parse_color(Token color) {
		const std::string& text = color.getText();
		short tint = 0;
		int i = 0;
		while (text[i] >= '0' && text[i] <= '9') {
			tint = tint * 10 + (text[i] - '0');
			i++;
		}
		short rgb = 0;
		for (int j = 0 ; j < 3 ; j++) {
			rgb *= 16;
			char h = text[i + 1 + j];
			if (h >= '0' && h <= '9') {
				rgb += h - '0';
			} else {
				rgb += 10 + (h & 0x5F) - 'A';
			}
		}
		return {0, tint, rgb};
	}

	void do_fade() {
		is_fading = false;
		while (fade_palette.size() < current_palette.size()) {
			fade_palette.push_back(0x000);
		}

		int t0 = NUMBER_TO_INT(fade_time);
		int t1 = NUMBER_TO_INT(time);
		if (t1 <= t0) {
			for (int i = 0; i < fade_palette.size(); i++) {
				if (current_palette[i] != fade_palette[i]) {
					colors.push_back({(short)t1, (short)i, current_palette[i]});
				}
			}
			return;
		}

		std::vector<short> pal = fade_palette;
		for (int t = t0 + 1; t <= t1; t++) {
			int w = (t - t0) * (1 << 16) / (t1 - t0);
			for (int i = 0; i < fade_palette.size(); i++) {
				short rgb0 = fade_palette[i];
				short rgb1 = current_palette[i];
				int r0 = rgb0 >> 8, g0 = (rgb0 >> 4) & 0xf, b0 = rgb0 & 0xf;
				int r1 = rgb1 >> 8, g1 = (rgb1 >> 4) & 0xf, b1 = rgb1 & 0xf;
				int r = r0 + (((r1 - r0) * w + (1 << 15)) >> 16);
				int g = g0 + (((g1 - g0) * w + (1 << 15)) >> 16);
				int b = b0 + (((b1 - b0) * w + (1 << 15)) >> 16);
				short rgb = (r << 8) + (g << 4) + b;
				if (rgb != pal[i]) {
					colors.push_back({(short)t, (short)i, rgb});
					pal[i] = rgb;
				}
			}
		}
		current_palette = pal;
	}

	void process_color_events(List<PEvent>& events) {
		for (PEvent event : events) {
			if (event.is<AColorEvent>()) {
				TintColor color = parse_color(event.cast<AColorEvent>().getColor());
				color.t = NUMBER_TO_INT(time);
				if (!is_fading) colors.push_back(color);
				while (current_palette.size() <= color.i) {
					current_palette.push_back(0x000);
				}
				current_palette[color.i] = color.rgb;
			} else if (event.is<AWaitEvent>()) {
				if (is_fading) do_fade();
				PExpression waitexp = event.cast<AWaitEvent>().getExpression();
				Value wait = apply(waitexp);
				time += wait.number;
			} else if (event.is<AFadeEvent>()) {
				if (is_fading) do_fade();
				fade_palette = current_palette;
				fade_time = time;
				is_fading = true;
				PExpression waitexp = event.cast<AFadeEvent>().getExpression();
				Value wait = apply(waitexp);
				time += wait.number;
			} else if (event.is<ARefEvent>()) {
				TIdentifier id = event.cast<ARefEvent>().getName();
				const std::string& name = id.getText();
				if (sym.look_map.count(name)) {
					ALookDecl look = sym.look_map[name];
					if (look_seen[look]) {
						throw CompileException(id, "Recursive look " + name);
					}
					look_seen[look] = true;
					process_color_events(look.getEvent());
					look_seen[look] = false;
				} else {
					throw CompileException(id, "Undefined look " + name);
				}
			}
		}
	}

public:
	std::vector<wire_mask_t> wire_conflicts;

	Interpreter(Reporter& rep, SymbolLinking& sym)
		: rep(rep), sym(sym), stats(nullptr), wire_conflicts(sym.wire_count) {}

	std::vector<Plot> interpret(AProcDecl main, RoseStatistics *stats) {
		this->stats = stats;

		procedure_phase = false;
		AProgram prog = main.parent().cast<AProgram>();
		sym.fact_values.clear();
		sym.traverse<AFactDecl>(prog, [&](AFactDecl fact) {
			Value fact_value = apply(fact.getExpression());
			sym.fact_values.push_back(fact_value.number);
		});

		State initial;
		initial.proc = main;
		initial.time = MAKE_NUMBER(0);
		initial.x = MAKE_NUMBER(0);
		initial.y = MAKE_NUMBER(0);
		initial.size = MAKE_NUMBER(2);
		initial.direction = MAKE_NUMBER(0);
		initial.tint = MAKE_NUMBER(1);
		initial.seed = 0xBABEFEED;
		initial.wire_values.resize(sym.wire_count);
		initial.wires_set = 0;
		initial.wires_written_since.resize(sym.wire_count);
		pending.push(std::move(initial));

		procedure_phase = true;
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

		sym.sortConstants();

		// Symmetric closure of wire conflicts
		for (int i = 0; i < sym.wire_count; i++) {
			wire_mask_t mask = wire_conflicts[i];
			for (int j = 0; j < sym.wire_count; j++) {
				if (mask & ((wire_mask_t)1 << j)) {
					wire_conflicts[j] |= (wire_mask_t)1 << i;
				}
			}
		}

		this->stats = nullptr;
		return output;
	}

	std::vector<TintColor> get_colors(AProgram program) {
		colors.clear();
		time = MAKE_NUMBER(0);
		current_palette.clear();
		is_fading = false;
		sym.traverse<APlanDecl>(program, [&](APlanDecl plan) {
			process_color_events(plan.getEvent());
		});
		if (is_fading) do_fade();
		return colors;
	}

	bool get_form(AProgram program, int *width_out, int *height_out, int *count_out, int *depth_out) {
		bool found = false;
		sym.traverse<AFormDecl>(program, [&](AFormDecl form) {
			short width = NUMBER_TO_INT(apply(form.getWidth()).number);
			short height = NUMBER_TO_INT(apply(form.getHeight()).number);
			short count = NUMBER_TO_INT(apply(form.getCount()).number);
			short depth = NUMBER_TO_INT(apply(form.getDepth()).number);
			if (count < 1) {
				throw CompileException(form.getToken(), "Layer count must be at least 1");
			}
			if (depth < 1) {
				throw CompileException(form.getToken(), "Layer depth must be at least 1");
			}
			if (found) {
				if (width != *width_out || height != *height_out || count != *count_out || depth != *depth_out) {
					throw CompileException(form.getToken(), "This form declaration conflicts with an earlier one");
				}
			}
			*width_out = width;
			*height_out = height;
			*count_out = count;
			*depth_out = depth;
			found = true;
		});
		return found;
	}

private:
	// Count CPU cycles
	void cpu(int cycles, int per_wire_cycles = 0) {
		if (stats != nullptr) {
			short f = NUMBER_TO_INT(state.time);
			if (f >= 0 && f < stats->frames) {
				stats->frame[f].cpu_compute_cycles += cycles;
				stats->frame[f].per_wire_cycles += per_wire_cycles;
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
			token = op.cast<APlusBinop>().getPlus();
			eval = [&](number_t a, number_t b) { return a + b; };
		} else if (op.is<AMinusBinop>()) {
			token = op.cast<AMinusBinop>().getMinus();
			eval = [&](number_t a, number_t b) { return a - b; };
		} else if (op.is<AMultiplyBinop>()) {
			token = op.cast<AMultiplyBinop>().getMul();
			eval = [&](number_t a, number_t b) {
				if (a >= (128 << 16) || a < -(128 << 16)) {
					rep.reportWarning(token, "Left operand overflows");
				}
				if (b >= (128 << 16) || b < -(128 << 16)) {
					rep.reportWarning(token, "Right operand overflows");
				}
				return (a << 8 >> 16) * (b << 8 >> 16);
			};
			cpu(126 - 20);
		} else if (op.is<ADivideBinop>()) {
			token = op.cast<ADivideBinop>().getDiv();
			eval = [&](number_t a, number_t b) {
				if (b >= (128 << 16) || b < -(128 << 16)) {
					rep.reportWarning(token, "Right operand overflows");
				}
				int divisor = b << 8 >> 16;
				if (divisor == 0) {
					throw CompileException(token, "Division by zero");
				}
				int div_result = a / divisor;
				if (b >= (128 << 16) || b < -(128 << 16)) {
					rep.reportWarning(token, "Result overflows");
				}
				return div_result << 8;
			};
			cpu(218 - 20);
		} else if (op.is<AAslBinop>()) {
			token = op.cast<AAslBinop>().getAsl();
			eval = [&](number_t a, number_t b) {
				int shift = (b >> 16) & 63;
				cpu(shift * 2);
				if (shift >= 32) return 0;
				return a << shift;
			};
		} else if (op.is<AAsrBinop>()) {
			token = op.cast<AAsrBinop>().getAsr();
			eval = [&](number_t a, number_t b) {
				int shift = (b >> 16) & 63;
				cpu(shift * 2);
				if (shift >= 32) return -1;
				return a >> shift;
			};
		} else if (op.is<ALsrBinop>()) {
			token = op.cast<ALsrBinop>().getLsr();
			eval = [&](number_t a, number_t b) {
				int shift = (b >> 16) & 63;
				cpu(shift * 2);
				if (shift >= 32) return 0;
				return (number_t)((unsigned)a >> shift);
			};
		} else if (op.is<ARolBinop>()) {
			token = op.cast<ARolBinop>().getRol();
			eval = [&](number_t a, number_t b) {
				int shift = (b >> 16) & 31;
				cpu(shift * 2);
				if (shift == 0) return a;
				return (number_t)((a << shift) | ((unsigned)a >> (32 - shift)));
			};
		} else if (op.is<ARorBinop>()) {
			token = op.cast<ARorBinop>().getRor();
			eval = [&](number_t a, number_t b) {
				int shift = (b >> 16) & 31;
				cpu(shift * 2);
				if (shift == 0) return a;
				return (number_t)(((unsigned)a >> shift) | (a << (32 - shift)));
			};
		} else if (op.is<AEqBinop>()) {
			token = op.cast<AEqBinop>().getEq();
			eval = [&](number_t a, number_t b) { return a == b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<ANeBinop>()) {
			token = op.cast<ANeBinop>().getNe();
			eval = [&](number_t a, number_t b) { return a != b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<ALtBinop>()) {
			token = op.cast<ALtBinop>().getLt();
			eval = [&](number_t a, number_t b) { return a < b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<ALeBinop>()) {
			token = op.cast<ALeBinop>().getLe();
			eval = [&](number_t a, number_t b) { return a <= b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<AGtBinop>()) {
			token = op.cast<AGtBinop>().getGt();
			eval = [&](number_t a, number_t b) { return a > b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<AGeBinop>()) {
			token = op.cast<AGeBinop>().getGe();
			eval = [&](number_t a, number_t b) { return a >= b ? MAKE_NUMBER(1) : MAKE_NUMBER(0); };
		} else if (op.is<AAndBinop>()) {
			token = op.cast<AAndBinop>().getAnd();
			eval = [&](number_t a, number_t b) { return a & b; };
		} else if (op.is<AOrBinop>()) {
			token = op.cast<AOrBinop>().getOr();
			eval = [&](number_t a, number_t b) { return a | b; };
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
		case VarKind::WIRE:
			if ((state.wires_set & ((wire_mask_t)1 << ref.index)) == 0) {
				throw CompileException(exp.getName(), "Uninitialized wire");
			}
			result = state.wire_values[ref.index];
			wire_conflicts[ref.index] |= state.wires_written_since[ref.index];
			break;
		case VarKind::FACT:
			if (ref.index >= sym.fact_values.size()) {
				throw CompileException(exp.getName(), "Facts can only refer to earlier facts");
			}
			result = Value(sym.fact_values[ref.index]);
			if (procedure_phase) {
				sym.registerConstant(exp, result.number);
			}
			break;
		case VarKind::PROCEDURE:
			result = Value(sym.procs[ref.index], true);
			break;
		}
		cpu(12 + 16);
	}

	void caseANumberExpression(ANumberExpression exp) override {
		result = Value(sym.literal_number[exp]);
		if (procedure_phase) {
			sym.registerConstant(exp, result.number);
		}
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
			cpu(344 + n_args * 34, 20);
		}
	}

	void caseATempStatement(ATempStatement s) override {
		state.stack.push_back(apply(s.getExpression()));
	}

	void caseAWireStatement(AWireStatement s) override {
		int index = sym.wire_index[s];
		state.wire_values[index] = apply(s.getExpression());
		state.wires_set |= (wire_mask_t)1 << index;
		for (int i = 0; i < sym.wire_count; i++) {
			state.wires_written_since[i] |= (wire_mask_t)1 << index;
		}
		state.wires_written_since[index] = 0;
	}

	void caseAWaitStatement(AWaitStatement s) override {
		Value wait = apply(s.getExpression());
		if (wait.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Wait value is not a number");
		}
		if (wait.number < 0) {
			rep.reportWarning(s.getToken(), "Negative wait");
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
			rep.reportWarning(s.getToken(), "Negative tint");
		} else if (tint_int >= stats->layer_count * stats->layer_depth) {
			rep.reportWarning(s.getToken(), "Tint value outside range");
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
