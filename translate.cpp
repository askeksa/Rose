
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "symbol_linking.h"
#include "interpret.h"
#include "code_generator.h"

#include <stdio.h>

using namespace rose;

template <typename T>
void writefile(std::vector<T> data, const char *filename) {
	// Convert to big endian
	for (int i = 0 ; i < data.size() ; i++) {
		T value = data[i];
		T swapped = 0;
		for (int j = 0 ; j < sizeof(T) ; j++) {
			swapped |= ((value >> (j * 8)) & 0xFF) << ((sizeof(T) - j - 1) * 8);
		}
		data[i] = swapped;
	}
	FILE *out = fopen(filename, "wb");
	fwrite(&data[0], sizeof(T), data.size(), out);
	fclose(out);
}

std::pair<std::vector<Plot>, std::vector<TintColor>> translate(const char *filename, int max_time, bool print) {
	std::vector<Plot> plots;
	std::vector<TintColor> colors;
	try {
		Lexer lexer(filename);
		Start ast = rose::Parser(&lexer).parse();
		SymbolLinking sl;
		ast.apply(sl);
		Interpreter in(sl, filename);
		AProgram program = ast.getPProgram().cast<AProgram>();
		if (program.getProcedure().size() == 0) {
			throw Exception("No procedures");
		}
		AProcedure mainproc = program.getProcedure().front().cast<AProcedure>();
		if (mainproc.getParams().size() != 0) {
			throw CompileException(mainproc.getName(), "Entry procedure must not have any parameters");
		}
		plots = in.interpret(mainproc, max_time);
		colors = in.get_colors(program);

		// Output
		CodeGenerator codegen(sl, filename);
		auto bytecodes_and_constants = codegen.generate(program);
		std::vector<bytecode_t> bytecodes = bytecodes_and_constants.first;
		std::vector<number_t> constants = bytecodes_and_constants.second;
		std::vector<unsigned short> colorscript;
		int color_frame = -1;
		for (auto c : colors) {
			if (c.t != color_frame) {
				int delta = c.t - color_frame;
				color_frame = c.t;
				colorscript.push_back(-delta);
			}
			colorscript.push_back(c.rgb | (c.i << 12));
		}
		writefile(bytecodes, "bytecodes.bin");
		writefile(constants, "constants.bin");
		writefile(colorscript, "colorscript.bin");

	} catch (const CompileException& exc) {
		if (print) {
			printf("%s:%d:%d: Error: %s\n", filename, exc.getToken().getLine(), exc.getToken().getPos(), exc.getMessage().c_str());
			fflush(stdout);
		}
	} catch (const Exception& exc) {
		if (print) {
			printf("%s: Error: %s\n", filename, exc.getMessage().c_str());
			fflush(stdout);
		}
	}

	return std::make_pair(std::move(plots), std::move(colors));
}

