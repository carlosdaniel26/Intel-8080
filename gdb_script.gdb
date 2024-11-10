# break emulate_instruction
break cpu.c:1004

run

### Display cpu state


# Ignore
skip video_buffer_to_screen
skip update_screen
skip get_rom
skip get_rom_size
skip printf
