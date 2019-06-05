;              T         T               T

	include	Rose:Circles.S


; Saved state:
; Next
; State
; Stack...

; Engine registers:
; D0 = Top of stack
; A0 = Sinus
; A3 = Stack
; A4 = Proc/const
; A5 = State

BC_PROC	=	$07
MIN_INPUT	=	$08
MAX_INPUT	=	$5F
SINGLE_SNIP	=	6
END_OF_SCRIPT	=	$FF


	section	Code,code

RunFrame:
	; A6 = Rose Space
	; D7 = Frame
	move.l	r_Sinus(a6),a0
	move.l	r_Constants(a6),a4
.frame:
	move.l	d7,d0
	lsl.l	#2,d0
	lea	r_StateLists(a6),a1
	add.l	d0,a1
	tst.l	(a1)
	beq.b	.done
	move.l	(a1),a3
	move.l	(a3)+,(a1)
	move.l	(a3)+,a5
	move.l	(a5),a1 ; st_proc
	jsr	(a1)
	bra.b	.frame
.done:	rts


InitStates:
	move.l	a6,a1
	add.l	#r_StateSpace,a1
	move.w	#MAX_TURTLES-1,d7
.state:	lea.l	STATE_SIZE(a1),a2
	move.l	a1,(a2)
	move.l	a2,a1
	dbf	d7,.state
;	move.l	a2,r_FreeState(a6)

InitMainTurtle:
	move.l	a6,a1
	add.l	#r_Instructions,a1
;	move.l	r_FreeState(a6),a2
	move.l	(a2),r_FreeState(a6)
	move.l	a1,(a2)
	if	USERANDOM
	move.l	#$BABEFEED,st_rand(a2)
	endc
	move.l	a2,d2
	move.l	d2,-(a2)
	lea	r_StateLists(a6),a1
	move.l	(a1),-(a2)
	move.l	a2,(a1)
	rts

InitEngine:
	; A6 = Rose Space

TranslateBytecode:
	move.l	r_Bytecode(a6),a0
	lea.l	r_Procedures(a6),a4
	move.l	a6,a1
	add.l	#r_Instructions,a1

	clr.w	d1
.procloop:
	cmp.b	#END_OF_SCRIPT,(a0)
	beq.b	InitStates
	move.l	a1,(a4)+	; Write proc address
.instloop:
	cmp.b	#MIN_INPUT,(a0)
	blo.b	.noinput
	cmp.b	#MAX_INPUT+1,(a0)
	blo.b	.input
.noinput:
	lsr.b	#1,d1
	bcc.b	.nopush
	move.w	#$2700,(a1)+	; move.l d0,-(a3)
.nopush:	bra.b	.nopop
.input:
	lsr.b	#1,d1
	bcs.b	.nopop
	move.w	#$201B,(a1)+	; move.l (a3)+,d0
.nopop:
	clr.w	d0
	move.b	(a0)+,d0
	bmi.b	.const

	moveq.l	#$F,d1
	and.b	d0,d1
	lsr.b	#4,d0
	bne.b	.notsingle
	subq.b	#2,d1
	bhs.b	.single
	; Get jump target
	move.l	(a7)+,a2
	addq.b	#2,d1
	beq.b	.done
	; Put jump
	move.w	#$6000,d1
	move.w	d1,(a1)+
	move.l	a1,-(a7)
	clr.w	(a1)+
.done:	; Put jump offset
	move.w	a1,d2
	sub.w	a2,d2
	move.w	d2,(a2)
	bra.b	.instloop
.single:
	move.b	d1,d0
	addq.b	#SINGLE_SNIP,d0
	bsr.b	PutSnip
	tst.b	d1
	beq.b	.procloop
	cmp.b	#BC_PROC-2,d1
	bne.b	.instloop
	move.b	(a0)+,d0
	lsl.w	#2,d0
	add.w	d0,-2(a1)
	bra.b	.instloop
