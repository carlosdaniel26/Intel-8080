break emulate_instruction

run

### Display cpu state

display /x cpu->rom[cpu->registers.pc]
display /x cpu->registers.pc

display /x cpu->registers.sp

display /x cpu->registers.A
display /x cpu->registers.B
display /x cpu->registers.C
display /x cpu->registers.D
display /x cpu->registers.E
display /x cpu->registers.H
display /x cpu->registers.L

break cpu.c:2097

# Ignore
skip video_buffer_to_screen
skip update_screen

continue 11