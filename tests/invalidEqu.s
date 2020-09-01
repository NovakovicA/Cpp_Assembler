.extern x1
.global s2,s5,s8


.equ e3, x1-x1+0x100
.equ e2, s1+s2-s7
.equ e1, -s1+0x100

.equ e4, e5+200
.equ e5, e4+100


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
s10:




.end
