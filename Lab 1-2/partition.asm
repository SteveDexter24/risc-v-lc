.globl _start

.data
	array1: .word -1 22 8 35 5 4 11 2 1 78
	label: .asciz "The array after partition with '8' as pivot :\n"
	space: .asciz " "

.text
_start:
# this is somewhat like in int main() {...}
la a1, array1 # address of array
addi a4, a4, 0	# a4 is set to 0
addi a5, a5, 10 # a5 is the number of elements in the array
jal partition	# go to partition
jal print 	# go to print

# end the program 
li a7, 10
ecall

# swap function
swap: 	slli s8, t4, 2
	add  s8, a1, s8
	lw  s4, 0(s8)
	lw  s5, 0(s6)
	sw  s5, 0(s8)
	sw  s4, 0(s6)
	jr ra

partition:

	addi sp, sp, -4 	# adjust stack pointer
	sw ra, 0(sp)

	lw t1, 8(a1) 		#load the pivot which is 8

	addi  a2, a1, 8
	addi  a3, a1, 36

	# set pivot as the last element in array
	lw s4, 0(a2)
	lw s5, 0(a3)
	sw s5, 0(a2)
	sw s4, 0(a3)


	addi t3, t3, 9  	# index of the last element in the array
	addi t4, t4, -1		# i = low - 1
	addi t5, t5, 0  	# j = 0

	# for(j = low; j < 9; j++)
	for_loop:
		bge t5, t3, end_for_loop	# if j >= 9 skip the loop

		slli s6, t5, 2  		# s6 = j * 4
		add s6, a1, s6  		# s6 = address of array + j * 4
		lw  s10, 0(s6)  		# load a[j] to s10

		bge s10, t1, end_if 		# when a[j] >= pivot, jump to end_if
		addi t4, t4, 1 			# i++
		
		jal ra, swap 			# call swap
		
		addi t5, t5, 1 			# j++
		j for_loop    			# jump to for_loop
	end_if:
		addi t5, t5, 1  		# j++
		j for_loop

	end_for_loop:
		addi t4, t4, 1 			# i++
		addi s6, a1, 36 		# get high

		jal ra swap

		lw ra, 0(sp)			# store the return address
		addi sp, sp, 4 			# adjust stack pointer
		jr ra

print:
	la a0, label
	li a7, 4
	ecall

	# for(int i = 0; i < 10; i++)
	addi a4, zero, 0
	addi a5, zero, 10

	looping:
		bge a4, a5, end_print
		slli a6, a4, 2
		add a6, a6, a1

		lw a0, 0(a6)
		li a7, 1
		ecall
		la a0, space
		li a7, 4
		ecall
		addi a4, a4, 1
		j looping

	end_print:
		jr ra
