#pragma once

#include "ast.h"
#include "symbol_linking.h"
#include "translate.h"
#include "bytecode.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

class CodeGenerator : private AnalysisAdapter {
	SymbolLinking& sym;
	const char *filename;
	std::vector<bytecode_t> out;
	RoseStatistics& stats;

	int stack_height;
	std::vector<int> saved_stack_height;
	int op_code;
	int cmp_code;
	nodemap<bool> tail_fork;

public:
	CodeGenerator(SymbolLinking& sym, const char *filename, RoseStatistics& stats)
		: sym(sym), filename(filename), stats(stats) {}

	std::pair<std::vector<bytecode_t>,std::vector<number_t>> generate(AProgram program) {
		program.getProcedure().apply(*this);
		out.push_back(END_OF_SCRIPT);

		return make_pair(std::move(out), sym.constants);
	}

private:
	void emit(bytecode_t code) {
		out.push_back(code);
		if (stack_height == STACK_AFTER_TAIL && code != BC_ELSE && code != BC_DONE && code != BC_END) {
			throw Exception("Instruction after tail call");
		}
		stack_height += stack_change(code);
		if ((code & 0xF0) == BC_WHEN(0)) {
			saved_stack_height.push_back(stack_height);
		} else if (code == BC_ELSE) {
			std::swap(stack_height, saved_stack_height.back());
		} else if (code == BC_DONE) {
			int when_height = saved_stack_height.back();
			saved_stack_height.pop_back();
			if (when_height == STACK_AFTER_TAIL || stack_height == STACK_AFTER_TAIL) {
				stack_height = std::min(when_height, stack_height);
			} else if (when_height != stack_height) {
				throw Exception(std::string("Mismatching stack heights: ") + std::to_string(when_height) + " vs " + std::to_string(stack_height));
			}
		} else if (code == BC_TAIL) {
			stack_height = STACK_AFTER_TAIL;
		}
		if (stack_height != STACK_AFTER_TAIL && stack_height > stats.max_stack_height) {
			stats.max_stack_height = stack_height;
		}
	}

	void pop(int count) {
		for (int i = 0; i < count; i++) {
			emit(BC_POP);
		}
	}

	void emit_constant(int value) {
		emit(BC_CONST(sym.constant_index[value]));
	}

	void mark_tail(PStatement s) {
		if (s.is<AForkStatement>()) {
			tail_fork[s] = true;
		} else if (s.is<AWhenStatement>()) {
			AWhenStatement when = s.cast<AWhenStatement>();
			if (!when.getWhen().empty()) {
				mark_tail(when.getWhen().back());
			}
			if (!when.getElse().empty()) {
				mark_tail(when.getElse().back());
			}
		}
	}

	void caseAProcedure(AProcedure proc) override {
		List<PStatement>& body = proc.getBody();
		if (!body.empty()) mark_tail(body.back());
		stack_height = proc.getParams().size();
		proc.getBody().apply(*this);
		emit(BC_END);		
	}

	void caseAPlusBinop(APlusBinop)         override { op_code = BC_OP(OP_ADD); cmp_code = CMP_NE; }
	void caseAMinusBinop(AMinusBinop)       override { op_code = BC_OP(OP_SUB); cmp_code = CMP_NE; }
	void caseAMultiplyBinop(AMultiplyBinop) override { op_code = BC_MUL;        cmp_code = CMP_NE; }
	void caseADivideBinop(ADivideBinop)     override { op_code = BC_DIV;        cmp_code = CMP_NE; }
	void caseAEqBinop(AEqBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_EQ; }
	void caseANeBinop(ANeBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_NE; }
	void caseALtBinop(ALtBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_LT; }
	void caseALeBinop(ALeBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_LE; }
	void caseAGtBinop(AGtBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_GT; }
	void caseAGeBinop(AGeBinop)             override { op_code = BC_OP(OP_CMP); cmp_code = CMP_GE; }
	void caseAAndBinop(AAndBinop)           override { op_code = BC_OP(OP_AND); cmp_code = CMP_NE; }
	void caseAOrBinop(AOrBinop)             override { op_code = BC_OP(OP_OR);  cmp_code = CMP_NE; }

	void caseABinaryExpression(ABinaryExpression exp) override {
		exp.getRight().apply(*this);
		exp.getLeft().apply(*this);
		exp.getOp().apply(*this);
		emit(op_code);
		if (op_code == BC_OP(OP_CMP) && !exp.parent().is<AWhenStatement>() && !exp.parent().is<ACondExpression>()) {
			// Produce truth value
			emit(BC_WHEN(cmp_code));
			emit_constant(MAKE_NUMBER(1));
			emit(BC_ELSE);
			emit_constant(MAKE_NUMBER(0));
			emit(BC_DONE);
		}
	}

	void caseANumberExpression(ANumberExpression exp) override {
		emit_constant(sym.literal_number[exp]);
	}

