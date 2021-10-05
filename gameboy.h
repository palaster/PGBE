#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define WIDTH 160
#define HEIGHT 144

#define CYCLES_PER_SECOND 4194304
#define FRAMES_PER_SECOND 59.727500569606
#define CYCLES_PER_FRAME CYCLES_PER_SECOND / FRAMES_PER_SECOND
#define TIME_BETWEEN_FRAMES 16666666.66

#define TIMA 0xff05
#define TMA 0xff06
#define TMC 0xff07

/*
// FIRST METHOD byte, bitPosition = byte & (1 << bitPosition)
// SECOND METHOD byte, bitPosition = (byte >> bitposition) & 1
*/

//static bool check_bit(uint8_t byte, uint8_t position) { return byte & (1 << position) != 0; } // aka GetVal
static bool check_bit(uint8_t byte, uint8_t position) { return (byte & (1 << position)) != 0; } // aka GetVal
static uint8_t bit_value(uint8_t byte, uint8_t position) { return (byte >> position) & 1; } // aka TestBit ** KIND OF EQUIVALENT TO check_bit
static uint8_t set_bit(uint8_t byte, uint8_t position) { return byte | (1 << position); }
static uint8_t reset_bit(uint8_t byte, uint8_t position) { return byte & (~(1 << position)); }
static uint8_t set_bit_to(uint8_t byte, uint8_t position, bool on) { return on ? set_bit(byte, position) : reset_bit(byte, position); }

typedef enum Color {
    WHITE,
    LIGHT_GRAY,
    DARK_GRAY,
    BLACK
} Color;

typedef struct CPU {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    bool zero, subtract, halfCarry, carry;
    bool halted;
    bool interrupts;
    bool pendingInterruptEnable;
    bool pendingInterruptDisable;
} CPU;

typedef struct GameBoy {
    int scanlineCounter;
    int timerCounter;
    int dividerCounter;
    bool romBanking;
    bool enableRAM;
    bool mBC1;
    bool mBC2;
    CPU cpu;
    uint8_t currentROMBank;
    uint8_t currentRAMBank;
    uint8_t ramBanks[0x8000];
    uint8_t cartridge[0x200000];
    uint8_t rom[0x10000];
    uint8_t screenData[WIDTH][HEIGHT][3];
} GameBoy;

// START OPCODE FUNCTION

void jp_from_word(GameBoy* gameBoy, uint16_t address);
void jp_from_bytes(GameBoy* gameBoy, uint8_t lower, uint8_t upper);
void jp_from_pc(GameBoy* gameBoy);

// END OPCODE FUNCTION

void doDMATransfer(GameBoy* gameBoy, uint8_t value);

void doRAMBankEnable(GameBoy* gameBoy, uint16_t address, uint8_t value);
void doChangeLoROMBank(GameBoy* gameBoy, uint8_t value);
void doChangeHiROMBank(GameBoy* gameBoy, uint8_t value);
void doRAMBankChange(GameBoy* gameBoy, uint8_t value);
void doChangeROMRAMMode(GameBoy* gameBoy, uint8_t value);
void handleBanking(GameBoy* gameBoy, uint16_t, uint8_t value);

uint8_t readFromMemory(GameBoy* gameBoy, uint16_t address);
void writeToMemory(GameBoy* gameBoy, uint16_t address, uint8_t value);

void setFFlagsFromByte(CPU* cpu, uint8_t newF);
uint8_t getFFlagsAsByte(CPU* cpu);

int update(GameBoy* gameBoy);

void updateTimer(GameBoy* gameBoy, int cycles);
bool isClockEnabled(GameBoy* gameBoy);
uint8_t getClockFreq(GameBoy* gameBoy);
void setClockFreq(GameBoy* gameBoy);
void doDividerRegister(GameBoy* gameBoy, int cycles);

void requestInterrupt(GameBoy* gameBoy, int id);
void serviceInterrupt(GameBoy* gameBoy, int interrupt);
int doInterrupts(GameBoy* gameBoy);

bool isLCDEnabled(GameBoy* gameBoy);
void setLCDStatus(GameBoy* gameBoy);

void updateGraphics(GameBoy* gameboy, int cycles);
void drawScanline(GameBoy* gameBoy);
void renderSprites(GameBoy* gameBoy);
void renderTiles(GameBoy* gameBoy);
Color getColor(GameBoy* gameBoy, uint16_t address, uint8_t colorNum);

void printCPU(GameBoy* gameBoy);