break intel8080_main
run
break emulate_instruction if cpu->registers.pc == 0x066C