.notsingle:
	subq.b	#2,d0
	bhs.b	.notwhen
	; Put condition into second highest nibble of branch word
	; 6 = ne, 7 = eq, 12 = ge, 13 = lt, 14 = gt, 15 = le
	ror.w	#8,d1
	or.w	#$6000,d1
	move.w	d1,(a1)+
	move.l	a1,-(a7)
	clr.w	(a1)+
	bra.b	.instloop
.notwhen:
	bsr.b	PutSnip

	subq.b	#2,d0
	blo.b	.fork_or_op
	lsr.b	#1,d0
	bcs.b	.longword
	not.w	d1
	bra.b	.longword
.fork_or_op:
	moveq.l	#fork_nargs+1-fork_end,d2
	lsl.b	#2,d1
	addq.b	#2,d0
	beq.b	.not_op
	moveq.l	#-2,d2
	lsl.b	#2,d1
.not_op:	move.b	d1,(a1,d2.w)
	bra.b	.outifodd
.const:
	moveq.l	#127,d1
	and.b	d0,d1
	moveq.l	#1,d0
	move.w	#$202C,(a1)+	; move.l x(a4),d0
.longword:
	lsl.w	#2,d1
	move.w	d1,(a1)+
.outifodd:
	move.b	d0,d1
	bra.w	.instloop

PutSnip:
	lea	Snipoffs(pc),a2
	lea	Snips(pc),a3
	add.w	d0,a2
	clr.w	d3
	move.b	(a2)+,d3
	move.b	(a2)+,d2
	sub.b	d3,d2
	beq.b	.empty
	add.w	d3,a3
	add.w	d3,a3
.copy:	move.w	(a3)+,(a1)+
	subq.b	#1,d2
	bne.b	.copy
.empty:	rts


Snips:

Snip_fork:
	;move.l	(a3)+,d0
	; Put nargs in moveq byte
	move.l	r_FreeState(a6),a2
	move.l	(a2),r_FreeState(a6)
	lea	4(a5),a1
	move.l	a2,d2
	move.l	d0,(a2)+
	rept	7+MAX_WIRES
	move.l	(a1)+,(a2)+
	endr
	move.l	d2,a2
fork_nargs:
	moveq.l	#0,d1
	sub.w	d1,a2
	move.l	a2,a1
	bra.b	.args_in
.args:	move.l	(a3)+,(a1)+
.args_in:	subq.w	#4,d1
	bge.b	.args
	move.l	d2,-(a2)
	clr.l	d0
	move.w	st_time(a5),d0
	lsl.l	#2,d0
	lea	r_StateLists(a6),a1
	add.l	d0,a1
	move.l	(a1),-(a2)
	move.l	a2,(a1)
fork_end:
Snip_op:
	; Put op in high nibble of op word:
	; 8 = or, 9 = sub, 11 = cmp, 12 = and, 13 = add
	;move.l	(a3)+,d0
	move.l	(a3)+,d1
	or.l	d1,d0
	;move.l	d0,-(a3)

Snip_writelocal:
	; Put index*4 in offset
	;move.l	(a3)+,d0
	dc.w	$2B40	; move.l d0,x(a5)
Snip_writestate:
	; Put index*4 in offset
	;move.l	(a3)+,d0
	dc.w	$2B40	; move.l d0,x(a5)
Snip_readlocal:
	; Put index*4 in offset
	dc.w	$202D	; move.l x(a5),d0
	;move.l	d0,-(a3)
Snip_readstate:
	; Put index*4 in offset
	dc.w	$202D	; move.l x(a5),d0
	;move.l	d0,-(a3)

Snip_end:
	move.l	r_FreeState(a6),(a5)
	move.l	a5,r_FreeState(a6)
	rts
Snip_rand:
	if	USERANDOM
	move.l	st_rand(a5),d0
	move.l	d0,d1
	swap.w	d0
	mulu.w	#$9D3D,d1
	add.l	d1,d0
	move.l	d0,st_rand(a5)
	clr.w	d0
	swap.w	d0
	;move.l	d0,-(a3)
	endc
