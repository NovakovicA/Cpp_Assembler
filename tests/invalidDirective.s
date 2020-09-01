.extern x1,x2,x3

.data
s1:.byte -1
.byte 0xFF
.word 0xAAAB
s2:.word s1
.skip 10
s3:.byte 0




.invalidDirective


.end