;              T         T               T

	ifnd	MAX_FRAMES

MAX_FRAMES	=	10000	; Length of Rose animation
MAX_CIRCLES	=	200	; Max circles drawn in a frame

MAX_TURTLES	=	300	; Max turtles alive at the same time
MAX_STACK	=	20	; Max depth of execution stack
MAX_WAIT	=	1000	; Max wait beyond end of program
WIRE_CAPACITY	=	0	; Number of wire slots

CODEBUFFER	=	150000	; Space set aside for translated code
COPPERBUFFER	=	32000	; Space set aside for coppers

USERANDOM	=	1
USEBIGMOVE	=	1

CLEARMEM	=	0

SCREENWIDTH	=	512
SCREENHEIGHT	=	180
SCREENDEPTH	=	4
SCREENSIZE	=	SCREENWIDTH*SCREENHEIGHT/8*SCREENDEPTH

DISPLAYWIDTH	=	320
GUARDBAND	=	144
LEFTPOS	=	129
TOPPOS	=	82

MAXRADIUS	=	70
CIRCLEDATA	=	10
MARGIN	=	100

BUFFERMARGIN	=	MAX_CIRCLES*68+28
ROSE_CHIPSIZE	=	$20000+(COPPERBUFFER&$10000)+SCREENSIZE

	endc
