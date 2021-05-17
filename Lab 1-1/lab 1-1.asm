.globl _start

.data
	var1: .word 15
	var2: .word 19
	print_ram_address: .asciz "Print RAM addresses of var1 and var2, respectively:\n"
	space: .asciz ", "
	next_line: .asciz "\n"
	prompt: .asciz "\nIncrease var1 by 1 and multiply var 2 by 4, values of var1 and var2 respectively are:\n"
	swapping: .asciz "\nAfter swapping var1 and var2, the value of var1 and var2, respectively are:\n"

.text
_start:
li a7, 4
la a0, print_ram_address
ecall


la a1, var1
#lw a0, 0(a1)
mv a0, a1
li a7, 1
ecall 

li a7, 4
la a0, next_line
ecall

la a2, var2
#lw a0, 0(a2)
mv a0, a2
li a7, 1
ecall

li a7, 4
la a0, next_line
ecall

li a7, 4
la a0, prompt
ecall


# print var1 + 1
lw a3, 0(a1)
addi a3, a3, 1
mv a0, a3
li a7, 1
ecall 

li a7, 4
la a0, next_line
ecall

# print var2 * 4
lw a4, 0(a2)
slli a4, a4, 2
mv a0, a4
li a7, 1
ecall 

li a7, 4
la a0, next_line
ecall

# swap var1 znd var 2

addi a5, a3, 0 # a5 is temp = address of a3
sw a4, 0(a1)
sw a5, 0(a2)

li a7, 4
la a0, swapping
ecall


# print var1 after swap
lw a3, 0(a1)
mv a0, a3
li a7, 1
ecall 

li a7, 4
la a0, next_line
ecall

# print var2 after swap
lw a4, 0(a2)
mv a0, a4
li a7, 1
ecall 

li a7, 4
la a0, next_line
ecall

#end program
li a7, 10
ecall
