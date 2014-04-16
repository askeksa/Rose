#pragma once

#include "ast.h"
#include "symbol_linking.h"
#include "translate.h"
#include "bytecode.h"

#include <vector>
#include <unordered_map>
#include <string>

class CodeGenerator : private AnalysisAdapter {
	SymbolLinking& sym;
	const char *filename;
	std::vector<number_t> constants;
	std::unordered_map<int,int> constant_index;
	std::vector<bytecode_t> out;
	RoseStatistics& stats;

	int stack_height;
	std::vector<int> saved_stack_height;
	int op_code;
	int cmp_code;

public:
	CodeGenerator(SymbolLinking& sym, const char *filename, RoseStatistics& stats)
		: sym(sym), filename(filename), stats(stats) {}

	std::pair<std::vector<bytecode_t>,std::vector<number_t>> generate(AProgram program) {
		constants.resize(program.getProcedure().size(), 0);
		program.getProcedure().apply(*this);

		return make_pair(std::move(out), std::move(constants));
	}

private:
	void emit(bytecode_t code) {
		out.push_back(code);
		stack_height += stack_change(code);
		if ((code & 0xF0) == BC_WHEN(0)) {
			saved_stack_height.push_back(stack_height);
		} else if (code == BC_ELSE) {
			std::swap(stack_height, saved_stack_height.back());
		} else if (code == BC_DONE) {
			int when_height = saved_stack_height.back();
			saved_stack_height.pop_back();
			if (when_height != stack_height) {
				throw Exception(std::string("Mismatching stack heights: ") + std::to_string(when_height) + " vs " + std::to_string(stack_height));
			}
		}
		if (stack_height > stats.max_stack_height) stats.max_stack_height = stack_height;
	}

	void emit_constant(int c) {
		int index;
		if (constant_index.count(c)) {
			index = constant_index[c];
		} else {
			index = constants.size();
			constant_index[c] = index;
			constants.push_back(c);
		}
		emit(BC_CONST(index));
	}

	void caseAProcedure(AProcedure proc) override {
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
			emit(BC_LOCAL(stack_height - var.index - 1));
			break;
		case VarKind::PROCEDURE:
			emit(BC_CONST(var.index));
			break;
		}
	}

	void caseANegExpression(ANegExpression exp) override {
		exp.getExpression().apply(*this);
		emit_constant(MAKE_NUMBER(0));
		emit(BC_OP(OP_SUB));
	}

	void caseASineExpression(ASineExpression exp) override {
		exp.getExpression().apply(*this);
		emit(BC_SINE);
	}

	void caseARandExpression(ARandExpression exp) override {
		emit(BC_RAND);
	}

	void caseACondExpression(ACondExpression exp) override {
		exp.getCond().apply(*this);
		emit(BC_WHEN(cmp_code));
		exp.getWhen().apply(*this);
		emit(BC_ELSE);
		exp.getElse().apply(*this);
		emit(BC_DONE);
	}

	void caseAWhenStatement(AWhenStatement s) override {
		s.getCond().apply(*this);
		emit(BC_WHEN(cmp_code));
		s.getWhen().apply(*this);
		if (sym.when_pop[s] > 0) {
			emit(BC_POP(sym.when_pop[s]));
		}
		if (!s.getElse().empty()) {
			emit(BC_ELSE);
			s.getElse().apply(*this);
			if (sym.else_pop[s] > 0) {
				emit(BC_POP(sym.else_pop[s]));
			}
		}
		emit(BC_DONE);
	}

	void caseAForkStatement(AForkStatement s) override {
		s.getArgs().reverse_apply(*this);
		s.getProc().apply(*this);
		emit(BC_FORK(s.getArgs().size()));
	}

	void caseATempStatement(ATempStatement s) override {
		s.getExpression().apply(*this);
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
		s.getY().apply(*this);
		s.getX().apply(*this);
		emit(BC_WSTATE(ST_X));
		emit(BC_WSTATE(ST_Y));
	}

	void caseADrawStatement(ADrawStatement s) override {
		emit(BC_DRAW);
	}

};
