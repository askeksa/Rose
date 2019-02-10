;              T         T               T

	include	Rose:RoseConfig.S
	include	Rose:Sinus.S

	rsreset
	; x,y,size,tint must be first
st_x	rs.l	1
st_y	rs.l	1
st_size	rs.l	1
st_tint	rs.l	1
st_rand	rs.l	1
st_dir	rs.l	1
st_time	rs.l	1
st_proc	rs.l	1	; Must be last
st_SIZE	rs.l	0


STATE_SIZE	=	st_SIZE+(MAX_STACK+2)*4


	rsreset

; Data pointers
r_ColorScript	rs.l	1
r_Constants	rs.l	1
r_Bytecode	rs.l	1
r_BytecodeEnd	rs.l	1

; Chip memory pointers
r_Circles	rs.l	1
r_Screen	rs.l	1
r_Coppers	rs.l	1
r_CoppersEnd	rs.l	1

; Display state
r_VBlank	rs.l	1
r_FrameCounter	rs.l	1
r_CopperWrite	rs.l	1
r_CopperLimit	rs.l	1

; Circle state
r_FreeCircle	rs.l	1
r_CircleTable	rs.w	MAXRADIUS+1
r_SquareTable	rs.w	MAXRADIUS+1
r_CircleBufPtr	rs.l	SCREENHEIGHT
r_CircleBuffer	rs.b	MAX_CIRCLES*(CIRCLEDATA+4)
r_CircleBufEnd	rs.b	0

; Engine state
r_FreeState	rs.l	1	; Last longword of first free state
r_Procedures	rs.l	256
r_StateLists	rs.l	MAX_FRAMES+MAX_WAIT
r_StateSpace	rs.b	4+MAX_TURTLES*STATE_SIZE
r_Sinus	rs.w	DEGREES
r_Instructions	rs.b	CODEBUFFER

; More display state
	rs.l	1
r_CopperPtr:	rs.l	MAX_FRAMES

r_SIZE	rs.b	0

ROSE_FASTSIZE	=	r_SIZE
