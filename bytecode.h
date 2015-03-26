#pragma once

#include <string>

static inline int verify(int base, int v, int max, const char *what) {
	if (v < 0 || v > max) {
		throw Exception(std::string("Argument out of range for ") + what + " instruction: " + std::to_string(v));
	}
	return base + v;
}

#define BC_DONE  0x00
#define BC_ELSE  0x01
#define BC_END   0x02
#define BC_RAND  0x03
#define BC_DRAW  0x04
#define BC_TAIL  0x06
#define BC_MUL   0x07
#define BC_SEED  0x08
#define BC_DIV   0x09
#define BC_WAIT  0x0A
#define BC_SINE  0x0B
#define BC_NEG   0x0D
#define BC_MOVE  0x0E
#define BC_WHEN(cond)  (verify(0x10, cond,  15, "WHEN"))
#define BC_FORK(nargs) (verify(0x20, nargs, 15, "FORK"))
#define BC_OP(op)      (verify(0x30, op,    15, "OP"))
#define BC_WSTATE(i)   (verify(0x40, i,     15, "WSTATE"))
#define BC_RSTATE(i)   (verify(0x50, i,     15, "RSTATE"))
#define BC_POP(n)      (verify(0x60, n,     15, "POP"))
#define BC_LOCAL(i)    (verify(0x70, i,     15, "LOCAL"))
#define BC_CONST(i)    (verify(0x80, i,    127, "CONST"))

// Negated condition branch nibble
#define CMP_EQ   6
#define CMP_NE   7
#define CMP_LT  12
#define CMP_GE  13
#define CMP_LE  14
#define CMP_GT  15

// Instruction high nibble
#define OP_OR     8
#define OP_SUB    9
#define OP_CMP   11
#define OP_AND   12
#define OP_ADD   13

// RSTATE and WSTATE offsets
#define ST_X    8
#define ST_Y    9
#define ST_SIZE 10
#define ST_TINT 11
#define ST_RAND 12
#define ST_DIR  13
#define ST_TIME 14
#define ST_PROC 15

// Special stack height to mark tail call
#define STACK_AFTER_TAIL 0x7A17

typedef unsigned char bytecode_t;

static inline int stack_change(bytecode_t bc) {
	static const int single[15] = { 0,0,0,1,0,0,0,-1,-1,-1,-1,0,0,0,-1 };
	int arg = bc & 15;
	switch (bc >> 4) {
	case 0: // Misc
		return single[bc];
	case 1: // WHEN
	case 3: // OP
	case 4: // WSTATE
		return -1;
	case 5: // RSTATE
	case 7: // LOCAL
		return 1;
	case 2: // FORK
		return -(arg + 1);
	case 6: // POP
		return -arg;
	default: // CONST
		return 1;
	}
}

