#include <stdint.h>
#include <stdio.h>
#include <json-c/json.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <helper.h>
#include <cpu.h>

uint16_t twoU8_to_u16value(uint8_t msb, uint8_t lsb)
{   
    return ((msb << 8) | lsb);
}

uint16_t twoU8_to_u16adress(uint8_t msb, uint8_t lsb)
{   
    return twoU8_to_u16value(lsb, msb);
}

uint8_t read_byte(Cpu8080 *cpu)
{
    unsigned PC = cpu->registers.pc;
    return (cpu->rom[PC+1]);
}

uint16_t read_byte_address(Cpu8080 *cpu)
{
    unsigned PC = cpu->registers.pc;
    uint16_t answer = twoU8_to_u16adress(cpu->rom[PC+1], cpu->rom[PC+2]);

    return answer;
}

Flags byteToFlags(uint8_t byte) 
{
    Flags flags;
    
    flags.cy = READBIT(byte, 0);
    flags.p = READBIT(byte, 2);
    flags.ac = READBIT(byte, 4);
    flags.z = READBIT(byte, 6);
    flags.s = READBIT(byte, 7);

    return flags;
}

uint8_t flagsToByte(Flags flags) 
{
    uint8_t byte = 0;
    
    if (flags.cy) set_bit(byte, 0);
    if (flags.p) set_bit(byte, 2);
    if (flags.ac) set_bit(byte, 4);
    if (flags.z) set_bit(byte, 6);
    if (flags.s) set_bit(byte, 7);
    
    return byte;
}

void cpu_to_json(const Cpu8080 *cpu)
{
    struct json_object *cpu_json = json_object_new_object();
    struct json_object *registers_json = json_object_new_object();
    struct json_object *flags_json = json_object_new_object();
    struct json_object *memory_json = json_object_new_array();

    // Registers
    json_object_object_add(registers_json, "A", json_object_new_int(cpu->registers.A));
    json_object_object_add(registers_json, "B", json_object_new_int(cpu->registers.B));
    json_object_object_add(registers_json, "C", json_object_new_int(cpu->registers.C));
    json_object_object_add(registers_json, "D", json_object_new_int(cpu->registers.D));
    json_object_object_add(registers_json, "E", json_object_new_int(cpu->registers.E));
    json_object_object_add(registers_json, "H", json_object_new_int(cpu->registers.H));
    json_object_object_add(registers_json, "L", json_object_new_int(cpu->registers.L));
    json_object_object_add(registers_json, "SP", json_object_new_int(cpu->registers.sp));
    json_object_object_add(registers_json, "PC", json_object_new_int(cpu->registers.pc));

    // Flags
    json_object_object_add(flags_json, "Z", json_object_new_boolean(cpu->registers.F.z));
    json_object_object_add(flags_json, "S", json_object_new_boolean(cpu->registers.F.s));
    json_object_object_add(flags_json, "P", json_object_new_boolean(cpu->registers.F.p));
    json_object_object_add(flags_json, "CY", json_object_new_boolean(cpu->registers.F.cy));
    json_object_object_add(flags_json, "AC", json_object_new_boolean(cpu->registers.F.ac));

    for (unsigned i = 0; i < TOTAL_MEMORY_SIZE; i++) 
    {
        json_object_array_add(memory_json, json_object_new_int(cpu->memory[i]));
    }

    // Put the objects together
    json_object_object_add(registers_json, "Flags", flags_json);
    json_object_object_add(cpu_json, "Registers", registers_json);
    json_object_object_add(cpu_json, "InterruptEnabled", json_object_new_boolean(cpu->interrupt_enabled));
    json_object_object_add(cpu_json, "Memory", memory_json);

    printf("state transformed in json\n");
    save_state_to_json_file(cpu_json);
}

uint64_t counter = 0;

void save_state_to_json_file(struct json_object *cpu_json)
{
    char dir_name[256];
    char file_name[256];

    sprintf(dir_name, "logs/%lu", counter);
    sprintf(file_name, "%s/cpu.json", dir_name);

    rmdir(dir_name);

    if (mkdir("logs", 0755) && errno != EEXIST) {
        perror("Error creating 'logs' directory");
        return;
    }
    
    if (mkdir(dir_name, 0755) && errno != EEXIST) {
        perror("Error creating counter-specific directory");
        return;
    }

    FILE *file = fopen(file_name, "w");

    if (file == NULL) {
        perror("Error while open file to write");
    }

    fprintf(file, "%s\n", json_object_to_json_string_ext(cpu_json, JSON_C_TO_STRING_PRETTY));

    fclose(file);

    counter++;
}