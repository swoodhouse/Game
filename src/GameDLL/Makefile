# Object files to create for the executable
DLL_OBJS = obj/attractors.o obj/game.o obj/GameDLL.o
WARNS = -Wall

# Names of tools to use when building
CC = x86_64-w64-mingw32-g++
DLL = GameDLL.dll

# Compiler flags
DLL_CFLAGS = -O2 -std=c++0x -Wall -IGameDLL -I../../ext/CUDD/cudd-3.0.0/cudd -I../../ext/CUDD/cudd-3.0.0/cplusplus

# Linker flags
#DLL_LDFLAGS = -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -Lcudd -Lobj -shared -s -Wl,--subsystem,windows,--out-implib,lib/libgame.a
#DLL_LDFLAGS = -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -Lcudd -Lobj -shared -s -Wl,--subsystem,windows,--out-implib,lib/libgame.a

#DLL_LDFLAGS = --verbose -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -l:libcudd.a -l:libobj.a -shared -s -Wl,--subsystem,windows,--out-implib,lib/libgame.a

#DLL_LDFLAGS = --verbose -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -l:libcudd.a -l:libobj.a -shared -s -Wl,--subsystem,windows,--out-implib,lib/libgame.a

#DLL_LDFLAGS = -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -l:libcudd.a -l:libobj.a -shared -Wl,--subsystem,windows,--out-implib,lib/libgame.a

DLL_LDFLAGS = -L../../ext/CUDD/cudd-3.0.0/cudd/.libs -L../../ext/CUDD/cudd-3.0.0/cplusplus/.libs -l:libobj.a -l:libcudd.a -shared -Wl,--subsystem,windows

.PHONY: all clean

all: bin/${DLL}

# Delete all build output
clean:
	if exist bin/* del /q bin/*
	if exist lib/* del /q lib/*
	if exist obj/* del /q obj/*

# Create build output directories if they don't exist
bin lib obj:

# Compile object files for DLL
obj/attractors.o: GameDLL/Attractors.cpp GameDLL/Attractors.h | obj
	${CC} ${DLL_CFLAGS} -c "$<" -o "$@"

obj/game.o: GameDLL/Game.cpp GameDLL/Game.h | obj
	${CC} ${DLL_CFLAGS} -c "$<" -o "$@"

obj/GameDLL.o: GameDLL/GameDLL.cpp | obj
	${CC} ${DLL_CFLAGS} -c "$<" -o "$@"

bin/${DLL}: ${DLL_OBJS} | bin lib
	${CC} -o "$@" ${DLL_OBJS} ${DLL_LDFLAGS}
