
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "symbol_linking.h"
#include "interpret.h"

using namespace rose;

std::pair<std::vector<Plot>, std::vector<TintColor>> translate(const char *filename, int max_time) {
	std::vector<Plot> plots;
	std::vector<TintColor> colors;
	try {
		Lexer lexer(filename);
		Start ast = rose::Parser(&lexer).parse();
		SymbolLinking sl;
		ast.apply(sl);
		Interpreter in(sl);
		AProgram program = ast.getPProgram().cast<AProgram>();
		AProcedure mainproc = program.getProcedure().front().cast<AProcedure>();
		plots = in.interpret(mainproc, max_time);
		colors = in.get_colors(program);
	} catch (const CompileException& exc) {
		printf("%s:%d:%d: %s\n", filename, exc.getToken().getLine(), exc.getToken().getPos(), exc.getMessage().c_str());
	} catch (const Exception& exc) {
		printf("%s: %s\n", filename, exc.getMessage().c_str());
	}

	return std::make_pair(std::move(plots), std::move(colors));
}

