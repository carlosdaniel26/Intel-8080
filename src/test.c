#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "rom.h"
#include "main.h"
#include "helper.h"
#include "cpu.h"

#define KRED  "\x1B[31m"
#define KWHT  "\x1B[37m"
#define KGRN  "\x1B[32m"

void test_error(const char *test_name, const unsigned part_id, void *unexpected_value)
{
    int *value = (int *)unexpected_value;
    printf("%sTest error: %s %s in part %u, instead = %X\n", 
           KRED, 
           KWHT, 
           test_name, 
           part_id, 
           value ? *value : 0);
    exit(0);
}

void test_success(const char *test_name, const unsigned part_id)
{
    printf("%sTest passed:%s %s in part %u\n", KGRN, KWHT, test_name, part_id);
}

void test_ADD(Cpu8080* cpu)
{
    cpu->registers.A = 1;
    ADD(cpu, 255);

    uint8_t F = (cpu->registers.F);

    if (cpu->registers.A != 0) 
    {
        test_error("ADD", 0, &cpu->registers.A);
    }
    test_success("ADD", 0);

    if (READBIT(F,  FLAG_CARRY_POS) == 0) 
    {
        test_error("ADD", 1, NULL);
    }
    test_success("ADD", 1);

    if ((cpu->registers.F & FLAG_ZERO_POS)  == 0) 
    {
        test_error("ADD", 2, NULL);
    }
    test_success("ADD", 2);

    if ((cpu->registers.F & FLAG_AUX_CARRY_POS)  == 0) 
    {
        test_error("ADD", 3, NULL);
    }
    test_success("ADD", 3);


    // Test SIGN Flag
    cpu->registers.A = 0;
    ADD(cpu, 255);

    if ((cpu->registers.F & FLAG_SIGN_POS)  == 0) 
    {
        test_error("ADD", 4, NULL);
    }
    test_success("ADD", 4);

    cpu = init_cpu();
}

void test_main()
{
    Cpu8080 *cpu = init_cpu();
    test_ADD(cpu);
}
