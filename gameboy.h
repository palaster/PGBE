#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "bit_logic.h"
#include "cpu.h"

#define WIDTH 160
#define HEIGHT 144

#define CYCLES_PER_SECOND 4194304
#define FRAMES_PER_SECOND 59.727500569606
#define CYCLES_PER_FRAME CYCLES_PER_SECOND / FRAMES_PER_SECOND
//#define TIME_BETWEEN_FRAMES_IN_NANOSECONDS 16666666.66 BASED ON 60 FPS
#define TIME_BETWEEN_FRAMES_IN_NANOSECONDS 16742706.2988

#define TIMA 0xff05
#define TMA 0xff06
#define TMC 0xff07

typedef enum Color {
    WHITE,
    LIGHT_GRAY,
    DARK_GRAY,
    BLACK
} Color;

typedef struct GameBoy {
    int scanlineCounter;
    int timerCounter;
    int dividerCounter;
    bool romBanking;
    bool enableRAM;
    bool mBC1;
    bool mBC2;
    bool haltBug;
    CPU cpu;
    uint8_t gamepadState;
    uint8_t currentROMBank;
    uint8_t currentRAMBank;
    uint8_t ramBanks[0x8000];
    uint8_t cartridge[0x200000];
    uint8_t rom[0x10000];
    uint8_t screenData[WIDTH][HEIGHT][3];
} GameBoy;

bool gameboyDebug();

void doDMATransfer(GameBoy* gameBoy, const uint8_t value);

void doRAMBankEnable(GameBoy* gameBoy, const uint16_t address, const uint8_t value);
void doChangeLoROMBank(GameBoy* gameBoy, const uint8_t value);
void doChangeHiROMBank(GameBoy* gameBoy, const uint8_t value);
void doRAMBankChange(GameBoy* gameBoy, const uint8_t value);
void doChangeROMRAMMode(GameBoy* gameBoy, const uint8_t value);
void handleBanking(GameBoy* gameBoy, const uint16_t address, const uint8_t value);

uint8_t readFromMemory(GameBoy* gameBoy, const uint16_t address);
void writeToMemory(GameBoy* gameBoy, const uint16_t address, const uint8_t value);

void updateTimer(GameBoy* gameBoy, const int cycles);
bool isClockEnabled(GameBoy* gameBoy);
uint8_t getClockFreq(GameBoy* gameBoy);
void setClockFreq(GameBoy* gameBoy);
void doDividerRegister(GameBoy* gameBoy, const int cycles);

void requestInterrupt(GameBoy* gameBoy, const int interrupt_id);
int doInterrupts(GameBoy* gameBoy);
void serviceInterrupt(GameBoy* gameBoy, const int interrupt_id);

bool isLCDEnabled(GameBoy* gameBoy);
void setLCDStatus(GameBoy* gameBoy);

void updateGraphics(GameBoy* gameboy, const int cycles);
void drawScanline(GameBoy* gameBoy);
void renderSprites(GameBoy* gameBoy);
void renderTiles(GameBoy* gameBoy);
Color getColor(GameBoy* gameBoy, const uint16_t address, const uint8_t colorNum);

uint8_t getGamepadState(GameBoy* gameBoy);
void keyPressed(GameBoy* gameBoy, const int key);
void keyReleased(GameBoy* gameBoy, const int key);