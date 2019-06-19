;              T         T               T

DEGREES	=	16384

	section	Code,code

MakeSinus:
	; A0 = Sinus
	clr.w	(a0)+
	lea.l	DEGREES/2*2-2(a0),a1
	clr.w	(a1)

	moveq.l	#1,d7
.loop:
	move.w	d7,d1
	mulu.w	d7,d1
	lsr.l	#8,d1

	move.w	#2373,d0
	mulu.w	d1,d0
	swap.w	d0
	neg.w	d0
	add.w	#21073,d0
	mulu.w	d1,d0
	swap.w	d0
	neg.w	d0
	add.w	#51469,d0
	mulu.w	d7,d0
	lsr.l	#8,d0
	lsr.l	#5,d0

	move.w	d0,(a0)+
	move.w	d0,-(a1)
	neg.w	d0
	move.w	d0,DEGREES/2*2(a1)
	move.w	d0,DEGREES/2*2-2(a0)

	addq.w	#1,d7
	cmp.w	#DEGREES/4,d7
	blt.b	.loop

	neg.w	d0
	move.w	d0,-(a1)
	neg.w	d0
	move.w	d0,DEGREES/2*2(a1)
	rts
