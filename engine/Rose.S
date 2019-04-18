;              T         T               T


	include	Rose:Engine.S

	ifnd	COMPATIBILITY
COMPATIBILITY	set	1
	endc

	section	Code,code

reg	set	$000
regval	macro
	dc.w	\1-reg,\2
reg	set	\1+2
	endm

RegisterData:
	regval	$02e,$0002
	regval	$042,$0000
	regval	$044,$ffff
	regval	$046,$0000
	regval	$08e,(TOPPOS<<8)+LEFTPOS
	regval	$090,((TOPPOS+SCREENHEIGHT-$100)<<8)+(LEFTPOS+DISPLAYWIDTH-$100)
	regval	$092,((LEFTPOS>>4)-1)<<3
	regval	$094,((LEFTPOS>>4)+(DISPLAYWIDTH>>4)-2)<<3
	if	COMPATIBILITY=2
	regval	$100,$4600
	else
BPLCON0:	regval	$100,$7600
	endc
	regval	$102,$0000
	regval	$104,$0040
	if	COMPATIBILITY>=1
	regval	$106,$0800
	else
	regval	$106,$0000
	endc
	regval	$108,(SCREENWIDTH-DISPLAYWIDTH)/8
	regval	$10a,(SCREENWIDTH-DISPLAYWIDTH)/8
	if	COMPATIBILITY>=1
	regval	$10c,$0800
	endc
	if	COMPATIBILITY<=1
	regval	$118,$ffff
	regval	$11a,$0000
	endc
	regval	$180,$0000
	if	COMPATIBILITY>=1
	regval	$1fc,$0000
	endc
RegisterData_End:


RoseInit:
	; A1 = Color Script
	; A2 = Constants
	; A3 = Bytecode
	; A5 = Rose Chip Space
	; A6 = Rose Space

	; Clear memory

	move.l	a5,a0
	move.l	#ROSE_CHIPSIZE/4,d0
.cloop1:	clr.l	(a0)+
	subq.l	#1,d0
	bgt.b	.cloop1

	move.l	a6,a0
	move.l	#ROSE_FASTSIZE/4,d0
.cloop2:	clr.l	(a0)+
	subq.l	#1,d0
	bgt.b	.cloop2

	; Initialize code pointers

	lea	PutCircle(pc),a0
	move.l	a0,r_PutCircle(a6)

	; Initialize data pointers

	move.l	a1,r_ColorScript(a6)
	move.l	a2,r_Constants(a6)
	move.l	a3,r_Bytecode(a6)

	; Initialize chip mem pointers

	move.l	a5,d0

	move.l	d0,d1
	add.l	#$10000-1,d1
	and.l	#-$10000,d1
	move.l	d1,r_Circles(a6)

	move.l	d1,d2
	add.l	#$10000,d2
	move.l	d2,r_Screen(a6)
	add.l	#SCREENSIZE,d2

	move.l	d2,a0
	cmp.w	#(-COPPERBUFFER)&$FFFF,d0
	bhi.b	.copperslast
	if	COPPERBUFFER&$10000
	addq.w	#1,r_Circles(a6)
	addq.w	#1,r_Screen(a6)
	endc
	move.l	a5,a0
.copperslast:
	move.l	a0,r_Coppers(a6)
	move.l	#-2,(a0)+
	move.l	a0,r_CopperLimit(a6)
	move.l	#-2,(a0)+
	move.l	a0,r_CopperWrite(a6)

	add.l	#COPPERBUFFER-MARGIN-4,a0
	move.l	a0,r_CoppersEnd(a6)

	; Initialize engine and circles

	bsr.w	InitEngine
	bsr.w	MakeCircles

	; Flush cache if possible

	if	COMPATIBILITY>=1
	move.l	a6,-(a7)
	move.l	$4.w,a6
	if	COMPATIBILITY=1
	cmp.w	#37,20(a6)
	blo.b	.noflush
	endc
	jsr	-636(a6)
.noflush:	move.l	(a7)+,a6
	endc

CopperDanger:
	move.w	#$0000,$dff02e

	; Pre-fill copper buffer
PreQueue:
	move.l	#-1,r_VBlank(a6)
	moveq.l	#0,d7
