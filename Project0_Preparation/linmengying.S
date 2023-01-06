.global main
main:
// TODO: [p0-task1] Add your code here!
	xor x1,x1,x1
    addi x2,x0,0x1
	addi x3,x0,101

Loop:
	add x1,x1,x2
	addi x2,x2,0x01
	bne x3,x2,Loop
	
end:
	j end
