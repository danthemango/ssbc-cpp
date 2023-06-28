; a file to test the assembler
; particularly address labels and offset references

@cbsNews
pushext 1
pushext ; ex3
0x02 ; ex4
0x01 ; ex5
pushext 0x02 ; ex2
0x01
pushext 0x0200 ; ex1
0x02
-1
popext 0x0200
jnz 0x0007 ; comments1
#mytag1 jnz 0x0007 ; comments2
jnz @cbsNews ; comments3
popext 0xFFFE
popext 0x0201
popext 0x0200
halt
noop
pushext 0x0201
add
sub
popinh

255
pushext
#firstPart
0x00
#secondAddress
; before
0x02 ; ex4
// my program:
@cbsNews.H
@cbsNews+3.H
@cbsNews+0xF.L
@cbsNews-0xF.L
; saying hello
pushimm 1
pushimm 0xF
pushimm 2 ; my 8
pushimm 6 // my 7
// this is my script
0
0xAA
0xBEEF

pushimm @cbsNews.H
pushimm @cbsNews.L

pushimm
0       ; Sigma 5 Example
pushimm 0xFF
; courceh
#mytag3 0x00 ; comments4
#mytag4 0x07 ; comments5
0x0007 #mytag5 ; comments6
pushimm 0x5
pushimm 0x1
pushext 0x0201
pushext 0x0200
0x0
0
; should have the value 000F (15)
@cbsNews+2

// hello single-line comment

-128
0x80
127
0x7F
0x1
0X7F
0XFF
0x80
0xFF
0x7FFF
0xFFFF
0x8000
0xf
127
+127
-127
-15
255
0xF
0xD0A
0xAA55
#cbsNews
0xDEAD
0xBEEF