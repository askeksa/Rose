#pragma once

#include "ast.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum class VarKind {
	GLOBAL,
	LOCAL,
	PROCEDURE
};

enum class GlobalKind {
	X, Y, DIRECTION
};

struct VarRef {
	VarKind kind;
	int index;
};

class Scope {
	Scope* parent;
	Node node;

	std::unordered_map<std::string,VarRef> scope_map;

public:

	Scope(Scope* parent, Node node) : parent(parent), node(node) {}

	template <class V>
	void add(TIdentifier var, VarKind kind, V value) {
		const std::string& name = var.getText();
		if (scope_map.count(name)) {
			throw CompileException(var, "Redefinition of " + name);
		}
		scope_map[name] = { kind, static_cast<int>(value) };
	}

	VarRef lookup(TIdentifier var) {
		const std::string& name = var.getText();
		if (scope_map.count(name)) {
			return scope_map[name];
		}
		if (parent == nullptr) {
			throw CompileException(var, "Undefined variable " + name);
		}		
		return parent->lookup(var);
	}

	Scope* pop() {
		Scope *p = parent;
		delete this;
		return p;
	}

	Node getNode() {
		return node;
	}
};

class SymbolLinking : public DepthFirstAdapter {
	const char *filename;

	int current_local_index;
	Scope* current_scope;

	nodemap<int> when_local_index;
	std::unordered_set<int> defied_lines;
	nodemap<std::unordered_set<std::string>> warning_nodes;

public:
	std::vector<AProcedure> procs;
	nodemap<VarRef> var_ref;
	nodemap<int> literal_number;
	nodemap<int> when_pop;
	nodemap<int> else_pop;

	SymbolLinking(const char *filename) : filename(filename) {}

	void warning(Token token, std::string message) {
		if (!defied_lines.count(token.getLine()) && !warning_nodes[token].count(message)) {
			printf("%s:%d:%d: Warning: %s\n", filename, token.getLine(), token.getPos(), message.c_str());
			warning_nodes[token].insert(message);
		}
	}

	void inAProgram(AProgram prog) override {
		current_scope = new Scope(nullptr, prog);
		current_scope->add(TIdentifier::make("x"), VarKind::GLOBAL, GlobalKind::X);
		current_scope->add(TIdentifier::make("y"), VarKind::GLOBAL, GlobalKind::Y);
		current_scope->add(TIdentifier::make("dir"), VarKind::GLOBAL, GlobalKind::DIRECTION);
		int current_proc_index = 0;
		for (auto p : prog.getProcedure()) {
			AProcedure proc = p.cast<AProcedure>();
			procs.push_back(proc);
			current_scope->add(proc.getName(), VarKind::PROCEDURE, current_proc_index++);
		}
	}

	void outAProgram(AProgram prog) override {
		current_scope = current_scope->pop();
	}

	void inAProcedure(AProcedure proc) override {
		current_scope = new Scope(current_scope, proc);
		current_local_index = 0;
		for (auto p : proc.getParams()) {
			ALocal local = p.cast<ALocal>();
			current_scope->add(local.getName(), VarKind::LOCAL, current_local_index++);
		}
	}

	void outATempStatement(ATempStatement temp) override {
		ALocal local = temp.getVar().cast<ALocal>();
		current_scope->add(local.getName(), VarKind::LOCAL, current_local_index++);
	}

	void outAProcedure(AProcedure proc) override {
		current_scope = current_scope->pop();
	}

	void inAVarExpression(AVarExpression var) override {
		VarRef ref = current_scope->lookup(var.getName());
		var_ref[var] = ref;
	}

	void inANumberExpression(ANumberExpression lit) override {
		const char *num = lit.getNumber().getText().c_str();
		char *end;
		int value;
		if (num[0] == '$') {
			if (strlen(num) > 9) {
				throw CompileException(lit.getNumber(), "Hex number too large");
			}
			value = strtoll(&num[1], &end, 16);
		} else {
			double fvalue = strtod(num, &end);
			if (fvalue >= 65536) {
				throw CompileException(lit.getNumber(), "Number too large");
			}
			if (fvalue >= 32768) {
				warning(lit.getNumber(), "Number overflows to negative");
			}
			value = int(fvalue * 65536);
		}
		if (*end != '\0') {
			throw CompileException(lit.getNumber(), "Number format error");
		}
		literal_number[lit] = value;
	}

	void inAWhenStatement(AWhenStatement when) override {
		when_local_index[when] = current_local_index;
		current_scope = new Scope(current_scope, when);
	}

	void inAElseMarker(AElseMarker m) override {
		AWhenStatement when = current_scope->getNode().cast<AWhenStatement>();
		when_pop[when] = current_local_index - when_local_index[when];
		current_local_index = when_local_index[when];
		current_scope = new Scope(current_scope->pop(), when);
	}

	void outAWhenStatement(AWhenStatement when) override {
		else_pop[when] = current_local_index - when_local_index[when];
		current_local_index = when_local_index[when];
		current_scope = current_scope->pop();
	}

	void inADefyStatement(ADefyStatement defy) {
		int line = defy.getToken().getLine();
		defied_lines.insert(line);
	}
};

