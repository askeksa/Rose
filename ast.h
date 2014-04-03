#pragma once

#include "node.h"
#include "token.h"
#include "list.h"
#include "prod.h"
#include "analysis.h"

using namespace rose;

#include <unordered_map>

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
