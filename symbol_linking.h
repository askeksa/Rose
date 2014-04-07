#pragma once

#include "ast.h"

#include <cstdlib>
#include <string>
#include <unordered_map>

enum class VarKind {
	GLOBAL,
	LOCAL,
	PROCEDURE
};

enum class GlobalKind {
	X, Y, DIRECTION
};

class SymbolLinking : public DepthFirstAdapter {
	std::unordered_map<std::string,AProcedure> proc_map;
	std::unordered_map<std::string,int> localindex_map;
	std::unordered_map<std::string,GlobalKind> global_map;
	int current_local_index;

public:
	nodemap<int> proc_index;
	nodemap<VarKind> var_kind;
	nodemap<GlobalKind> var_global;
	nodemap<int> var_localindex;
	nodemap<AProcedure> var_proc;
	nodemap<int> literal_number;

	void inAProgram(AProgram prog) {
		int current_proc_index = 0;
		for (auto p : prog.getProcedure()) {
			AProcedure proc = p.cast<AProcedure>();
			proc_map[proc.getName().getText()] = proc;
			proc_index[proc] = current_proc_index++;
		}
		global_map["x"] = GlobalKind::X;
		global_map["y"] = GlobalKind::Y;
		global_map["dir"] = GlobalKind::DIRECTION;
	}

	void outAProgram(AProgram prog) {
		proc_map.clear();
		global_map.clear();
	}

	void inAProcedure(AProcedure proc) {
		current_local_index = 0;
		for (auto p : proc.getParams()) {
			ALocal local = p.cast<ALocal>();
			localindex_map[local.getName().getText()] = current_local_index++;
		}
	}

	void outATempStatement(ATempStatement temp) {
		// TODO: Handle when/else/done scope
		ALocal local = temp.getVar().cast<ALocal>();
		localindex_map[local.getName().getText()] = current_local_index++;
	}

	void outAProcedure(AProcedure proc) {
		localindex_map.clear();
	}

	void inAVarExpression(AVarExpression var) {
		std::string name = var.getName().getText();
		if (global_map.count(name)) {
			var_kind[var] = VarKind::GLOBAL;
			var_global[var] = global_map[name];
		} else if (localindex_map.count(name)) {
			var_kind[var] = VarKind::LOCAL;
			var_localindex[var] = localindex_map[name];
		} else if (proc_map.count(name)) {
			var_kind[var] = VarKind::PROCEDURE;
			var_proc[var] = proc_map[name];
		} else {
			throw CompileException(var.getName(), "Unknown variable " + name);
		}
	}

	void inANumberExpression(ANumberExpression lit) {
		const char *num = lit.getNumber().getText().c_str();
		char *end;
		double val = strtod(num, &end);
		if (*end != '\0') {
			throw CompileException(lit.getNumber(), "Number format error");
		}
		literal_number[lit] = int(val * 65536);
	}
};

