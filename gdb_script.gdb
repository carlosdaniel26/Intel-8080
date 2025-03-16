# Set a breakpoint at the start of the program to ensure symbols are loaded
break main
run
break emulate_instruction
continue
display /x cpu->registers.pc

define print_sp
    printf "SP: 0x%04X\n", cpu->registers.sp
end

define print_pc
    printf "PC: 0x%04X -> Instruction: 0x%02X\n", cpu->registers.pc, cpu->rom[cpu->registers.pc]
end

define print_flags
    printf "Flags (F): CY: %d, P: %d, AC: %d, Z: %d, S: %d\n", cpu->registers.F.cy, cpu->registers.F.p, cpu->registers.F.ac, cpu->registers.F.z, cpu->registers.F.s
end

define print_reg
    printf "A: 0x%02X, B: 0x%02X, C: 0x%02X, D: 0x%02X, E: 0x%02X, H: 0x%02X, L: 0x%02X\n", cpu->registers.A, cpu->registers.B, cpu->registers.C, cpu->registers.D, cpu->registers.E, cpu->registers.H, cpu->registers.L
end


# Ignore
skip video_buffer_to_screen
skip update_screen

break