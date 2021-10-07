#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct GameBoy GameBoy;

typedef struct CPU {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    bool zero, subtract, halfCarry, carry;
    bool halted;
    bool interruptsEnabled;
    bool pendingInterruptEnable;
} CPU;

bool cpuDebug();

uint8_t fetch(GameBoy* gameBoy);

void pop(GameBoy* gameBoy, uint8_t* lower, uint8_t* upper);
void push(GameBoy* gameBoy, const uint8_t lower, const uint8_t upper);

void jp_from_word(GameBoy* gameBoy, const uint16_t address);
void jp_from_bytes(GameBoy* gameBoy, const uint8_t lower, const uint8_t upper);
void jp_from_pc(GameBoy* gameBoy);

int decodeAndExecute(GameBoy* gameBoy, const uint8_t instruction);
int decodeAndExecuteCB(GameBoy* gameBoy, const uint8_t instruction);

int updateCPU(GameBoy* gameBoy);

void setFFlagsFromByte(CPU* cpu, const uint8_t newF);
uint8_t getFFlagsAsByte(CPU* cpu);

void printCPU(CPU* cpu);