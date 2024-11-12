break emulate_instruction if cpu->registers.pc == 0x3a8
#break emulate_instruction if cpu->rom[cpu->registers.pc+1] == 0x47

run

### Display cpu state

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

# Ignore
skip video_buffer_to_screen
skip update_screen

break

