
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "symbol_linking.h"
#include "interpret.h"
#include "code_generator.h"

#include <algorithm>
#include <cstdio>
#include <fstream>

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

static AProgram loadProgram(const char *filename, std::string& current_filename,
		nodemap<AProgram>& parts, nodemap<std::string>& part_path,
		std::vector<std::string>& paths) {
	current_filename = filename;
	if (!std::ifstream(filename)) {
		printf("File not found: %s\n", filename);
	}
	paths.emplace_back(filename);
	Lexer lexer(filename);
	Start ast = rose::Parser(&lexer).parse();
	AProgram program = ast.getPProgram().cast<AProgram>();
	for (PDecl decl : program.getDecl()) {
		if (decl.is<APartDecl>()) {
			APartDecl part = decl.cast<APartDecl>();
			const std::string& file_string = part.getFile().getText();
			const std::string file = file_string.substr(1, file_string.size() - 2);
			std::string path(filename);
			path.resize(path.find_last_of("/\\") + 1);
			path += file;
			if (!std::count(paths.begin(), paths.end(), path)) {
				AProgram part_program = loadProgram(path.c_str(), current_filename, parts, part_path, paths);
				parts[part] = part_program;
				part_path[part] = path;
			}
		}
	}
	return program;
}

RoseResult translate(const char *filename, int max_time,
                     int width, int height,
                     int layer_count, int layer_depth) {
	RoseResult result;
	result.width = width;
	result.height = height;
	result.layer_count = layer_count;
	result.layer_depth = layer_depth;
	result.error = false;
	std::string current_filename;
	try {
		nodemap<AProgram> parts;
		nodemap<std::string> part_path;
		AProgram program = loadProgram(filename, current_filename, parts, part_path, result.paths);
		Reporter rep(filename, program, parts, part_path);
		try {
			SymbolLinking sym(rep, parts);
			program.apply(sym);
			Interpreter in(rep, sym);
			AProcDecl mainproc;
			int n_proc = 0;
			sym.traverse<AProcDecl>(program, [&](AProcDecl proc) {
				if (n_proc == 0) {
					mainproc = proc;
				}
				n_proc++;
			});
			if (n_proc == 0) {
				throw Exception("No procedures");
			}
			if (mainproc.getParams().size() != 0) {
				throw CompileException(mainproc.getName(), "Entry procedure must not have any parameters");
			}

			in.get_form(program, &width, &height, &layer_count, &layer_depth);
			result.width = width;
			result.height = height;
			result.layer_count = layer_count;
			result.layer_depth = layer_depth;

			result.stats.reset(new RoseStatistics(max_time, width, height, layer_count, layer_depth));
			RoseStatistics& stats = *result.stats;

			result.plots = in.interpret(mainproc, &stats);
			result.colors = in.get_colors(program);

			// Output
			CodeGenerator codegen(rep, parts, sym, stats);
			auto bytecodes_and_constants = codegen.generate(program);
			std::vector<bytecode_t> bytecodes = bytecodes_and_constants.first;
			std::vector<number_t> constants = bytecodes_and_constants.second;
			std::vector<unsigned short> colorscript;
			int color_frame = -1;
			for (auto c : result.colors) {
				if (c.t != color_frame) {
					int delta = c.t - color_frame;
					color_frame = c.t;
					colorscript.push_back(-delta);
				}
				colorscript.push_back(c.rgb | (c.i << 12));
			}
			colorscript.push_back(0x8000);
			writefile(bytecodes, "bytecodes.bin");
			writefile(constants, "constants.bin");
			writefile(colorscript, "colorscript.bin");

			stats.print(stdout);

			printf("\n");
			int n = sym.constants.size();
			int n_columns = 4;
			int n_rows = (n - 1) / n_columns + 1;
			for (int r = 0 ; r < n_rows ; r++) {
				for (int c = 0 ; c < n_columns ; c++) {
					int i = r + c * n_rows;
					if (i < n) {
						int value = sym.constants[i];
						int frac = 16;
						while (frac > 0 && ((value >> (16 - frac)) & 1) == 0) {
							frac--;
						}
						int float_width = 6 + (frac > 0) + frac;
						int count = sym.constant_count[value];
						printf("%3d %08X%*.*f%*s", count, value, float_width, frac, value / 65536.0, 23 - float_width, "");
					}
				}
				printf("\n");
			}
			fflush(stdout);

		} catch (const CompileException& exc) {
			rep.reportError(exc);
			result.error = true;
		}
	} catch (const Exception& exc) {
		printf("%s: %s\n", current_filename.c_str(), exc.getMessage().c_str());
		fflush(stdout);
		result.error = true;
	}

	return result;
}