Snip_draw:
	movem.l	st_x(a5),d0-d3
	swap.w	d0
	swap.w	d1
	swap.w	d2
	swap.w	d3
	move.l	r_PutCircle(a6),a1
	jsr	(a1)
Snip_tail:
	move.l	(a5),a1 ; st_proc
	jmp	(a1)
Snip_plot:
	movem.l	st_x(a5),d0-d3
	swap.w	d0
	swap.w	d1
	swap.w	d2
	swap.w	d3
	not.w	d3
	move.l	r_PutCircle(a6),a1
	jsr	(a1)
Snip_proc:
	move.l	r_Procedures+0(a6),d0
Snip_pop:
	;move.l	(a3)+,d0
Snip_div:
	;move.l	(a3)+,d0
	move.l	(a3)+,d1
	asr.l	#8,d1
	divs.w	d1,d0
	ext.l	d0
	asl.l	#8,d0
	;move.l	d0,-(a3)
Snip_wait:
	;move.l	(a3)+,d0
	add.l	d0,st_time(a5)
	lea.l	.continue(pc),a1
	move.l	a1,(a5) ; st_proc
	move.l	a5,-(a3)
	clr.l	d0
	move.w	st_time(a5),d0
	lsl.l	#2,d0
	lea	r_StateLists(a6),a1
	add.l	d0,a1
	move.l	(a1),-(a3)
	move.l	a3,(a1)
	rts
.continue:
Snip_sine:
	;move.l	(a3)+,d0
	lsr.w	#2,d0
	add.w	d0,d0
	move.w	(a0,d0.w),d0
	ext.l	d0
	asl.l	#2,d0
	;move.l	d0,-(a3)
Snip_seed:
	if	USERANDOM
	;move.l	(a3)+,d0
	move.l	d0,d1
	swap.w	d0
	mulu.w	#$9D3D,d1
	add.l	d1,d0
	move.l	d0,d1
	swap.w	d0
	mulu.w	#$9D3D,d1
	add.l	d1,d0
	move.l	d0,st_rand(a5)
	endc
Snip_neg:
	;move.l	(a3)+,d0
	neg.l	d0
	;move.l	d0,-(a3)
Snip_move:
	;move.l	(a3)+,d0
	move.l	d0,d2
	move.l	st_dir(a5),d0
	asr.l	#8,d0
	lsr.w	#2,d0
	add.w	d0,d0
	move.w	(a0,d0.w),d1	; Sine
	move.l	st_dir(a5),d0
	asr.l	#8,d0
	add.w	#$4000,d0
	lsr.w	#2,d0
	add.w	d0,d0
	move.w	(a0,d0.w),d0	; Cosine
	if	USEBIGMOVE
	cmp.l	#32<<16,d2
	blt.b	.small
	asl.l	#2,d2
	swap.w	d2
	muls.w	d2,d0
	muls.w	d2,d1
	bra.b	.done
.small:	endc
	asr.l	#6,d2
	muls.w	d2,d0
	muls.w	d2,d1
	asr.l	#8,d1
	asr.l	#8,d0
.done:	add.l	d0,st_x(a5)
	add.l	d1,st_y(a5)
Snip_mul:
	;move.l	(a3)+,d0
	move.l	(a3)+,d1
	asr.l	#8,d1
	asr.l	#8,d0
	muls.w	d1,d0
	;move.l	d0,-(a3)

EndOfSnips:


SNIP	macro
	dc.b	(Snip_\1-Snips)/2
	endm

Snipoffs:
	SNIP	fork
	SNIP	op
	SNIP	writelocal
	SNIP	writestate
	SNIP	readlocal
	SNIP	readstate

	SNIP	end
	SNIP	rand
	SNIP	draw
	SNIP	tail
	SNIP	plot
	SNIP	proc
	SNIP	pop
	SNIP	div
	SNIP	wait
	SNIP	sine
	SNIP	seed
	SNIP	neg
	SNIP	move
	SNIP	mul

	dc.b	(EndOfSnips-Snips)/2
	even
