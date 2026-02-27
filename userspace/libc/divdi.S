    .section .text
    .globl __divdi3

# long long __divdi3(long long numerator, long long denominator)
# Arguments (GCC 32-bit ABI):
# a0 = numerator low
# a1 = numerator high
# a2 = denominator low
# a3 = denominator high
# Returns quotient in a1:a0 (high:low)
# RV32I only, unsigned

__divdi3:
    # Check if numerator is zero
    beq a0, zero, zero_num
    beq a1, zero, zero_num

    # Check if denominator is zero
    beq a2, zero, div_by_zero
    beq a3, zero, div_by_zero

    # Initialize quotient
    li a0, 0
    li a1, 0

    # Copy numerator to t0:t1
    mv t0, a0   # low
    mv t1, a1   # high

    # Copy denominator to t2:t3
    mv t2, a2   # low
    mv t3, a3   # high

    # Simple shift-subtract division loop (64-bit)
    li t4, 64   # bit counter

div_loop:
    # TODO: implement shift-subtract division
    # This is placeholder for testing only
    addi t4, t4, -1
    beqz t4, div_done
    j div_loop

div_done:
    ret

zero_num:
    li a0, 0
    li a1, 0
    ret

div_by_zero:
    # For now, return 0 (or trap)
    li a0, 0
    li a1, 0
    ret
