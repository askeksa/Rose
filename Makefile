
CC := i686-w64-mingw32-g++
CFLAGS := -O3 -Iparser/rose -I../glfw-3.0.4.bin.WIN32/include -I../glew-1.10.0/include -I../portaudio/include -Wno-write-strings -std=c++11
LFLAGS := ../glew-1.10.0/lib/Release/Win32/glew32s.lib -L../glfw-3.0.4.bin.WIN32/lib-mingw -L../portaudio/mingw32/usr/local/lib -lglfw3 -lopengl32 -lportaudio.dll -luser32 -lgdi32 -static-libgcc -static-libstdc++ -s
#CC := x86_64-w64-mingw32-g++
#CFLAGS := -O3 -Iparser/rose -I../glfw-3.0.4.bin.WIN64/include -I../glew-1.10.0/include -Wno-write-strings -std=c++11
#LFLAGS := ../glew-1.10.0/lib/Release/x64/glew32s.lib -L../glfw-3.0.4.bin.WIN64/lib-mingw -lglfw3 -luser32 -lopengl32 -lgdi32 -static-libgcc -static-libstdc++ -s

rose: main.o translate.o $(patsubst parser/%.cpp,%.o,$(wildcard parser/*.cpp))
	$(CC) $^ $(LFLAGS) -o rose

%.o: %.cpp Makefile
	$(CC) $(CFLAGS) $< -c

%.o: parser/%.cpp parser Makefile
	$(CC) $(CFLAGS) $< -c

main.o: main.cpp shaders.h translate.h

translate.o: translate.cpp ast.h symbol_linking.h interpret.h code_generator.h bytecode.h parser

parser: rose.sablecc
	java -jar sablecc.jar -t cxx -d parser rose.sablecc
	touch parser

clean:
	rm -f *.o rose
