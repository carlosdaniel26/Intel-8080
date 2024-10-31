#include <stdio.h>
#include <stdlib.h>

#include "rom.h"
#include "debug.h"

// ROM functions
char* get_rom() 
{
    FILE *fp = fopen(ROM_FILE, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(fp);
        return NULL;
    }

    long bufsize = ftell(fp);
    if (bufsize == -1) {
        perror("Error getting file size");
        fclose(fp);
        return NULL;
    }

    char *source = malloc(sizeof(char) * (bufsize + 1));
    if (source == NULL) {
        perror("Memory allocation error");
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        free(source);
        fclose(fp);
        return NULL;
    }

    size_t newLen = fread(source, sizeof(char), bufsize, fp);
    if (ferror(fp) != 0) {
        fputs("Error reading file", stderr);
        free(source);
        fclose(fp);
        return NULL;
    } else {
        source[newLen] = '\0';
    }

    fclose(fp);

    return source;
}


int get_rom_size() 
{
    FILE *fp = fopen(ROM_FILE, "rb"); // Open in binary mode
    if (fp == NULL) {
        perror("Error opening file");
        return 0;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(fp);
        return 0;
    }

    int bufsize = ftell(fp);
    if (bufsize == -1) {
        perror("Error getting file size");
    }

    fclose(fp);
    return bufsize;
}
