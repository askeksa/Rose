#pragma once

#include <string>

static inline int verify(int base, int v, int max, const char *what) {
	if (v < 0 || v > max) {
		throw Exception(std::string("Argument out of range for ") + what + " instruction: " + std::to_string(v));
	}
	return base + v;
}

// Instruction notes:
// i = input
// o = output
// j = jump
// t = jump target

#define BC_DONE  0x00                                      //    t
#define BC_ELSE  0x01                                      //   jt
#define BC_END   0x02                                      //
#define BC_RAND  0x03                                      //  o
#define BC_DRAW  0x04                                      //
#define BC_TAIL  0x05                                      //  o
#define BC_PLOT  0x06                                      //
#define BC_PROC  0x07                                      //  o
#define BC_POP   0x08                                      // i
#define BC_DIV   0x09                                      // io
#define BC_WAIT  0x0A                                      // i
#define BC_SINE  0x0B                                      // io
#define BC_SEED  0x0C                                      // i
#define BC_NEG   0x0D                                      // io
#define BC_MOVE  0x0E                                      // i
#define BC_MUL   0x0F                                      // io
#define BC_WHEN(cond)  (verify(0x10, cond,  15, "WHEN"))   // i j
#define BC_FORK(nargs) (verify(0x20, nargs, 15, "FORK"))   // i
#define BC_OP(op)      (verify(0x30, op,    15, "OP"))     // io
#define BC_WLOCAL(i)   (verify(0x40, i,     15, "WLOCAL")) // i
#define BC_WSTATE(i)   (verify(0x50, i,     15, "WSTATE")) // i
#define BC_RLOCAL(i)   (verify(0x60, i,     15, "RLOCAL")) //  o
#define BC_RSTATE(i)   (verify(0x70, i,     15, "RSTATE")) //  o
#define BC_CONST(i)    (verify(0x80, i,    126, "CONST"))  //  o

#define END_OF_SCRIPT 0xFF

// Negated condition branch nibble (for WHEN)
#define CMP_EQ   6
#define CMP_NE   7
#define CMP_LT  12
#define CMP_GE  13
#define CMP_LE  14
#define CMP_GT  15

// Instruction high nibble (for OP)
#define OP_OR    8
#define OP_SUB   9
#define OP_CMP  11
#define OP_AND  12
#define OP_ADD  13

// RSTATE and WSTATE offsets
#define ST_PROC  0
#define ST_X     1
#define ST_Y     2
#define ST_SIZE  3
#define ST_TINT  4
#define ST_RAND  5
#define ST_DIR   6
#define ST_TIME  7
#define ST_WIRE0 8

// Special stack height to mark tail call
#define STACK_AFTER_TAIL 0x7A17

typedef unsigned char bytecode_t;

static inline int stack_change(bytecode_t bc) {
	static const int single[16] = { 0,0,0,1,0,0,0,1,-1,-1,-1,0,-1,0,-1,-1 };
	int arg = bc & 15;
	switch (bc >> 4) {
	case 0: // Misc
		return single[bc];
	case 1: // WHEN
	case 3: // OP
	case 4: // WLOCAL
	case 5: // WSTATE
		return -1;
	case 6: // RLOCAL
	case 7: // RSTATE
		return 1;
	case 2: // FORK
		return -(arg + 1);
	default: // CONST
		return 1;
	}
}

