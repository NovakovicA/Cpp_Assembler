.extern x1
.global s2,s5,s8
.global e6

.equ e6, +s2-s1+s5-s4+s8-s7+s1
.equ e5, s1-s2+s3
.equ e4, e3+0x100+x1
.equ e3, e2+0x100
.equ e2, e1+0x100
.equ e1, 0x100



.data
s1:.byte 0
s2:.byte 0xFF
s3:.byte s1

.section test1
s4:.byte 0
s5:.byte 0xFF
s6:.byte s4

.section test2
s7:.byte 0
s8:.byte 0xFF
s9:.byte s7

.text
add e1,%r0
add e6,%r0




.end