#pragma once

#include "node.h"
#include "token.h"
#include "list.h"
#include "prod.h"
#include "analysis.h"

using namespace rose;

#include <functional>
#include <unordered_map>
#include <unordered_set>

// 16:16 fixed point
typedef int number_t;
#define MAKE_NUMBER(n) (int((n) * 65536))
#define NUMBER_TO_INT(n) ((short)((n) >> 16))

template <class T>
class nodemap {
	std::unordered_map<void*,T> inner_map;
public:
	template <class N>
	T& operator[](N n) {
		void* key = *(void**)(void*)&n;
		return inner_map[key];
	}

	size_t size() { return inner_map.size(); }
};

class Reporter;

class ProgramAdapter : public DepthFirstAdapter {
protected:
	Reporter& rep;
	nodemap<AProgram>& parts;

public:
	ProgramAdapter(Reporter& rep, nodemap<AProgram>& parts) : rep(rep), parts(parts) {}

	virtual ~ProgramAdapter() {}

	template <typename N>
	void traverse(AProgram program, std::function<void (N node)> fun) {
		for (PDecl decl : program.getDecl()) {
			if (decl.is<N>()) {
				N node = decl.cast<N>();
				fun(node);
			}
			if (decl.is<APartDecl>()) {
				APartDecl part = decl.cast<APartDecl>();
				AProgram part_program = parts[part];
				if (part_program) {
					traverse(part_program, fun);
				}
			}
		}
	}

	template <typename N>
	void visit(AProgram program) {
		traverse<N>(program, [this](N node) {
			node.apply(*this);
		});
	}
};

template <typename R>
class ReturningAdapter : public AnalysisAdapter {
protected:
	R result;

	R apply(Node n) {
		n.apply(*this);
		return result;
	}

	virtual ~ReturningAdapter() {}
};

class CompileException : public Exception {
public:
	inline CompileException (Token token, const std::string& msg) : Exception (msg), token(token) { }
	Token getToken () const { return token; }

private:
	Token token;
};

class Reporter {
	const char* main_filename;
	AProgram program;
	nodemap<AProgram>& parts;
	nodemap<std::string>& part_path;
	std::unordered_set<int> defied_lines;
	nodemap<std::unordered_set<std::string>> warning_nodes;

	const char* filename(Token token) {
		Node node = token;
		while (!node.is<AProgram>()) {
			node = node.parent();
		}
		AProgram token_program = node.cast<AProgram>();

		if (token_program == program) {
			return main_filename;
		}

		APartDecl token_part;
		ProgramAdapter adapter(*this, parts);
		adapter.traverse<APartDecl>(program, [&](APartDecl part) {
			if (parts[part] == token_program) {
				token_part = part;
			}
		});
		return part_path[token_part].c_str();
	}

public:
	Reporter(const char* main_filename, AProgram program, nodemap<AProgram>& parts, nodemap<std::string>& part_path)
		: main_filename(main_filename), program(program), parts(parts), part_path(part_path) {
	}

	void defy(Token token) {
		defied_lines.insert(token.getLine());
	}

	void reportWarning(Token token, std::string message) {
		if (!defied_lines.count(token.getLine()) && !warning_nodes[token].count(message)) {
			printf("%s:%d:%d: Warning: %s\n", filename(token), token.getLine(), token.getPos(), message.c_str());
			fflush(stdout);
			warning_nodes[token].insert(message);
		}
	}

	void reportError(const CompileException& exc) {
		printf("%s:%d:%d: Error: %s\n", filename(exc.getToken()), exc.getToken().getLine(), exc.getToken().getPos(), exc.getMessage().c_str());
		fflush(stdout);
	}
};
