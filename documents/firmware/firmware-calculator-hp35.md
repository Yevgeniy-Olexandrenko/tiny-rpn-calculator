# Introduction

The HP-35 was HP's first pocket calculator. It was introduced at a time when most calculators (including expensive desktop models) had only the four basic functions. The HP-35 was the first pocket calculator with transcendental functions and the first with RPN.

[![HP-35](https://www.hpmuseum.org/3qs/35v33q.jpg)](https://www.hpmuseum.org/hp35.htm)

The calculator had a four register stack (**X**, **Y**, **Z** and **T**), the `ENTER↑` key pushed the displayed value (**X**) down the stack. Any binary operation popped the top two registers, and pushed the result. When the stack was popped the **T** register duplicated into the **Z** register.

Internally, the calculator was organized around a serial (one-bit) processor chipset made under contract by Mostek, processing 56-bit floating-point numbers, representing 14-digit BCD numbers.

The calculator used a traditional floating decimal display for numbers that could be displayed in that format, but automatically switched to scientific notation for other numbers. The fifteen-digit LED display was capable of displaying a ten-digit mantissa plus its sign and a decimal point and a two-digit exponent plus its sign.

# Reverse Polish Notation

In the 1920's, Jan Lukasiewicz developed a formal logic system which allowed mathematical expressions to be specified without parentheses by placing the operators before (prefix notation) or after (postfix notation) the operands. For example, the (infix notation) expression:

```
(4 + 5) × 6
```

could be expressed in postfix notation as:

```
4 5 + 6 ×    or    6 4 5 + ×
```

HP adjusted the postfix notation for a calculator keyboard, added a stack to hold the operands and functions to reorder the stack. HP dubbed the result Reverse Polish Notation (RPN) in honor of Lukasiewicz.

In the years that followed, computer scientists realized that RPN or postfix notation was very efficient for computer math. As a postfix expression is scanned from left to right, operands are simply placed into a last-in, first-out (LIFO) stack and operators may be immediately applied to the operands at the bottom of the stack. By contrast, expressions with parentheses and precedence (infix notation) require that operators be delayed until some later point.

In comparison, testing of Reverse Polish Notation with algebraic notation, RPN has been found to lead to faster calculations, for two reasons. The first reason is that RPN calculators do not need expressions to be parenthesized, so fewer operations need to be entered to perform typical calculations. Additionally, users of RPN calculators made fewer mistakes than for other types of calculators. Later research clarified that the increased speed from Reverse Polish Notation may be attributed to the smaller number of keystrokes needed to enter this notation, rather than to a smaller cognitive load on its users.

At the time that the HP-35 was introduced, other pocket calculators typically used a partial algebraic model. That meant they could evaluate trivial expressions like 4+5 but couldn't handle anything that involved parentheses or algebraic precedence. RPN allowed HP to produce a pocket calculator that could evaluate arbitrary expressions using the available technology. During the 1970s and 1980s, HP used RPN in all of their desktop and hand-held calculators, and continued to use it in some models into the 2020s.

### Four level RPN introduced on the HP-35 has the following features: ###

- Stack has four levels (X, Y, Z, T)
- Only X displayed
- Numbers in T copied down on pop
- All operations left result in X register
- Store and Recall commands provided for X register only
- Store X leaves the X register untouched
- All Unary functions worked on X register
- Implied stack lift on new entry (e.g., 5 log 10 resulted in 5 log in Y and 10 in X)


# HP-35 Emulator

Tiny RPN Calculator | HP-35 Calculator | Operation
------------------- | ---------------- | ---------
`0` `1` `2` `3` `4` `5` `6` `7` `8` `9` | `0` `1` `2` `3` `4` `5` `6` `7` `8` `9` | Enter number digits
`.` | `.` | Enter decimal point
`↑` | `ENTER↑` | Push number to stack
`F` || Function shift key
`E` | `EEX` | Enter exponent
`±` | `CHS` | Change sign
`C` | `CLX` | Clear X register
`F` `AC`  | `CLR` | Clear everything
`F` `X↔Y` | `X↔Y` | Exchange X and Y numbers
`F` `÷` | `÷` | Divide Y by X
`F` `×` | `×` | Multiply Y by X
`F` `−` | `−` | Substrac X from Y
`F` `+` | `+` | Add Y with X
`F` `STO`   | `STO` | Store number in memory
`F` `RCL`   | `RCL` | Recall number from memory
`F` `MEM+`  || **Extension:** Add X to stored number
`F` `CONST` | `𝛑` | Enter 𝛑 constant
`F` `ROT↑`  || **Extension:** Roll stack up
`F` `ROT↓`  | `R↓` | Roll stack down
`F` `MATH`  || Enter Math menu
`F` `TRIG`  || Enter Trigonometric menu
`F` `PROG`  || Enter Programming menu
`F` `MATH` `xʸ` | `xʸ` | Power of positive number
`F` `MATH` `√x` | `√x` | Square root of number
`F` `MATH` `1/x` | `1/x` | Reciprocal of number
`F` `MATH` `LOG` | `log` | Common logarithm of number
`F` `MATH` `LN` | `ln` | Natural logarithm of number
`F` `MATH` `eˣ` | `eˣ` | Natural antilogarithm
`F` `TRIG` `SIN`  | `sin` | Sine of angle in degrees
`F` `TRIG` `COS`  | `cos` | Cosine of angle in degrees
`F` `TRIG` `TAN`  | `tan` | Tangent of angle in degrees
`F` `TRIG` `ASIN` | `arc` `sin` | Arcsine of number
`F` `TRIG` `ACOS` | `arc` `cos` | Arccosine of number
`F` `TRIG` `ATAN` | `arc` `tan` | Arctangent of number
`F` `PROG` `TIME` || Set time to Y hours and X minutes
`F` `PROG` `DATE` || Set date to Y day of the month and X month
`F` `PROG` `YEAR` || Set year to X value
`F` `LAST`  || **Extension:** Repeat last operation from menu
