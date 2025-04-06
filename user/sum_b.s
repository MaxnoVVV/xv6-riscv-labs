.globl main

.section .rodata
    delimiter: .word 32
.section .data
    buf: .zero 512
    len: .word 512
    error_message_1: .asciz "Usage: sum_a <int64 number> <int64 number>"
    error_message_2: .asciz "Invalid number"
    string: .asciz "%s\n"
    print_message: .asciz "The sum is %d\n"
    input_format: .asciz "|%s|\n"

.text

main:
    la a0, buf
    lw a1, len
    call gets

    la a0, buf
    call replace_eol
    la a0, input_format
    la a1, buf
    call printf
    la a0, buf
    lw a1, len
    call find_delimiter #a0 - pointer to delimiter
    sb x0, 0(a0) #put \0 to delimiter

    addi sp, sp, -4
    sw a0, 0(sp) # sp -> delimiter pointer

    la a0, buf
    call atoi
    la a1, buf
    call check_int
    mv t0, a0
    lw a0, 0(sp)
    sw t0, 0(sp) # a0 -> delimiter pointer, sp -> 1st number

    addi sp, sp, -4
    addi a0, a0, 1
    sw a0, 0(sp) #sp -> delimiter+1 pointer


    call atoi
    lw a1, 0(sp)
    call check_int
    addi sp, sp, 4
    lw a1, 0(sp)

    li a7, 22
    ecall
    mv a1, a0
    la a0, print_message
    call printf
    li a0, 0
    addi sp, sp, 4
    call exit
    ret


check_int: #a0 - number, a1 - pointer
    mv t0, a1
    beq x0, a0, result_zero
    ret

result_zero:
    lb t1, 0(t0)
    li t2, 48 #0

    bne t1, t2, invalid_number
    addi t0, t0, 1
    lb t1, 0(t0)
    li t2, 0
    li t3, 10
    beq t1, t2, ok
    beq t1, t3, ok
    j invalid_number
ok:
    ret


find_delimiter: #a0 - pointer to string
    mv t0, a0
    li t1, 0
    li t3, 32
    li t4, 0 #counter
    li t5, 1 #max delimiters
    mv t6, t0 #t6 - pointer to delim
loop:
    lb t2, 0(t0)
    beq t2, t1, end_loop
    bne t2, t3, not_delim
    addi t4, t4, 1
    mv t6, t0
not_delim:
    addi t0, t0, 1
    j loop
end_loop:
    bne t5, t4, print_usage
    mv a0, t6
    ret


print_usage:
    la a0, string
    la a1, error_message_1
    call printf
    li a0, -1
    call exit
    ret

invalid_number:
    la a0, string
    la a1, error_message_2
    call printf
    li a0, -1
    call exit
    ret

replace_eol: #a0 - pointer to buf
    li t0, 0
    li t4, 10
    lw t1, len
replace_eol_loop:
    add t2, a0, t0
    lw t3, 0(t2)
    beq t3, t4, replace_eol_ret
    addi t0, t0, 1
    blt t0, t1, replace_eol_loop
    ret
replace_eol_ret:
    li t0, 0
    sw t0, 0(t2)
    ret

