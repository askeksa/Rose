#pragma once

#include "ast.h"
#include "symbol_linking.h"
#include "translate.h"

#include <functional>
#include <cstring>
#include <queue>

// 16:16 fixed point
typedef int number_t;
#define MAKE_NUMBER(n) (int(n * 65536))
#define NUMBER_TO_INT(n) ((short)(n >> 16))

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
	std::vector<Value> stack;

	State() {}
	State(AProcedure proc, const State& parent, std::vector<Value> stack) : proc(proc), stack(std::move(stack)) {
		time = parent.time;
		x = parent.x;
		y = parent.y;
		size = parent.size;
		direction = parent.direction;
		tint = parent.tint;
	}

	State(State&& state) = default;
	State& operator=(State&& state) = default;
};

class Interpreter : private ReturningAdapter<Value> {
	SymbolLinking& sym;
	State state;
	std::queue<State> pending;
	std::vector<Plot> output;

public:
	Interpreter(SymbolLinking& sym) : sym(sym) {}

	std::vector<Plot> interpret(AProcedure main, int max_time) {
		State initial;
		initial.proc = main;
		initial.time = MAKE_NUMBER(0);
		initial.x = MAKE_NUMBER(0);
		initial.y = MAKE_NUMBER(0);
		initial.size = MAKE_NUMBER(2);
		initial.direction = MAKE_NUMBER(0);
		initial.tint = MAKE_NUMBER(1);
		pending.push(std::move(initial));

		while (!pending.empty()) {
			state = std::move(pending.front());
			pending.pop();
			if (state.time < MAKE_NUMBER(max_time)) {
				state.proc.getBody().apply(*this);
			}
		}

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

private:
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

	// Expressions

	void caseABinaryExpression(ABinaryExpression exp) {
		Value left = apply(exp.getLeft());
		Value right = apply(exp.getRight());
		std::function<number_t(number_t,number_t)> eval;
		Token token;
		PBinop op = exp.getOp();
		if (op.is<APlusBinop>()) {
			eval = [&](number_t a, number_t b) { return a + b; };
			token = op.cast<APlusBinop>().getPlus();
		} else if (op.is<AMinusBinop>()) {
			eval = [&](number_t a, number_t b) { return a - b; };
			token = op.cast<AMinusBinop>().getMinus();
		} else if (op.is<AMultiplyBinop>()) {
			eval = [&](number_t a, number_t b) { return (a >> 8) * (b >> 8); };
			token = op.cast<AMultiplyBinop>().getMul();
		} else if (op.is<ADivideBinop>()) {
			eval = [&](number_t a, number_t b) { return (a / (b << 8 >> 16)) << 8; };
			token = op.cast<ADivideBinop>().getDiv();
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
			token = op.cast<AGtBinop>().getGt();
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

	void caseANegExpression(ANegExpression exp) {
		Value inner = apply(exp.getExpression());
		if (inner.kind != ValueKind::NUMBER) {
			throw CompileException(exp.getToken(), "Operand of negation is not a number");
		}
		result = Value(-inner.number);
	}

	void caseAVarExpression(AVarExpression exp) {
		switch (sym.var_kind[exp]) {
		case VarKind::GLOBAL:
			switch (sym.var_global[exp]) {
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
			result = state.stack[sym.local_index[sym.var_local[exp]]];
			break;
		case VarKind::PROCEDURE:
			result = Value(sym.var_proc[exp], true);
			break;
		}
	}

	void caseANumberExpression(ANumberExpression exp) {
		result = Value(sym.literal_number[exp]);
	}

	// Statements

	void caseAWhenStatement(AWhenStatement s) {
		Value cond = apply(s.getCond());
		if (cond.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Condition is not a number");
		}
		if (cond.number > 0) {
			s.getWhen().apply(*this);
		} else {
			s.getElse().apply(*this);
		}
	}

	void caseAForkStatement(AForkStatement s) {
		Value proc = apply(s.getProc());
		if (proc.kind != ValueKind::PROCEDURE) {
			throw CompileException(s.getToken(), "Target is not a procedure");
		}
		std::vector<Value> args;
		for (auto a : s.getArgs()) {
			args.push_back(apply(a));
		}
		pending.emplace(proc.proc, state, std::move(args));
	}

	void ATempStatement(ATempStatement s) {
		state.stack.push_back(apply(s.getExpression()));
	}

	void caseAWaitStatement(AWaitStatement s) {
		Value wait = apply(s.getExpression());
		if (wait.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Wait value is not a number");
		}
		state.time += wait.number;
	}

	void caseATurnStatement(ATurnStatement s) {
		Value turn = apply(s.getExpression());
		if (turn.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Turn value is not a number");
		}
		state.direction += turn.number;
	}

	void caseATurnStatement(AFaceStatement s) {
		Value face = apply(s.getExpression());
		if (face.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Face value is not a number");
		}
		state.direction = face.number;
	}

	void caseASizeStatement(ASizeStatement s) {
		Value size = apply(s.getExpression());
		if (size.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Size is not a number");
		}
		state.size = size.number;
	}

	void caseATintStatement(ATintStatement s) {
		Value tint = apply(s.getExpression());
		if (tint.kind != ValueKind::NUMBER) {
			throw CompileException(s.getToken(), "Tint is not a number");
		}
		state.tint = tint.number;
	}

	void caseAMoveStatement(AMoveStatement s) {
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
		} else {
			// High distance move
			state.x += (m << 2 >> 16) * ca;
			state.y += (m << 2 >> 16) * sa;
		}
	}

	void caseADrawStatement(ADrawStatement s) {
		output.push_back({NUMBER_TO_INT(state.time), NUMBER_TO_INT(state.x), NUMBER_TO_INT(state.y),
			NUMBER_TO_INT(state.size), NUMBER_TO_INT(state.tint)});
	}

};
