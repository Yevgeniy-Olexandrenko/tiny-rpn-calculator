# Introduction

The HP-35 was HP's first pocket calculator. It was introduced at a time when most calculators (including expensive desktop models) had only the four basic functions. The HP-35 was the first pocket calculator with transcendental functions and the first with RPN.

[![HP-35](https://www.hpmuseum.org/3qs/35v33q.jpg)](https://www.hpmuseum.org/hp35.htm)

The calculator had a four register stack (**X**, **Y**, **Z** and **T**), the `ENTER↑` key pushed the displayed value (**X**) down the stack. Any binary operation popped the top two registers, and pushed the result. When the stack was popped the **T** register duplicated into the **Z** register.

Internally, the calculator was organized around a serial (one-bit) processor chipset made under contract by Mostek, processing 56-bit floating-point numbers, representing 14-digit BCD numbers.

The calculator used a traditional floating decimal display for numbers that could be displayed in that format, but automatically switched to scientific notation for other numbers. The fifteen-digit LED display was capable of displaying a ten-digit mantissa plus its sign and a decimal point and a two-digit exponent plus its sign.

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
