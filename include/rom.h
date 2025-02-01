#ifndef ROM_H
#define ROM_H

#define ROM_SPACE_INVADERS "./rom/invaders.b"
#define ROM_8080PRE "./rom/cpu_tests/8080PRE.COM"

#define ROM_FILE ROM_8080PRE

char* get_rom();
int get_rom_size();

#endif