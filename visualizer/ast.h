#pragma once

#include "node.h"
#include "token.h"
#include "list.h"
#include "prod.h"
#include "analysis.h"

using namespace rose;

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
	const char *filename;
	std::unordered_set<int> defied_lines;
	nodemap<std::unordered_set<std::string>> warning_nodes;

public:
	Reporter(const char *filename) : filename(filename) {}

	void defy(Token token) {
		defied_lines.insert(token.getLine());
	}

	void reportWarning(Token token, std::string message) {
		if (!defied_lines.count(token.getLine()) && !warning_nodes[token].count(message)) {
			printf("%s:%d:%d: Warning: %s\n", filename, token.getLine(), token.getPos(), message.c_str());
			fflush(stdout);
			warning_nodes[token].insert(message);
		}
	}

	void reportError(const Exception& exc) {
		printf("%s: Error: %s\n", filename, exc.getMessage().c_str());
		fflush(stdout);
	}

	void reportCompileError(const CompileException& exc) {
		printf("%s:%d:%d: Error: %s\n", filename, exc.getToken().getLine(), exc.getToken().getPos(), exc.getMessage().c_str());
		fflush(stdout);
	}
};
