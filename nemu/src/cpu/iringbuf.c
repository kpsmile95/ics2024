#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RING_BUFFER_SIZE 8

typedef struct {
    uint32_t pc;
    uint8_t inst[16]; // 假设指令最长为16字节
    int ilen;
} TraceEntry;

static TraceEntry ring_buffer[RING_BUFFER_SIZE];
static int ring_buffer_index = 0;
const char *filename = "trace.txt";

void init_ring_buffer() {
    memset(ring_buffer, 0, sizeof(ring_buffer));
    ring_buffer_index = 0;
}

void log_instruction(uint64_t pc, uint8_t *inst, int ilen) {
    TraceEntry *entry = &ring_buffer[ring_buffer_index];
    entry->pc = pc;
    entry->ilen = ilen;
    memcpy(entry->inst, inst, ilen);
    ring_buffer_index = (ring_buffer_index + 1) % RING_BUFFER_SIZE;
}

void print_ring_buffer() {
    printf("Recent instructions:\n");
    for (int i = 0; i < RING_BUFFER_SIZE; i++) {
        int index = (ring_buffer_index + i) % RING_BUFFER_SIZE;
        TraceEntry *entry = &ring_buffer[index];
        printf("0x%08x: ", entry->pc);
        for (int j = 0; j < entry->ilen; j++) {
            printf("%02x ", entry->inst[j]);
        }
        printf("\n");
    }
}

void write_ring_buffer_to_file() {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    fprintf(file, "Recent instructions:\n");
    for (int i = 0; i < RING_BUFFER_SIZE; i++) {
        int index = (ring_buffer_index + i) % RING_BUFFER_SIZE;
        TraceEntry *entry = &ring_buffer[index];
        fprintf(file, "0x%08x: ", entry->pc);
        for (int j = 0; j < entry->ilen; j++) {
            fprintf(file, "%02x ", entry->inst[j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}