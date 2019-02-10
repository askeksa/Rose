;              T         T               T

	include	Rose:RoseSpace.S


	section	Code,code

MakeCircles:
	move.l	r_Circles(a6),a1
	lea	r_CircleTable(a6),a2
	lea	r_SquareTable(a6),a3
	moveq.l	#0,d7
.radiusloop:	move.w	a1,(a2)+
	move.w	d7,d0
	add.w	d0,d0

	move.w	d7,d5
	asr.w	#3,d5
	addq.w	#1,d5	; Width in words
	muls.w	d0,d5
	move.w	a1,(a3)
	add.w	d5,(a3)+	; Pointer to center line

	addq.w	#1,d0
	muls.w	d0,d0	; Radius^2
	move.w	d7,d6
	neg.w	d6	; Y
.yloop:	move.w	d6,d1
	muls.w	d1,d1	; Y^2
	move.w	d7,d2
	muls.w	d2,d2
	add.w	d2,d1	; X^2 + Y^2
	asl.w	#2,d1
	sub.w	d0,d1	; X^2 + Y^2 - R^2
	move.w	d7,d2
	add.w	d2,d2
	subq.w	#1,d2	; Delta
	asl.w	#2,d2

	move.w	d7,d5
	asr.w	#3,d5	; Width in words - 1
.wloop:	moveq.l	#16-1,d4
.ploop:	move.w	d1,d3
	add.l	d3,d3
	sub.w	d2,d1
	subq.w	#8,d2
	dbf	d4,.ploop
	swap.w	d3
	move.w	d3,(a1)+
	dbf	d5,.wloop

	addq.w	#1,d6
	cmp.w	d7,d6
	ble.b	.yloop

	addq.w	#1,d7
	cmp.w	#MAXRADIUS,d7
	ble.b	.radiusloop
	rts


InitCircleBuffer:
	lea	r_CircleBufPtr(a6),a2
	move.w	#SCREENHEIGHT-1,d5
.lineloop:	clr.l	(a2)+
	dbf	d5,.lineloop
	lea	r_CircleBufEnd(a6),a4
	move.l	a4,r_FreeCircle(a6)
	rts


PutCircle:
	; D0 = X
	; D1 = Y
	; D2 = Radius
	; D3 = Color

	; Cull to left and top
	add.w	d2,d0
	bmi.b	.outside
	add.w	d2,d1
	bmi.b	.outside

	; Cull to right
	add.w	d2,d2
	sub.w	d2,d0
	cmp.w	#DISPLAYWIDTH,d0
	bge.w	.outside

	move.w	d2,d4
	lea	r_CircleTable(a6),a2
	move.w	(a2,d2.w),d6
	lea	r_SquareTable(a6),a2
	move.w	(a2,d2.w),d5
	lsr.w	#4,d2
	addq.w	#1,d2	; Circle Width in words

	; Clip to top
	sub.w	d4,d1
	bpl.b	.notopclip
	add.w	d1,d4
	muls.w	d2,d1
	add.w	d1,d1
	sub.w	d1,d6
	clr.w	d1
.notopclip:
	addq.w	#1,d4	; Blit Height

	; Make square?
	tst.w	d3
	bpl.b	.circle
	move.w	d5,d6
.circle:
	; Clip and cull to bottom
	move.w	#SCREENHEIGHT,d5
	sub.w	d1,d5
	sub.w	d4,d5
	bpl.b	.nobottomclip
	add.w	d5,d4
	ble.w	.outside
.nobottomclip:
	; Put circle
	lea	r_FreeCircle(a6),a2
	move.l	(a2),a1
	subq.l	#4,a1
	movem.w	d0/d2/d3/d4/d6,-(a1)
	move.l	a1,(a2)
	lea	r_CircleBufPtr(a6),a2
	lsl.w	#2,d1
	add.w	d1,a2
	move.l	(a2),CIRCLEDATA(a1)
	move.l	a1,(a2)
.outside:
	rts


COPBUFSTALL	macro
	cmp.l	r_CoppersEnd(a6),a3
	bls.b	.stall\@
	move.l	#$008A0000,(a3)+
	move.l	r_Coppers(a6),a3
	addq.l	#4,a3
.stall\@:
	move.l	r_CopperLimit(a6),d1
	sub.l	a3,d1
	cmp.l	#MARGIN,d1
	bls.b	.stall\@
	endm

CollectCircles:
	; A3 = Destination copper instructions
	; A4 = Copper pointers

	lea	r_CircleBufPtr(a6),a2
	suba.l	a0,a0
.lineloop:
	move.l	(a2)+,a1
	bra.w	.circlelend
.circleloop:
	COPBUFSTALL

	movem.w	(a1)+,d0/d2/d3/d4/a5

	moveq.l	#-2,d1
	tst.w	d3
	bpl.b	.circle
	sub.w	d2,d1
	sub.w	d2,d1
	not.w	d3
.circle:
	add.w	#GUARDBAND,d0	; X on banded screen
	addq.w	#1,d2	; Blit width

	lsl.w	#6,d4
	add.w	d2,d4	; Blit Size

	moveq.l	#SCREENWIDTH/8,d5
	add.w	d2,d2	; Width in bytes
	sub.w	d2,d5	; C/D modulo

	moveq.l	#15,d6
	and.w	d0,d6	; Shift
	ror.w	#4,d6
	or.w	#$0B0A,d6	; C and not A
	
	asr.w	#4,d0	; Word offset

	add.w	d0,d0
	add.w	a0,d0	; Dest ptr

	move.l	#$00010000,(a3)+; wait for blitter

	move.w	#$060,(a3)+
	move.w	d5,(a3)+
	move.w	#$064,(a3)+
	move.w	d1,(a3)+
	move.w	#$066,(a3)+
	move.w	d5,(a3)+

	roxr.w	#1,d3
	subx.w	d5,d5
	and.w	#$F0,d5	; if color bit set then A, else 0
	or.w	d6,d5	; if color bit set then C or A, else C and not A

	move.w	#$040,(a3)+
	move.w	d5,(a3)+
	move.w	#$04A,(a3)+
	move.w	d0,(a3)+
	move.w	#$052,(a3)+
	move.w	a5,(a3)+
	move.w	#$056,(a3)+
	move.w	d0,(a3)+
	move.w	#$058,(a3)+
	move.w	d4,(a3)+

	move.l	#$00010000,(a3)+; wait for blitter
	move.l	#$00010000,(a3)+; wait for blitter

	add.w	#SCREENWIDTH*SCREENHEIGHT/8,d0

	roxr.w	#1,d3
	subx.w	d5,d5
	and.w	#$F0,d5	; if color bit set then A, else 0
	or.w	d6,d5	; if color bit set then C or A, else C and not A

	move.w	#$040,(a3)+
	move.w	d5,(a3)+
	move.w	#$04A,(a3)+
	move.w	d0,(a3)+
	move.w	#$052,(a3)+
	move.w	a5,(a3)+
	move.w	#$056,(a3)+
	move.w	d0,(a3)+
	move.w	#$058,(a3)+
	move.w	d4,(a3)+

	move.l	#$00010000,(a3)+; wait for blitter

	move.l	(a1),a1
.circlelend:
	move.l	a1,d0
	bne.w	.circleloop

	add.w	#SCREENWIDTH/8,a0
	cmp.w	#SCREENHEIGHT*(SCREENWIDTH/8),a0
	blo.w	.lineloop
	rts


