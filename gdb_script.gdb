# Set a breakpoint at the start of the program to ensure symbols are loaded
break main
run
break emulate_instruction
continue
display /x cpu->registers.pc

define display_cpu_state
    display /x cpu->rom[cpu->registers.pc]
    display /x cpu->registers.pc

    display /x cpu->registers.F.cy
    display /x cpu->registers.F.p
    display /x cpu->registers.F.ac
    display /x cpu->registers.F.z
    display /x cpu->registers.F.s

    display /x cpu->registers.sp

    display /x cpu->registers.A
    display /x cpu->registers.B
    display /x cpu->registers.C
    display /x cpu->registers.D
    display /x cpu->registers.E
    display /x cpu->registers.H
    display /x cpu->registers.L
end

# Ignore
skip video_buffer_to_screen
skip update_screen

break