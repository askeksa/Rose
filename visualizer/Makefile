
EXTERNAL := ../../Andres
BUILD = build
DIST_DIR = ../dist/Rose

CC := i686-w64-mingw32-g++
CFLAGS := -Iparser/rose -I$(EXTERNAL)/glfw-3.0.4.bin.WIN32/include -I$(EXTERNAL)/glew-1.10.0/include -I$(EXTERNAL)/portaudio/include -Wno-write-strings -std=c++11
LFLAGS := $(EXTERNAL)/glew-1.10.0/lib/Release/Win32/glew32s.lib -L$(EXTERNAL)/glfw-3.0.4.bin.WIN32/lib-mingw $(EXTERNAL)/portaudio/mingw32/usr/local/lib/libportaudio-2.dll -lglfw3 -lopengl32 -luser32 -lgdi32 -static-libgcc -static-libstdc++
#CC := x86_64-w64-mingw32-g++
#CFLAGS := -O3 -Iparser/rose -I$(EXTERNAL)/glfw-3.0.4.bin.WIN64/include -I$(EXTERNAL)/glew-1.10.0/include -I$(EXTERNAL)/portaudio/include -Wno-write-strings -std=c++11
#LFLAGS := $(EXTERNAL)/glew-1.10.0/lib/Release/x64/glew32s.lib -L$(EXTERNAL)/glfw-3.0.4.bin.WIN64/lib-mingw -lglfw3 -luser32 -lopengl32 -lgdi32 -static-libgcc -static-libstdc++ -s

ifeq ($(DEBUG),yes)
CFLAGS += -g
else
CFLAGS += -O3
LFLAGS += -s
endif

$(BUILD)/rose: $(patsubst %,$(BUILD)/%.o,main translate renderer music) $(patsubst parser/%.cpp,$(BUILD)/%.o,$(wildcard parser/*.cpp))
	$(CC) $^ $(LFLAGS) -o $(BUILD)/rose
	cp lib/* $(BUILD)/

$(BUILD)/%.o: %.cpp Makefile
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILD)/%.o: parser/%.cpp parser Makefile
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILD)/main.o: main.cpp translate.h rose_result.h music.h

$(BUILD)/translate.o: translate.cpp translate.h rose_result.h ast.h symbol_linking.h interpret.h code_generator.h bytecode.h parser

$(BUILD)/renderer.o: renderer.cpp shaders.h rose_result.h

$(BUILD)/music.o: music.cpp music.h

parser: rose.sablecc
	mkdir -p parser
	java -jar tools/sablecc.jar -t cxx -d parser rose.sablecc
	touch parser

clean:
	rm -f $(BUILD)/*

dist: $(BUILD)/rose
	rm -rf $(DIST_DIR)
	mkdir -p $(DIST_DIR)
	cp $(BUILD)/rose $(DIST_DIR)/
	cp lib/* $(DIST_DIR)/
	cp -R ../examples $(DIST_DIR)/
	cp -R ../music $(DIST_DIR)/
