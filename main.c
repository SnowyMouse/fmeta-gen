// SPDX-License-Identifier: GPL-3.0-only

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct fmeta_file {
    uint64_t two;
    char s3dpak[0x100];
    char pad1[0x8];
    uint64_t ff;
    char map[0x100];
    char pad2[0x4];
    uint32_t one;
    uint64_t size;
    char pad3[0x41E0];
};

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("%s <compressed-map> <fmeta>\n", *argv);
        return 1;
    }

    // Initialize a file
    struct fmeta_file file;
    memset(&file, 0, sizeof(file));
    file.two = 2;
    file.one = 1;
    file.ff = ~((uint64_t)0);
    char *o = argv[1];
    char *end = argv[1];
    for(char *i = argv[1]; *i; i++) {
        if(*i == '\\' || *i == '/') {
            o = i + 1;
        }
    }
    for(char *i = o; *i; i++) {
        if(*i == '.') {
            end = i;
        }
    }
    size_t size = end - o;
    char name[sizeof(file.map)];
    if(size == 0 || size > sizeof(name)) {
        size = sizeof(name);
    }
    snprintf(name, size, "%s", o);
    snprintf(file.s3dpak, sizeof(file.s3dpak), "%s.s3dpak", name);
    snprintf(file.map, sizeof(file.map), "%s.map", name);

    FILE *f = fopen(argv[1], "rb");
    if(!f) {
        fprintf(stderr, "(X)> Failed to open %s\n", argv[1]);
        return 1;
    }

    // Get the count and offsets
    uint32_t count;
    if(fread(&count, sizeof(count), 1, f) == 0) {
        fclose(f);
        fprintf(stderr, "(X)> Invalid file %s (failed to read count)\n", argv[1]);
        return 1;
    }

    uint32_t *offsets = malloc(count * sizeof(uint32_t));
    if(fread(offsets, sizeof(*offsets), count, f) == 0) {
        fclose(f);
        fprintf(stderr, "(X)> Invalid file %s (failed to read offsets)\n", argv[1]);
        return 1;
    }
    rewind(f);

    // Get the total size
    size_t current_offset = 0;
    for(uint32_t i = 0; i < count; i++) {
        if(current_offset > offsets[i]) {
            fclose(f);
            fprintf(stderr, "(X)> Invalid file %s (unexpected size difference)\n", argv[1]);
            return 1;
        }

        // Seek to it
        size_t diff = offsets[i] - current_offset;
        fseek(f, diff, SEEK_CUR);
        current_offset += diff;

        // Add it to our size
        uint32_t size_to_add;
        if(fread(&size_to_add, sizeof(size_to_add), 1, f) == 0) {
            fclose(f);
            fprintf(stderr, "(X)> Invalid file %s (failed to read size)\n", argv[1]);
            return 1;
        }
        file.size += size_to_add;
        current_offset += sizeof(size_to_add);
    }

    free(offsets);
    fclose(f);

    printf("(^)> Got the size: %zu bytes\n", (size_t)file.size);

    FILE *fmeta = fopen(argv[2], "wb");
    if(!fmeta) {
        fprintf(stderr, "(X)> Failed to open %s\n", argv[2]);
        return 1;
    }

    if(fwrite(&file, sizeof(file), 1, fmeta) == 0) {
        fprintf(stderr, "(X)> Failed to write to %s\n", argv[2]);
        return 1;
    }
    fclose(fmeta);

    printf("(^)< Done!\n");
}
