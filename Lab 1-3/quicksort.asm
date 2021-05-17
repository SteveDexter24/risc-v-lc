.globl _start

.data 
	array: .word -1, 22, 8, 35, 5, 4, 11, 2, 1, 78
	#array: .word -1, -4, -3, -2, 100, 300, 200, -100, 11, 6 # test pass
	space: .asciz " "
	prompt: .asciz"After Sorting the Array:\n"

.text # Defines the start of the code section for the program .
_start:

la a0, array # load address of the array
addi a1, zero, 0 # set low = 0
addi a2, zero, 9 # index array size - 1
jal quicksort # Call quicksort
j print # jump to print results

quicksort:				

	addi sp, sp, -16		# Adjust stack pointer

	sw a0, 0(sp)			# store array
	sw a1, 4(sp)			# store low
	sw a2, 8(sp)			# store high
	sw ra, 12(sp)			# store return address

	mv t0, a2			#set high as t0
	
	# if (low < high)
	bge a1, a2, endif		# if low >= high, endif
			
	jal partition			# call partition
	mv s0, a7			# pivot, s0 = a7

	lw a1, 4(sp)			# a1 = low
	addi a2, s0, -1			# a2 = pi - 1
	jal quicksort			# call quicksort

	addi a1, s0, 1			# a1 = pi + 1
	lw a2, 8(sp)			# a2 = high
	jal quicksort			# call quicksort

 endif:

 	lw a0, 0(sp)			#restore a0
 	lw a1, 4(sp)			#restore a1
 	lw a2, 8(sp)			#restore a2
 	lw ra, 12(sp)			#restore return address
 	addi sp, sp, 16		#restore the stack
 	jr ra				#return to caller


swap:				
	#swap(i, j)
	slli s8, a1, 2 		# offset of i
	add  s8, a0, s8 	#get address of a[i]
	lw  s10, 0(s8) 		# load a[i]
	lw  s11, 0(t1)		# a[j] = s11
	sw  s11, 0(s8)
	sw  s10, 0(t1)
	jr ra			#jump back to where this function was called

 	
# int partition (int array[], int low, int high) 
partition: 			

	addi sp, sp, -16	# adjust stack pointer

	sw a0, 0(sp)		# store array
	sw a1, 4(sp)		# store low
	sw a2, 8(sp)		# store high
	sw ra, 12(sp)		# store the return address

	mv s1, a1		# set s1 to low
	mv s2, a2		# set s2 to high

	slli t1, s2, 2		# get the offset for index of high
	add t1, a0, t1		# t1 = address of array + 4 * high
	lw t2, 0(t1)		# load array[high] and set it as t2, which is the pivot
	
	# for (int j = low; j <= high - 1; j++) 
	addi t3, s1, -1 	#t3, i = low -1
	mv t4, s1		#t4, j = low
	addi t5, s2, -1		#t5 = high - 1

	for_loop:
		bgt t4, t5, end_for	# if low > high - 1	

		slli t1, t4, 2		# get offset of j, j * 4
		add t1, t1, a0		# get *array[j]
		lw s7, 0(t1)		# get array[j] = s7

		# if (arr[j] < pivot) 		 
		bge s7, t2, end_if	# if arr[j] >= pivot go to end_if
		addi t3, t3, 1		# i++

		mv a1, t3		#a1 = i
		mv a2, t4		#a2 = j
		
		jal ra swap

		addi t4, t4, 1		# j++
		j for_loop

	    end_if:
		addi t4, t4, 1		# j++
		j for_loop		# junp back to forloop

	end_for:
		addi a1, t3, 1			# a1 = i + 1
		mv a2, s2			# a2 = high
		add a7, zero, a1		# return (i + 1);
						
		slli t1, a2, 2			# offset of high 
		add t1, a0, t1			# add offset t1 = *array[high]			
		jal ra swap			# swap(&arr[i + 1], &arr[high]); 
		
		# restore stuff:
		lw a0, 0(sp)		# restore array
		lw a1, 4(sp)		# restore low
		lw a2, 8(sp)		# restore high
		lw ra, 12(sp)		# return address
		addi sp, sp, 16		# adjust stack pointer
		jr ra			# return to caller
		
	


# Print the results
print:
 	mv a1, a0
	addi t0, zero, 9
	addi t1, zero, 0
	
	la a0, prompt
	li a7, 4
	ecall

start_print:
	bgt t1, t0, exit_print
	slli s9, t1, 2
	add s9, a1, s9
	lw a0, 0(s9)
	li a7, 1
	ecall

	la a0, space
	li a7, 4
	ecall
	addi t1, t1, 1
	j start_print

exit_print:
	li a7, 10 # end the program
	ecall
	
