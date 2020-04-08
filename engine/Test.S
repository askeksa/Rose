;              T         T               T

	include	"lvo/exec_lib.i"
	include	"lvo/dos_lib.i"

MODE_OLDFILE	=	1005

COMPATIBILITY	=	1
FASTMEM	=	0
RMBPAUSE	=	0
FPUINT	=	0
COPPER	=	1
BLITTER	=	1
SPRITE	=	0
TOPAZ	=	0
SECTIONHACK	=	-1

	section	Code,code
	include	Inc:DemoStartup.S

	include	Rose:Rose.S
	include	Rose:Sinus.S


	section	Data,data

ColorScriptName:
	dc.b	"colorscript.bin",0
BytecodeName:
	dc.b	"bytecodes.bin",0
ConstantsName:
	dc.b	"constants.bin",0
DosName:
	dc.b	"dos.library",0
	even

	section	Fast,bss

ColorScript:
	ds.w	10000
ColorScript_End:
Bytecode:
	ds.b	20000
Bytecode_End:
Constants:
	ds.l	382
Constants_End:


	section	Fast,bss

RoseSpace:	ds.b	ROSE_FASTSIZE
RoseSinus:	ds.w	DEGREES

	section	Chip,bss_c

RoseChip:	ds.b	ROSE_CHIPSIZE


	section	Code,code

LoadFile:
	; A2 = Name
	; D4 = Address
	; D3 = Length

	lea.l	DosName,a1
	move.l	$4.w,a6
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,a6

	move.l	a2,d1
	move.l	#MODE_OLDFILE,d2
	jsr	_LVOOpen(a6)
	move.l	d0,a2

	move.l	a2,d1
	move.l	d4,d2
	jsr	_LVORead(a6)
	move.l	d0,d2

	move.l	a2,d1
	jsr	_LVOClose(a6)

	move.l	a6,a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)

	move.l	d2,d0
	rts

_Precalc:
	lea.l	RoseSinus,a0
	bsr.w	MakeSinus

	lea.l	Bytecode,a3
	lea.l	BytecodeName,a2
	move.l	a3,d4
	move.l	#Bytecode_End-Bytecode,d3
	bsr	LoadFile

	lea.l	ConstantsName,a2
	move.l	#Constants,d4
	move.l	#Constants_End-Constants,d3
	bsr	LoadFile

	lea.l	ColorScriptName,a2
	move.l	#ColorScript,d4
	move.l	#ColorScript_End-ColorScript,d3
	bsr	LoadFile
	lea.l	ColorScript,a1

	lea.l	Constants,a2
	lea.l	RoseSinus,a4
	lea.l	RoseChip,a5
	lea.l	RoseSpace,a6
	bsr.w	RoseInit
	rts

_Main:
	; Main demo routine, called by the startup.
	; Demo will quit when this routine returns.
	lea.l	RoseSpace,a6
	bsr.w	RoseMain
	rts

_Interrupt:
	; Called by the vblank interrupt.
	lea.l	RoseSpace,a6
	tst.l	r_Ready(a6)
	beq.b	.notready
	; Skip all vblank tasks (including music) until Rose is ready.
	bsr.w	RoseInterrupt
.notready:	rts

_Exit:
	rts
