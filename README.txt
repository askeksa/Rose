Rose - a programming language for temporal turtle graphics


INTRODUCTION

A Rose program produces an animation drawn by a number of "turtles"
moving around on a canvas. Each turtle has the following state:

- Position (x and y)
- Direction
- Pen size
- Color index, called "tint"

Unless specific otherwise, all numeric values in Rose are 16.16 bits
fixed-point values.

The position and direction are available as global variables x, y and dir.


SYNTAX

Rose is whitespace-agnostic: line breaks can be inserted or omitted anywhere
between tokens.

Just for the fun of it, and because I enjoy using a thesaurus, all keywords
in Rose have four letters.

A Rose program consists of two parts - a color script and a set of procedures.
The color script has the syntax:

- plan <event>*

where <event> is one of:

- wait <expression>
  Wait a number of frames. The expression cannot contain variables.
- <tint>:<color>
  Set the given tint (integer) to the given color (three-digit hex value).

A procedure has the syntax:

- proc <name> <param>* <statement>*

where <name> and <param> are identifiers, and <statement> is one of:

- move <expression>
  Move forward a number of pixels.
- turn <expression>
  Turn a number of 256-on-a-circle degrees clockwise.
- face <expression>
  Turn to a number of 256-on-a-circle degrees clockwise from horizontal right.
- size <expression>
  Set pen radius in pixels (rounded to nearest integer-and-a-half).
- tint <expression>
  Set the current tint (rounded down to an integer).
- draw
  Draw a circular dot at the current position.
- wait <expression>
  Wait a number of frames.
- fork <procedure> <expression>*
  Branch off a new turtle with a copy of this turtle's state, running the
  given procedure with the given arguments. This turtle continues executing
  the rest of the current procedure in parallel.
- temp <variable> = <expression>
  Assign a value to a local variable.
- when <expression> <statement>* done
  Run the block of statements if the expression evaluates to non-zero.
- when <expression> <statement>* else <statement>* done
  Run the first block of statements if the expression evaluates to non-zero,
  or the second block of statements otherwise.

An <expression> is one of:

- <digit>+[.<digit>+]
  Constant value.
- <name>
  Procedure or value of parameter, temporary variable or global variable.
- <expression> op <expression>
  Operation, where op is one of (*, /, +, -, ==, !=, <, <=, >, >=, &, |).
  Operator semantics and precedence are as in C, except * and / which do
  fixed-point adjustment to match fixed-point multiplication and division.
- ~ <expression>
  Negate value. Used instead of - to enable airy, delimiter-sparse syntax.
- ( <expression> )
  For grouping.

Procedures can be assigned to temporaries and passed as arguments to
procedures. The procedure part of a fork statement can refer directly to
a procedure or to a parameter or local variable containing a procedure.

And just to mention it one more time, because this could be a common
pitfall: Remember to use ~ and not - for negation, including for
writing negative numbers!


THE VISUALIZER

Run the visualizer with a Rose program as argument. It will continuously
monitor the file and reload it whenever its modification time changes.

Keyboard shortcuts:
- SPACE: start/stop animation.
- RIGHT/LEFT: Step one frame forward/backward.
- PGDOWN/PGUP: Step 50 frames forward/backward.
- BACKSPACE: Go back to frame animation was last started.
- HOME: Go to first frame.
- ESCAPE: Quit the visualizer.

Clicking or holding the left mouse button sets the time proportionally to
the mouse X position within the window, up to 10000 frames (3m20s at 50fps).