PreLoop:
	bsr.w	Body

	move.l	r_CoppersEnd(a6),a0
	sub.w	#BUFFERMARGIN,a0
	cmp.l	a0,a3
	bhs.b	.done

	addq.l	#1,d7
	cmp.l	#MAX_PREFRAMES-2,d7
	blt.b	PreLoop
.done:
	move.l	d7,r_FrameCounter(a6)
	rts

	
Body:
	bsr.w	InitCircleBuffer
	bsr.w	RunFrame

	move.l	r_CopperWrite(a6),a3
	move.l	a6,a4
	add.l	#r_CopperPtr,a4

	COPBUFSTALL

	move.l	a3,-(a7)

	bsr.w	CollectCircles
	move.l	#-2,(a3)+

	move.l	d7,d0
	lsl.l	#2,d0
	move.l	(a7)+,(a4,d0.l)

	move.l	a3,r_CopperWrite(a6)
	rts

RoseMain:
	; A6 = Rose Space
	lea.l	$dff000,a3
	move.l	r_Screen(a6),$048(a3)
	move.l	r_Circles(a6),$050(a3)
	move.l	r_Screen(a6),$054(a3)
	move.l	r_Coppers(a6),a0
	move.l	a0,$080(a3)
	addq.l	#4,a0
	move.l	a0,$084(a3)
	move.w	a0,$088(a3)

	lea	RegisterData(pc),a0
	if	COMPATIBILITY=1
	btst.b	#1,$004(a3)
	beq.b	.noaga
	move.w	#$4600,BPLCON0+2-RegisterData(a0)
.noaga:	endc
	moveq.l	#(RegisterData_End-RegisterData)/4-1,d7
.copperloop:	add.w	(a0)+,a3
	move.w	(a0)+,(a3)+
	dbf	d7,.copperloop

	move.l	r_FrameCounter(a6),d7
MainLoop:
	bsr.w	Body

	addq.l	#1,d7
	move.l	d7,r_FrameCounter(a6)
	cmp.l	#MAX_FRAMES-1,d7
	blt.b	MainLoop

.exitwait:	cmp.l	r_VBlank(a6),d7
	bgt.b	.exitwait
	rts


RoseInterrupt:
	; A6 = Rose Space
	lea.l	$dff000,a3
	move.l	a6,a4
	add.l	#r_CopperPtr,a4

	; Set current copperlist as write limit
	lea.l	r_CopperLimit(a6),a0
	move.l	r_VBlank(a6),d0
	cmp.l	#MAX_FRAMES,d0
	bge.b	.lastcopper
	lsl.l	#2,d0
	move.l	(a4,d0.l),d1
	beq.b	.lastcopper
	move.l	d1,(a0)
.lastcopper:
	; Set bitplane pointers
	move.l	r_Screen(a6),a1
	add.w	#GUARDBAND/8,a1
	lea.l	$0e0(a3),a0
	move.l	a1,(a0)+
	add.w	#SCREENWIDTH*SCREENHEIGHT/8,a1
	move.l	a1,(a0)+
	add.w	#SCREENWIDTH*SCREENHEIGHT/8,a1
	move.l	a1,(a0)+
	add.w	#SCREENWIDTH*SCREENHEIGHT/8,a1
	move.l	a1,(a0)+

	; Next frame
	addq.l	#1,r_VBlank(a6)
	beq.b	.colorsdone

	; Execute color script
	if	COMPATIBILITY<=1
	lea.l	$188(a3),a0
	else
	lea.l	$190(a3),a0
	endc
	if	COMPATIBILITY=1
	btst.b	#1,$004(a3)
	beq.b	.noaga
	addq.l	#8,a0
.noaga:	endc
	move.l	r_ColorScript(a6),a1
	addq.w	#1,(a1)+
	bmi.b	.colorsdone
.putcolor:	move.w	(a1)+,d2
	move.w	d2,d1
	rol.w	#5,d1
	and.w	#14,d1
	move.w	d2,(a0,d1.w)
	tst.w	(a1)
	bpl.b	.putcolor
	move.l	a1,r_ColorScript(a6)
.colorsdone:

	; Set next copperlist
	move.l	r_Coppers(a6),a0
	move.l	r_VBlank(a6),d0
	cmp.l	#MAX_FRAMES,d0
	bge.b	.nocopper
	lsl.l	#2,d0
	move.l	(a4,d0.l),d1
	beq.b	.nocopper
	move.l	d1,a0
.nocopper:	move.l	a0,$080(a3)
	rts