	void caseAVarExpression(AVarExpression exp) override {
		VarRef var = sym.var_ref[exp];
		switch (var.kind) {
		case VarKind::GLOBAL:
			switch (static_cast<GlobalKind>(var.index)) {
			case GlobalKind::X:
				emit(BC_RSTATE(ST_X));
				break;
			case GlobalKind::Y:
				emit(BC_RSTATE(ST_Y));
				break;
			case GlobalKind::DIRECTION:
				emit(BC_RSTATE(ST_DIR));
				break;
			}
			break;
		case VarKind::LOCAL:
			emit(BC_RLOCAL(var.index));
			break;
		case VarKind::WIRE:
			emit(BC_RSTATE(ST_WIRE0 + var.index));
			if (var.index >= stats.max_wires) {
				stats.max_wires = var.index + 1;
			}
			break;
		case VarKind::PROCEDURE:
			emit(BC_PROC);
			out.push_back(var.index);
			break;
		}
	}

	void caseANegExpression(ANegExpression exp) override {
		exp.getExpression().apply(*this);
		emit(BC_NEG);
	}

	void caseASineExpression(ASineExpression exp) override {
		exp.getExpression().apply(*this);
		emit(BC_SINE);
	}

	void caseARandExpression(ARandExpression exp) override {
		emit(BC_RAND);
	}

	void caseACondExpression(ACondExpression exp) override {
		cmp_code = CMP_NE;
		exp.getCond().apply(*this);
		emit(BC_WHEN(cmp_code));
		exp.getWhen().apply(*this);
		emit(BC_ELSE);
		exp.getElse().apply(*this);
		emit(BC_DONE);
	}

	void caseAWhenStatement(AWhenStatement s) override {
		cmp_code = CMP_NE;
		s.getCond().apply(*this);
		emit(BC_WHEN(cmp_code));
		s.getWhen().apply(*this);
		if (stack_height != STACK_AFTER_TAIL) {
			pop(sym.when_pop[s]);
		}
		if (!s.getElse().empty()) {
			emit(BC_ELSE);
			s.getElse().apply(*this);
			if (stack_height != STACK_AFTER_TAIL) {
				pop(sym.else_pop[s]);
			}
		}
		emit(BC_DONE);
	}

	bool makeTailCall(AForkStatement s) {
		if (tail_fork[s]) {
			// Not enough space for arguments?
			if (s.getArgs().size() > stack_height) {
				//sym.warning(s.getToken(), "Tail fork not optimized because of too little stack space");
				return false;
			}

			// Find non-identity arguments
			std::vector<std::pair<int,PExpression>> args;
			int index = 0;
			for (PExpression exp : s.getArgs()) {
				bool identity = false;
				if (exp.is<AVarExpression>()) {
					VarRef var = sym.var_ref[exp];
					if (var.kind == VarKind::LOCAL && var.index == index) {
						identity = true;
					}
				}
				if (!identity) {
					args.emplace_back(index, exp);
				}
				index++;
			}

			// Generate code
			s.getProc().apply(*this);
			for (auto& a : args) {
				a.second.apply(*this);
			}
			std::reverse(args.begin(), args.end());
			for (auto& a : args) {
				emit(BC_WLOCAL(a.first));
			}
			emit(BC_WSTATE(ST_PROC));
			pop(stack_height - s.getArgs().size());
			emit(BC_TAIL);

			return true;
		}
		return false;
	}

	void caseAForkStatement(AForkStatement s) override {
		if (!makeTailCall(s)) {
			s.getArgs().apply(*this);
			s.getProc().apply(*this);
			emit(BC_FORK(s.getArgs().size()));
		}
	}

	void caseATempStatement(ATempStatement s) override {
		s.getExpression().apply(*this);
	}

	void caseAWireStatement(AWireStatement s) override {
		int index = sym.wire_index[s];
		s.getExpression().apply(*this);
		emit(BC_WSTATE(ST_WIRE0 + index));
		if (index >= stats.max_wires) {
			stats.max_wires = index + 1;
		}
	}

	void caseAWaitStatement(AWaitStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_WAIT);
	}

	void caseATurnStatement(ATurnStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_RSTATE(ST_DIR));
		emit(BC_OP(OP_ADD));
		emit(BC_WSTATE(ST_DIR));
	}

	void caseAFaceStatement(AFaceStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_WSTATE(ST_DIR));
	}

	void caseASizeStatement(ASizeStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_WSTATE(ST_SIZE));
	}

	void caseATintStatement(ATintStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_WSTATE(ST_TINT));
	}

	void caseASeedStatement(ASeedStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_SEED);
	}

	void caseAMoveStatement(AMoveStatement s) override {
		s.getExpression().apply(*this);
		emit(BC_MOVE);
	}

	void caseAJumpStatement(AJumpStatement s) override {
		bool same_x = false, same_y = false;
		if (s.getX().is<AVarExpression>()) {
			VarRef var = sym.var_ref[s.getX()];
			same_x = var.kind == VarKind::GLOBAL
			      && static_cast<GlobalKind>(var.index) == GlobalKind::X;
		}
		if (s.getY().is<AVarExpression>()) {
			VarRef var = sym.var_ref[s.getY()];
			same_y = var.kind == VarKind::GLOBAL
			      && static_cast<GlobalKind>(var.index) == GlobalKind::Y;
		}
		if (!same_y) s.getY().apply(*this);
		if (!same_x) s.getX().apply(*this);
		if (!same_x) emit(BC_WSTATE(ST_X));
		if (!same_y) emit(BC_WSTATE(ST_Y));
	}

	void caseADrawStatement(ADrawStatement s) override {
		emit(BC_DRAW);
	}

	void caseAPlotStatement(APlotStatement s) override {
		emit(BC_PLOT);
	}

};
