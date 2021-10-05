#include "gameboy.h"
#include <time.h>

#include <SDL2/SDL.h>

#define VERTICAL_BLANK_SCAN_LINE 144
#define VERTICAL_BLANK_SCAN_LINE_MAX 153
#define SCANLINE_COUNTER_START 456

// Make 16 bit from two 8 bits = ((uint16_t) upper << 8) | lower
// Make two 8 bits from 16 bit = upper = (uint8_t) (word >> 8)
//                               lower = (uint8_t) word

uint16_t compose_bytes(uint8_t lower, uint8_t upper) {
    uint16_t upper16 = (uint16_t) upper;
    upper16 <<= 8;
    return (upper16 | lower);
}

void rlc(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = check_bit(*reg, 7);
    uint8_t truncated = check_bit(*reg, 7);
    uint8_t result = ((*reg << 1) | truncated);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = carry;
}

void rrc(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = check_bit(*reg, 0);
    uint8_t truncated = check_bit(*reg, 0);
    uint8_t result = ((*reg >> 1) | (truncated << 7));
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = carry;
}

void rl(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = gameBoy->cpu.carry;
    bool willCarry = check_bit(*reg, 7);
    uint8_t result = (*reg << 1);
    result |= carry;
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = willCarry;
}

void rr(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = gameBoy->cpu.carry;
    bool willCarry = check_bit(*reg, 0);
    uint8_t result = (*reg >> 1);
    result |= (carry << 7);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = willCarry;
}

void sla(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = check_bit(*reg, 7);
    uint8_t result = (*reg << 1);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = carry;
}

void sra(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t carry = check_bit(*reg, 0);
    uint8_t top = check_bit(*reg, 7);
    uint8_t result = *reg >> 1;
    result = set_bit_to(result, 7, top);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = carry;
}

void srl(GameBoy* gameBoy, uint8_t* reg) {
    bool leastBitSet = check_bit(*reg, 0);
    uint8_t result = (*reg >> 1);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = leastBitSet;
}

void swap(GameBoy* gameBoy, uint8_t* reg) {
    uint8_t lower = *reg & 0x0f;
    uint8_t upper = (*reg & 0xf0) >> 4;
    uint8_t result = ((lower << 4) | upper);
    *reg = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = false;
}

void bit(GameBoy* gameBoy, uint8_t bit, uint8_t reg) {
    gameBoy->cpu.zero = !check_bit(reg, bit);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = true;
}

void set(GameBoy* gameBoy, uint8_t bit, uint8_t* reg) {
    *reg = set_bit(*reg, bit);
}

void res(GameBoy* gameBoy, uint8_t bit, uint8_t* reg) {
    *reg = reset_bit(*reg, bit);
}

int decodeAndExecuteCB(GameBoy* gameBoy, const uint8_t rawInstruction) {
    switch(rawInstruction) {
        case 0x00: {
            // RLC B
            printf("RLC B\n");
            rlc(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x01: {
            // RLC C
            printf("RLC C\n");
            rlc(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x02: {
            // RLC D
            printf("RLC D\n");
            rlc(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x03: {
            // RLC E
            printf("RLC E\n");
            rlc(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x04: {
            // RLC H
            printf("RLC H\n");
            rlc(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x05: {
            // RLC L
            printf("RLC L\n");
            rlc(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x06: {
            // RLC (HL)
            printf("RLC (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rlc(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x07: {
            // RLC A
            printf("RLC A\n");
            rlc(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x08: {
            // RRC B
            printf("RRC B\n");
            rrc(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x09: {
            // RRC C
            printf("RRC C\n");
            rrc(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x0a: {
            // RRC D
            printf("RRC D\n");
            rrc(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x0b: {
            // RRC E
            printf("RRC E\n");
            rrc(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x0c: {
            // RRC H
            printf("RRC H\n");
            rrc(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x0d: {
            // RRC L
            printf("RRC L\n");
            rrc(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x0e: {
            // RRC (HL)
            printf("RRC (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rrc(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x0f: {
            // RRC A
            printf("RRC A\n");
            rrc(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x10: {
            // RL B
            printf("RL B\n");
            rl(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x11: {
            // RL C
            printf("RL C\n");
            rl(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x12: {
            // RL D
            printf("RL D\n");
            rl(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x13: {
            // RL E
            printf("RL E\n");
            rl(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x14: {
            // RL H
            printf("RL H\n");
            rl(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x15: {
            // RL L
            printf("RL L\n");
            rl(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x16: {
            // RL (HL)
            printf("RL (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rl(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x17: {
            // RL A
            printf("RL A\n");
            rl(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x18: {
            // RR B
            printf("RR B\n");
            rr(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x19: {
            // RR C
            printf("RR C\n");
            rr(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x1a: {
            // RR D
            printf("RR D\n");
            rr(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x1b: {
            // RR E
            printf("RR E\n");
            rr(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x1c: {
            // RR H
            printf("RR H\n");
            rr(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x1d: {
            // RR L
            printf("RR L\n");
            rr(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x1e: {
            // RR (HL)
            printf("RR (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rr(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x1f: {
            // RR A
            printf("RR A\n");
            rr(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x20: {
            // SLA B
            printf("SLA B\n");
            sla(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x21: {
            // SLA C
            printf("SLA C\n");
            sla(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x22: {
            // SLA D
            printf("SLA D\n");
            sla(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x23: {
            // SLA E
            printf("SLA E\n");
            sla(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x24: {
            // SLA H
            printf("SLA H\n");
            sla(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x25: {
            // SLA L
            printf("SLA L\n");
            sla(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x26: {
            // SLA (HL)
            printf("SLA (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            sla(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x27: {
            // SLA A
            printf("SLA A\n");
            sla(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x28: {
            // SRA B
            printf("SRA B\n");
            sra(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x29: {
            // SRA C
            printf("SRA C\n");
            sra(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x2a: {
            // SRA D
            printf("SRA D\n");
            sra(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x2b: {
            // SRA E
            printf("SRA E\n");
            sra(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x2c: {
            // SRA H
            printf("SRA H\n");
            sra(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x2d: {
            // SRA L
            printf("SRA L\n");
            sra(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x2e: {
            // SRA (HL)
            printf("SRA (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            sra(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x2f: {
            // SRA A
            printf("SRA A\n");
            sra(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x30: {
            // SWAP B
            printf("SWAP B\n");
            swap(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x31: {
            // SWAP C
            printf("SWAP C\n");
            swap(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x32: {
            // SWAP D
            printf("SWAP D\n");
            swap(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x33: {
            // SWAP E
            printf("SWAP E\n");
            swap(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x34: {
            // SWAP H
            printf("SWAP H\n");
            swap(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x35: {
            // SWAP L
            printf("SWAP L\n");
            swap(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x36: {
            // SWAP (HL)
            printf("SWAP (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            swap(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x37: {
            // SWAP A
            printf("SWAP A\n");
            swap(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x38: {
            // SRL B
            printf("SRL B\n");
            srl(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x39: {
            // SRL C
            printf("SRL C\n");
            srl(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x3a: {
            // SRL D
            printf("SRL D\n");
            srl(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x3b: {
            // SRL E
            printf("SRL E\n");
            srl(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x3c: {
            // SRL H
            printf("SRL H\n");
            srl(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x3d: {
            // SRL L
            printf("SRL L\n");
            srl(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x3e: {
            // SRL (HL)
            printf("SRL (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            srl(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x3f: {
            // SRL A
            printf("SRL A\n");
            srl(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x40: {
            // BIT 0 B
            printf("BIT 0 B\n");
            bit(gameBoy, 0, gameBoy->cpu.b);
            return 8;
        }
        case 0x41: {
            // BIT 0 C
            printf("BIT 0 C\n");
            bit(gameBoy, 0, gameBoy->cpu.c);
            return 8;
        }
        case 0x42: {
            // BIT 0 D
            printf("BIT 0 D\n");
            bit(gameBoy, 0, gameBoy->cpu.d);
            return 8;
        }
        case 0x43: {
            // BIT 0 E
            printf("BIT 0 E\n");
            bit(gameBoy, 0, gameBoy->cpu.e);
            return 8;
        }
        case 0x44: {
            // BIT 0 H
            printf("BIT 0 H\n");
            bit(gameBoy, 0, gameBoy->cpu.h);
            return 8;
        }
        case 0x45: {
            // BIT 0 L
            printf("BIT 0 L\n");
            bit(gameBoy, 0, gameBoy->cpu.l);
            return 8;
        }
        case 0x46: {
            // BIT 0 (HL)
            printf("BIT 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 0, value);
            return 12;
        }
        case 0x47: {
            // BIT 0 A
            printf("BIT 0 A\n");
            bit(gameBoy, 0, gameBoy->cpu.a);
            return 8;
        }
        case 0x48: {
            // BIT 1 B
            printf("BIT 1 B\n");
            bit(gameBoy, 1, gameBoy->cpu.b);
            return 8;
        }
        case 0x49: {
            // BIT 1 C
            printf("BIT 1 C\n");
            bit(gameBoy, 1, gameBoy->cpu.c);
            return 8;
        }
        case 0x4a: {
            // BIT 1 D
            printf("BIT 1 D\n");
            bit(gameBoy, 1, gameBoy->cpu.d);
            return 8;
        }
        case 0x4b: {
            // BIT 1 E
            printf("BIT 1 E\n");
            bit(gameBoy, 1, gameBoy->cpu.e);
            return 8;
        }
        case 0x4c: {
            // BIT 1 H
            printf("BIT 1 H\n");
            bit(gameBoy, 1, gameBoy->cpu.h);
            return 8;
        }
        case 0x4d: {
            // BIT 1 L
            printf("BIT 1 L\n");
            bit(gameBoy, 1, gameBoy->cpu.l);
            return 8;
        }
        case 0x4e: {
            // BIT 1 (HL)
            printf("BIT 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 1, value);
            return 12;
        }
        case 0x4f: {
            // BIT 1 A
            printf("BIT 1 A\n");
            bit(gameBoy, 1, gameBoy->cpu.a);
            return 8;
        }
        case 0x50: {
            // BIT 2 B
            printf("BIT 2 B\n");
            bit(gameBoy, 2, gameBoy->cpu.b);
            return 8;
        }
        case 0x51: {
            // BIT 2 C
            printf("BIT 2 C\n");
            bit(gameBoy, 2, gameBoy->cpu.c);
            return 8;
        }
        case 0x52: {
            // BIT 2 D
            printf("BIT 2 D\n");
            bit(gameBoy, 2, gameBoy->cpu.d);
            return 8;
        }
        case 0x53: {
            // BIT 2 E
            printf("BIT 2 E\n");
            bit(gameBoy, 2, gameBoy->cpu.e);
            return 8;
        }
        case 0x54: {
            // BIT 2 H
            printf("BIT 2 H\n");
            bit(gameBoy, 2, gameBoy->cpu.h);
            return 8;
        }
        case 0x55: {
            // BIT 2 L
            printf("BIT 2 L\n");
            bit(gameBoy, 2, gameBoy->cpu.l);
            return 8;
        }
        case 0x56: {
            // BIT 2 (HL)
            printf("BIT 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 2, value);
            return 12;
        }
        case 0x57: {
            // BIT 2 A
            printf("BIT 2 A\n");
            bit(gameBoy, 2, gameBoy->cpu.a);
            return 8;
        }
        case 0x58: {
            // BIT 3 B
            printf("BIT 3 B\n");
            bit(gameBoy, 3, gameBoy->cpu.b);
            return 8;
        }
        case 0x59: {
            // BIT 3 C
            printf("BIT 3 C\n");
            bit(gameBoy, 3, gameBoy->cpu.c);
            return 8;
        }
        case 0x5a: {
            // BIT 3 D
            printf("BIT 3 D\n");
            bit(gameBoy, 3, gameBoy->cpu.d);
            return 8;
        }
        case 0x5b: {
            // BIT 3 E
            printf("BIT 3 E\n");
            bit(gameBoy, 3, gameBoy->cpu.e);
            return 8;
        }
        case 0x5c: {
            // BIT 3 H
            printf("BIT 3 H\n");
            bit(gameBoy, 3, gameBoy->cpu.h);
            return 8;
        }
        case 0x5d: {
            // BIT 3 L
            printf("BIT 3 L\n");
            bit(gameBoy, 3, gameBoy->cpu.l);
            return 8;
        }
        case 0x5e: {
            // BIT 3 (HL)
            printf("BIT 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 3, value);
            return 12;
        }
        case 0x5f: {
            // BIT 3 A
            printf("BIT 3 A\n");
            bit(gameBoy, 3, gameBoy->cpu.a);
            return 8;
        }
        case 0x60: {
            // BIT 4 B
            printf("BIT 4 B\n");
            bit(gameBoy, 4, gameBoy->cpu.b);
            return 8;
        }
        case 0x61: {
            // BIT 4 C
            printf("BIT 4 C\n");
            bit(gameBoy, 4, gameBoy->cpu.c);
            return 8;
        }
        case 0x62: {
            // BIT 4 D
            printf("BIT 4 D\n");
            bit(gameBoy, 4, gameBoy->cpu.d);
            return 8;
        }
        case 0x63: {
            // BIT 4 E
            printf("BIT 4 E\n");
            bit(gameBoy, 4, gameBoy->cpu.e);
            return 8;
        }
        case 0x64: {
            // BIT 4 H
            printf("BIT 4 H\n");
            bit(gameBoy, 4, gameBoy->cpu.h);
            return 8;
        }
        case 0x65: {
            // BIT 4 L
            printf("BIT 4 L\n");
            bit(gameBoy, 4, gameBoy->cpu.l);
            return 8;
        }
        case 0x66: {
            // BIT 4 (HL)
            printf("BIT 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 4, value);
            return 12;
        }
        case 0x67: {
            // BIT 4 A
            printf("BIT 4 A\n");
            bit(gameBoy, 4, gameBoy->cpu.a);
            return 8;
        }
        case 0x68: {
            // BIT 5 B
            printf("BIT 5 B\n");
            bit(gameBoy, 5, gameBoy->cpu.b);
            return 8;
        }
        case 0x69: {
            // BIT 5 C
            printf("BIT 5 C\n");
            bit(gameBoy, 5, gameBoy->cpu.c);
            return 8;
        }
        case 0x6a: {
            // BIT 5 D
            printf("BIT 5 D\n");
            bit(gameBoy, 5, gameBoy->cpu.d);
            return 8;
        }
        case 0x6b: {
            // BIT 5 E
            printf("BIT 5 E\n");
            bit(gameBoy, 5, gameBoy->cpu.e);
            return 8;
        }
        case 0x6c: {
            // BIT 5 H
            printf("BIT 5 H\n");
            bit(gameBoy, 5, gameBoy->cpu.h);
            return 8;
        }
        case 0x6d: {
            // BIT 5 L
            printf("BIT 5 L\n");
            bit(gameBoy, 5, gameBoy->cpu.l);
            return 8;
        }
        case 0x6e: {
            // BIT 5 (HL)
            printf("BIT 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 5, value);
            return 12;
        }
        case 0x6f: {
            // BIT 5 A
            printf("BIT 5 A\n");
            bit(gameBoy, 5, gameBoy->cpu.a);
            return 8;
        }
        case 0x70: {
            // BIT 6 B
            printf("BIT 6 B\n");
            bit(gameBoy, 6, gameBoy->cpu.b);
            return 8;
        }
        case 0x71: {
            // BIT 6 C
            printf("BIT 6 C\n");
            bit(gameBoy, 6, gameBoy->cpu.c);
            return 8;
        }
        case 0x72: {
            // BIT 6 D
            printf("BIT 6 D\n");
            bit(gameBoy, 6, gameBoy->cpu.d);
            return 8;
        }
        case 0x73: {
            // BIT 6 E
            printf("BIT 6 E\n");
            bit(gameBoy, 6, gameBoy->cpu.e);
            return 8;
        }
        case 0x74: {
            // BIT 6 H
            printf("BIT 6 H\n");
            bit(gameBoy, 6, gameBoy->cpu.h);
            return 8;
        }
        case 0x75: {
            // BIT 6 L
            printf("BIT 6 L\n");
            bit(gameBoy, 6, gameBoy->cpu.l);
            return 8;
        }
        case 0x76: {
            // BIT 6 (HL)
            printf("BIT 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 6, value);
            return 12;
        }
        case 0x77: {
            // BIT 6 A
            printf("BIT 6 A\n");
            bit(gameBoy, 6, gameBoy->cpu.a);
            return 8;
        }
        case 0x78: {
            // BIT 7 B
            printf("BIT 7 B\n");
            bit(gameBoy, 7, gameBoy->cpu.b);
            return 8;
        }
        case 0x79: {
            // BIT 7 C
            printf("BIT 7 C\n");
            bit(gameBoy, 7, gameBoy->cpu.c);
            return 8;
        }
        case 0x7a: {
            // BIT 7 D
            printf("BIT 7 D\n");
            bit(gameBoy, 7, gameBoy->cpu.d);
            return 8;
        }
        case 0x7b: {
            // BIT 7 E
            printf("BIT 7 E\n");
            bit(gameBoy, 7, gameBoy->cpu.e);
            return 8;
        }
        case 0x7c: {
            // BIT 7 H
            printf("BIT 7 H\n");
            bit(gameBoy, 7, gameBoy->cpu.h);
            return 8;
        }
        case 0x7d: {
            // BIT 7 L
            printf("BIT 7 L\n");
            bit(gameBoy, 7, gameBoy->cpu.l);
            return 8;
        }
        case 0x7e: {
            // BIT 7 (HL)
            printf("BIT 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 7, value);
            return 12;
        }
        case 0x7f: {
            // BIT 7 A
            printf("BIT 7 A\n");
            bit(gameBoy, 7, gameBoy->cpu.a);
            return 8;
        }
        case 0x80: {
            // RES 0 B
            printf("RES 0 B\n");
            res(gameBoy, 0, &gameBoy->cpu.b);
            return 8;
        }
        case 0x81: {
            // RES 0 C
            printf("RES 0 C\n");
            res(gameBoy, 0, &gameBoy->cpu.c);
            return 8;
        }
        case 0x82: {
            // RES 0 D
            printf("RES 0 D\n");
            res(gameBoy, 0, &gameBoy->cpu.d);
            return 8;
        }
        case 0x83: {
            // RES 0 E
            printf("RES 0 E\n");
            res(gameBoy, 0, &gameBoy->cpu.e);
            return 8;
        }
        case 0x84: {
            // RES 0 H
            printf("RES 0 H\n");
            res(gameBoy, 0, &gameBoy->cpu.h);
            return 8;
        }
        case 0x85: {
            // RES 0 L
            printf("RES 0 L\n");
            res(gameBoy, 0, &gameBoy->cpu.l);
            return 8;
        }
        case 0x86: {
            // RES 0 (HL)
            printf("RES 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 0, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x87: {
            // RES 0 A
            printf("RES 0 A\n");
            res(gameBoy, 0, &gameBoy->cpu.a);
            return 8;
        }
        case 0x88: {
            // RES 1 B
            printf("RES 1 B\n");
            res(gameBoy, 1, &gameBoy->cpu.b);
            return 8;
        }
        case 0x89: {
            // RES 1 C
            printf("RES 1 C\n");
            res(gameBoy, 1, &gameBoy->cpu.c);
            return 8;
        }
        case 0x8a: {
            // RES 1 D
            printf("RES 1 D\n");
            res(gameBoy, 1, &gameBoy->cpu.d);
            return 8;
        }
        case 0x8b: {
            // RES 1 E
            printf("RES 1 E\n");
            res(gameBoy, 1, &gameBoy->cpu.e);
            return 8;
        }
        case 0x8c: {
            // RES 1 H
            printf("RES 1 H\n");
            res(gameBoy, 1, &gameBoy->cpu.h);
            return 8;
        }
        case 0x8d: {
            // RES 1 L
            printf("RES 1 L\n");
            res(gameBoy, 1, &gameBoy->cpu.l);
            return 8;
        }
        case 0x8e: {
            // RES 1 (HL)
            printf("RES 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 1, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x8f: {
            // RES 1 A
            printf("RES 1 A\n");
            res(gameBoy, 1, &gameBoy->cpu.a);
            return 8;
        }
        case 0x90: {
            // RES 2 B
            printf("RES 2 B\n");
            res(gameBoy, 2, &gameBoy->cpu.b);
            return 8;
        }
        case 0x91: {
            // RES 2 C
            printf("RES 2 C\n");
            res(gameBoy, 2, &gameBoy->cpu.c);
            return 8;
        }
        case 0x92: {
            // RES 2 D
            printf("RES 2 D\n");
            res(gameBoy, 2, &gameBoy->cpu.d);
            return 8;
        }
        case 0x93: {
            // RES 2 E
            printf("RES 2 E\n");
            res(gameBoy, 2, &gameBoy->cpu.e);
            return 8;
        }
        case 0x94: {
            // RES 2 H
            printf("RES 2 H\n");
            res(gameBoy, 2, &gameBoy->cpu.h);
            return 8;
        }
        case 0x95: {
            // RES 2 L
            printf("RES 2 L\n");
            res(gameBoy, 2, &gameBoy->cpu.l);
            return 8;
        }
        case 0x96: {
            // RES 2 (HL)
            printf("RES 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 2, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x97: {
            // RES 2 A
            printf("RES 2 A\n");
            res(gameBoy, 2, &gameBoy->cpu.a);
            return 8;
        }
        case 0x98: {
            // RES 3 B
            printf("RES 3 B\n");
            res(gameBoy, 3, &gameBoy->cpu.b);
            return 8;
        }
        case 0x99: {
            // RES 3 C
            printf("RES 3 C\n");
            res(gameBoy, 3, &gameBoy->cpu.c);
            return 8;
        }
        case 0x9a: {
            // RES 3 D
            printf("RES 3 D\n");
            res(gameBoy, 3, &gameBoy->cpu.d);
            return 8;
        }
        case 0x9b: {
            // RES 3 E
            printf("RES 3 E\n");
            res(gameBoy, 3, &gameBoy->cpu.e);
            return 8;
        }
        case 0x9c: {
            // RES 3 H
            printf("RES 3 H\n");
            res(gameBoy, 3, &gameBoy->cpu.h);
            return 8;
        }
        case 0x9d: {
            // RES 3 L
            printf("RES 3 L\n");
            res(gameBoy, 3, &gameBoy->cpu.l);
            return 8;
        }
        case 0x9e: {
            // RES 3 (HL)
            printf("RES 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 3, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x9f: {
            // RES 3 A
            printf("RES 3 A\n");
            res(gameBoy, 3, &gameBoy->cpu.a);
            return 8;
        }
        case 0xa0: {
            // RES 4 B
            printf("RES 4 B\n");
            res(gameBoy, 4, &gameBoy->cpu.b);
            return 8;
        }
        case 0xa1: {
            // RES 4 C
            printf("RES 4 C\n");
            res(gameBoy, 4, &gameBoy->cpu.c);
            return 8;
        }
        case 0xa2: {
            // RES 4 D
            printf("RES 4 D\n");
            res(gameBoy, 4, &gameBoy->cpu.d);
            return 8;
        }
        case 0xa3: {
            // RES 4 E
            printf("RES 4 E\n");
            res(gameBoy, 4, &gameBoy->cpu.e);
            return 8;
        }
        case 0xa4: {
            // RES 4 H
            printf("RES 4 H\n");
            res(gameBoy, 4, &gameBoy->cpu.h);
            return 8;
        }
        case 0xa5: {
            // RES 4 L
            printf("RES 4 L\n");
            res(gameBoy, 4, &gameBoy->cpu.l);
            return 8;
        }
        case 0xa6: {
            // RES 4 (HL)
            printf("RES 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 4, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xa7: {
            // RES 4 A
            printf("RES 4 A\n");
            res(gameBoy, 4, &gameBoy->cpu.a);
            return 8;
        }
        case 0xa8: {
            // RES 5 B
            printf("RES 5 B\n");
            res(gameBoy, 5, &gameBoy->cpu.b);
            return 8;
        }
        case 0xa9: {
            // RES 5 C
            printf("RES 5 C\n");
            res(gameBoy, 5, &gameBoy->cpu.c);
            return 8;
        }
        case 0xaa: {
            // RES 5 D
            printf("RES 5 D\n");
            res(gameBoy, 5, &gameBoy->cpu.d);
            return 8;
        }
        case 0xab: {
            // RES 5 E
            printf("RES 5 E\n");
            res(gameBoy, 5, &gameBoy->cpu.e);
            return 8;
        }
        case 0xac: {
            // RES 5 H
            printf("RES 5 H\n");
            res(gameBoy, 5, &gameBoy->cpu.h);
            return 8;
        }
        case 0xad: {
            // RES 5 L
            printf("RES 5 L\n");
            res(gameBoy, 5, &gameBoy->cpu.l);
            return 8;
        }
        case 0xae: {
            // RES 5 (HL)
            printf("RES 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 5, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xaf: {
            // RES 5 A
            printf("RES 5 A\n");
            res(gameBoy, 5, &gameBoy->cpu.a);
            return 8;
        }
        case 0xb0: {
            // RES 6 B
            printf("RES 6 B\n");
            res(gameBoy, 6, &gameBoy->cpu.b);
            return 8;
        }
        case 0xb1: {
            // RES 6 C
            printf("RES 6 C\n");
            res(gameBoy, 6, &gameBoy->cpu.c);
            return 8;
        }
        case 0xb2: {
            // RES 6 D
            printf("RES 6 D\n");
            res(gameBoy, 6, &gameBoy->cpu.d);
            return 8;
        }
        case 0xb3: {
            // RES 6 E
            printf("RES 6 E\n");
            res(gameBoy, 6, &gameBoy->cpu.e);
            return 8;
        }
        case 0xb4: {
            // RES 6 H
            printf("RES 6 H\n");
            res(gameBoy, 6, &gameBoy->cpu.h);
            return 8;
        }
        case 0xb5: {
            // RES 6 L
            printf("RES 6 L\n");
            res(gameBoy, 6, &gameBoy->cpu.l);
            return 8;
        }
        case 0xb6: {
            // RES 6 (HL)
            printf("RES 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 6, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xb7: {
            // RES 6 A
            printf("RES 6 A\n");
            res(gameBoy, 6, &gameBoy->cpu.a);
            return 8;
        }
        case 0xb8: {
            // RES 7 B
            printf("RES 7 B\n");
            res(gameBoy, 7, &gameBoy->cpu.b);
            return 8;
        }
        case 0xb9: {
            // RES 7 C
            printf("RES 7 C\n");
            res(gameBoy, 7, &gameBoy->cpu.c);
            return 8;
        }
        case 0xba: {
            // RES 7 D
            printf("RES 7 D\n");
            res(gameBoy, 7, &gameBoy->cpu.d);
            return 8;
        }
        case 0xbb: {
            // RES 7 E
            printf("RES 7 E\n");
            res(gameBoy, 7, &gameBoy->cpu.e);
            return 8;
        }
        case 0xbc: {
            // RES 7 H
            printf("RES 7 H\n");
            res(gameBoy, 7, &gameBoy->cpu.h);
            return 8;
        }
        case 0xbd: {
            // RES 7 L
            printf("RES 7 L\n");
            res(gameBoy, 7, &gameBoy->cpu.l);
            return 8;
        }
        case 0xbe: {
            // RES 7 (HL)
            printf("RES 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 7, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xbf: {
            // RES 7 A
            printf("RES 7 A\n");
            res(gameBoy, 7, &gameBoy->cpu.a);
            return 8;
        }
        case 0xc0: {
            // SET 0 B
            printf("SET 0 B\n");
            set(gameBoy, 0, &gameBoy->cpu.b);
            return 8;
        }
        case 0xc1: {
            // SET 0 C
            printf("SET 0 C\n");
            set(gameBoy, 0, &gameBoy->cpu.c);
            return 8;
        }
        case 0xc2: {
            // SET 0 D
            printf("SET 0 D\n");
            set(gameBoy, 0, &gameBoy->cpu.d);
            return 8;
        }
        case 0xc3: {
            // SET 0 E
            printf("SET 0 E\n");
            set(gameBoy, 0, &gameBoy->cpu.e);
            return 8;
        }
        case 0xc4: {
            // SET 0 H
            printf("SET 0 H\n");
            set(gameBoy, 0, &gameBoy->cpu.h);
            return 8;
        }
        case 0xc5: {
            // SET 0 L
            printf("SET 0 L\n");
            set(gameBoy, 0, &gameBoy->cpu.l);
            return 8;
        }
        case 0xc6: {
            // SET 0 (HL)
            printf("SET 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 0, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xc7: {
            // SET 0 A
            printf("SET 0 A\n");
            set(gameBoy, 0, &gameBoy->cpu.a);
            return 8;
        }
        case 0xc8: {
            // SET 1 B
            printf("SET 1 B\n");
            set(gameBoy, 1, &gameBoy->cpu.b);
            return 8;
        }
        case 0xc9: {
            // SET 1 C
            printf("SET 1 C\n");
            set(gameBoy, 1, &gameBoy->cpu.c);
            return 8;
        }
        case 0xca: {
            // SET 1 D
            printf("SET 1 D\n");
            set(gameBoy, 1, &gameBoy->cpu.d);
            return 8;
        }
        case 0xcb: {
            // SET 1 E
            printf("SET 1 E\n");
            set(gameBoy, 1, &gameBoy->cpu.e);
            return 8;
        }
        case 0xcc: {
            // SET 1 H
            printf("SET 1 H\n");
            set(gameBoy, 1, &gameBoy->cpu.h);
            return 8;
        }
        case 0xcd: {
            // SET 1 L
            printf("SET 1 L\n");
            set(gameBoy, 1, &gameBoy->cpu.l);
            return 8;
        }
        case 0xce: {
            // SET 1 (HL)
            printf("SET 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 1, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xcf: {
            // SET 1 A
            printf("SET 1 A\n");
            set(gameBoy, 1, &gameBoy->cpu.a);
            return 8;
        }
        case 0xd0: {
            // SET 2 B
            printf("SET 2 B\n");
            set(gameBoy, 2, &gameBoy->cpu.b);
            return 8;
        }
        case 0xd1: {
            // SET 2 C
            printf("SET 2 C\n");
            set(gameBoy, 2, &gameBoy->cpu.c);
            return 8;
        }
        case 0xd2: {
            // SET 2 D
            printf("SET 2 D\n");
            set(gameBoy, 2, &gameBoy->cpu.d);
            return 8;
        }
        case 0xd3: {
            // SET 2 E
            printf("SET 2 E\n");
            set(gameBoy, 2, &gameBoy->cpu.e);
            return 8;
        }
        case 0xd4: {
            // SET 2 H
            printf("SET 2 H\n");
            set(gameBoy, 2, &gameBoy->cpu.h);
            return 8;
        }
        case 0xd5: {
            // SET 2 L
            printf("SET 2 L\n");
            set(gameBoy, 2, &gameBoy->cpu.l);
            return 8;
        }
        case 0xd6: {
            // SET 2 (HL)
            printf("SET 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 2, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xd7: {
            // SET 2 A
            printf("SET 2 A\n");
            set(gameBoy, 2, &gameBoy->cpu.a);
            return 8;
        }
        case 0xd8: {
            // SET 3 B
            printf("SET 3 B\n");
            set(gameBoy, 3, &gameBoy->cpu.b);
            return 8;
        }
        case 0xd9: {
            // SET 3 C
            printf("SET 3 C\n");
            set(gameBoy, 3, &gameBoy->cpu.c);
            return 8;
        }
        case 0xda: {
            // SET 3 D
            printf("SET 3 D\n");
            set(gameBoy, 3, &gameBoy->cpu.d);
            return 8;
        }
        case 0xdb: {
            // SET 3 E
            printf("SET 3 E\n");
            set(gameBoy, 3, &gameBoy->cpu.e);
            return 8;
        }
        case 0xdc: {
            // SET 3 H
            printf("SET 3 H\n");
            set(gameBoy, 3, &gameBoy->cpu.h);
            return 8;
        }
        case 0xdd: {
            // SET 3 L
            printf("SET 3 L\n");
            set(gameBoy, 3, &gameBoy->cpu.l);
            return 8;
        }
        case 0xde: {
            // SET 3 (HL)
            printf("SET 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 3, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xdf: {
            // SET 3 A
            printf("SET 3 A\n");
            set(gameBoy, 3, &gameBoy->cpu.a);
            return 8;
        }
        case 0xe0: {
            // SET 4 B
            printf("SET 4 B\n");
            set(gameBoy, 4, &gameBoy->cpu.b);
            return 8;
        }
        case 0xe1: {
            // SET 4 C
            printf("SET 4 C\n");
            set(gameBoy, 4, &gameBoy->cpu.c);
            return 8;
        }
        case 0xe2: {
            // SET 4 D
            printf("SET 4 D\n");
            set(gameBoy, 4, &gameBoy->cpu.d);
            return 8;
        }
        case 0xe3: {
            // SET 4 E
            printf("SET 4 E\n");
            set(gameBoy, 4, &gameBoy->cpu.e);
            return 8;
        }
        case 0xe4: {
            // SET 4 H
            printf("SET 4 H\n");
            set(gameBoy, 4, &gameBoy->cpu.h);
            return 8;
        }
        case 0xe5: {
            // SET 4 L
            printf("SET 4 L\n");
            set(gameBoy, 4, &gameBoy->cpu.l);
            return 8;
        }
        case 0xe6: {
            // SET 4 (HL)
            printf("SET 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 4, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xe7: {
            // SET 4 A
            printf("SET 4 A\n");
            set(gameBoy, 4, &gameBoy->cpu.a);
            return 8;
        }
        case 0xe8: {
            // SET 5 B
            printf("SET 5 B\n");
            set(gameBoy, 5, &gameBoy->cpu.b);
            return 8;
        }
        case 0xe9: {
            // SET 5 C
            printf("SET 5 C\n");
            set(gameBoy, 5, &gameBoy->cpu.c);
            return 8;
        }
        case 0xea: {
            // SET 5 D
            printf("SET 5 D\n");
            set(gameBoy, 5, &gameBoy->cpu.d);
            return 8;
        }
        case 0xeb: {
            // SET 5 E
            printf("SET 5 E\n");
            set(gameBoy, 5, &gameBoy->cpu.e);
            return 8;
        }
        case 0xec: {
            // SET 5 H
            printf("SET 5 H\n");
            set(gameBoy, 5, &gameBoy->cpu.h);
            return 8;
        }
        case 0xed: {
            // SET 5 L
            printf("SET 5 L\n");
            set(gameBoy, 5, &gameBoy->cpu.l);
            return 8;
        }
        case 0xee: {
            // SET 5 (HL)
            printf("SET 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 5, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xef: {
            // SET 5 A
            printf("SET 5 A\n");
            set(gameBoy, 5, &gameBoy->cpu.a);
            return 8;
        }
        case 0xf0: {
            // SET 6 B
            printf("SET 6 B\n");
            set(gameBoy, 6, &gameBoy->cpu.b);
            return 8;
        }
        case 0xf1: {
            // SET 6 C
            printf("SET 6 C\n");
            set(gameBoy, 6, &gameBoy->cpu.c);
            return 8;
        }
        case 0xf2: {
            // SET 6 D
            printf("SET 6 D\n");
            set(gameBoy, 6, &gameBoy->cpu.d);
            return 8;
        }
        case 0xf3: {
            // SET 6 E
            printf("SET 6 E\n");
            set(gameBoy, 6, &gameBoy->cpu.e);
            return 8;
        }
        case 0xf4: {
            // SET 6 H
            printf("SET 6 H\n");
            set(gameBoy, 6, &gameBoy->cpu.h);
            return 8;
        }
        case 0xf5: {
            // SET 6 L
            printf("SET 6 L\n");
            set(gameBoy, 6, &gameBoy->cpu.l);
            return 8;
        }
        case 0xf6: {
            // SET 6 (HL)
            printf("SET 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 6, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xf7: {
            // SET 6 A
            printf("SET 6 A\n");
            set(gameBoy, 6, &gameBoy->cpu.a);
            return 8;
        }
        case 0xf8: {
            // SET 7 B
            printf("SET 7 B\n");
            set(gameBoy, 7, &gameBoy->cpu.b);
            return 8;
        }
        case 0xf9: {
            // SET 7 C
            printf("SET 7 C\n");
            set(gameBoy, 7, &gameBoy->cpu.c);
            return 8;
        }
        case 0xfa: {
            // SET 7 D
            printf("SET 7 D\n");
            set(gameBoy, 7, &gameBoy->cpu.d);
            return 8;
        }
        case 0xfb: {
            // SET 7 E
            printf("SET 7 E\n");
            set(gameBoy, 7, &gameBoy->cpu.e);
            return 8;
        }
        case 0xfc: {
            // SET 7 H
            printf("SET 7 H\n");
            set(gameBoy, 7, &gameBoy->cpu.h);
            return 8;
        }
        case 0xfd: {
            // SET 7 L
            printf("SET 7 L\n");
            set(gameBoy, 7, &gameBoy->cpu.l);
            return 8;
        }
        case 0xfe: {
            // SET 7 (HL)
            printf("SET 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 7, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xff: {
            // SET 7 A
            printf("SET 7 A\n");
            set(gameBoy, 7, &gameBoy->cpu.a);
            return 8;
        }
    }
}

void ld_word(uint8_t* lowerDes, uint8_t* upperDes, uint8_t lower, uint8_t upper) {
    *lowerDes = lower;
    *upperDes = upper;
}

void ld_byte(uint8_t* des, uint8_t src) { *des = src; }

void inc_word(uint8_t* lower, uint8_t* upper) {
    uint16_t word = compose_bytes(*lower, *upper);
    word++;
    *upper = (uint8_t) (word >> 8);
    *lower = (uint8_t) (word);
}

void dec_word(uint8_t* lower, uint8_t* upper) {
    uint16_t word = compose_bytes(*lower, *upper);
    word--;
    *upper = (uint8_t) (word >> 8);
    *lower = (uint8_t) (word);
}

void inc_byte(GameBoy* gameBoy, uint8_t* reg) {
    (*reg)++;
    gameBoy->cpu.zero = (*reg == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = ((*reg & 0x0f) == 0x00);
}

void dec_byte(GameBoy* gameBoy, uint8_t* reg) {
    (*reg)--;
    gameBoy->cpu.zero = (*reg == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = ((*reg & 0x0f) == 0x0f);
}

void add_word(GameBoy* gameBoy, uint8_t* lowerDes, uint8_t* upperDes, uint8_t lower, uint8_t upper) {
    uint16_t desWord = compose_bytes(*lowerDes, *upperDes);
    uint16_t srcWord = compose_bytes(lower, upper);
    uint result = (desWord + srcWord);
    *upperDes = (uint8_t) (result >> 8);
    *lowerDes = (uint8_t) (result);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = ((desWord & 0xfff) + (srcWord & 0xfff) > 0xfff);
    gameBoy->cpu.carry = ((result & 0x10000) != 0);
}

void add_byte(GameBoy* gameBoy, uint8_t* des, uint8_t addend) {
    uint8_t first = *des;
    uint8_t second = addend;
    uint result = first + second;
    *des = (uint8_t) result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = (((first & 0xf) + (second & 0xf)) > 0xf);
    gameBoy->cpu.carry = ((result & 0x100) != 0);
}

void adc_byte(GameBoy* gameBoy, uint8_t* des, uint8_t addend) {
    uint8_t first = *des;
    uint8_t second = addend;
    uint8_t carry = gameBoy->cpu.carry;
    uint result = first + second + carry;
    *des = (uint8_t) result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = (((first & 0xf) + (second & 0xf) + carry) > 0xf);
    gameBoy->cpu.carry = (result > 0xff);
}

void sub_byte(GameBoy* gameBoy, uint8_t* des, uint8_t subtrahend) {
    uint8_t first = *des;
    uint8_t second = subtrahend;
    uint8_t result = first - second;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = (((first & 0xf) - (second & 0xf)) < 0);
    gameBoy->cpu.carry = (first < second);
}

void sbc_byte(GameBoy* gameBoy, uint8_t* des, uint8_t subtrahend) {
    uint8_t first = *des;
    uint8_t second = subtrahend;
    uint8_t carry = gameBoy->cpu.carry;
    int result = (first - second - carry);
    *des = (uint8_t) result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = (((first & 0xf) - (second & 0xf) - carry) < 0);
    gameBoy->cpu.carry = (result < 0);
}

void and_byte(GameBoy* gameBoy, uint8_t* des, uint8_t value) {
    uint8_t result = *des & value;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = true;
    gameBoy->cpu.carry = false;
}

void xor_byte(GameBoy* gameBoy, uint8_t* des, uint8_t value) {
    uint8_t result = *des ^ value;
    *des = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = false;
}

void or_byte(GameBoy* gameBoy, uint8_t* des, uint8_t value) {
    uint8_t result = *des | value;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = false;
}

void cp_byte(GameBoy* gameBoy, uint8_t des, uint8_t value) {
    uint8_t first = des;
    uint8_t second = value;
    uint8_t result = first - second;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = (((first & 0xf) - (second & 0xf)) < 0);
    gameBoy->cpu.carry = (first < second);
}

void pop(GameBoy* gameBoy, uint8_t* lower, uint8_t* upper) {
    *lower = readFromMemory(gameBoy, gameBoy->cpu.sp++);
    *upper = readFromMemory(gameBoy, gameBoy->cpu.sp++);
}

void push(GameBoy* gameBoy, uint8_t lower, uint8_t upper) {
    writeToMemory(gameBoy, --gameBoy->cpu.sp, upper);
    writeToMemory(gameBoy, --gameBoy->cpu.sp, lower);
}

void ret(GameBoy* gameBoy) {
    uint8_t lower = 0;
    uint8_t upper = 0;
    pop(gameBoy, &lower, &upper);
    jp_from_bytes(gameBoy, lower, upper);
}

void jp_from_word(GameBoy* gameBoy, uint16_t address) { gameBoy->cpu.pc = address; }

void jp_from_bytes(GameBoy* gameBoy, uint8_t lower, uint8_t upper) { jp_from_word(gameBoy, compose_bytes(lower, upper)); }

void jp_from_pc(GameBoy* gameBoy) {
    uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    jp_from_bytes(gameBoy, lower, upper);
}

void call(GameBoy* gameBoy) {
    uint8_t lowerNew = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint8_t upperNew = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint16_t nextInstruction = gameBoy->cpu.pc;
    uint8_t upperNext = (uint8_t) (nextInstruction >> 8);
    uint8_t lowerNext = (uint8_t) (nextInstruction);
    push(gameBoy, lowerNext, upperNext);
    jp_from_bytes(gameBoy, lowerNew, upperNew);
}

void rst(GameBoy* gameBoy, uint8_t value) {
    uint8_t upper = (uint8_t) (gameBoy->cpu.pc >> 8);
    uint8_t lower = (uint8_t) (gameBoy->cpu.pc);
    push(gameBoy, lower, upper);
    jp_from_word(gameBoy, 0x0000 + value);
}

void jr(GameBoy* gameBoy) {
    int8_t value = (int8_t) readFromMemory(gameBoy, gameBoy->cpu.pc++);
    jp_from_word(gameBoy, gameBoy->cpu.pc + value);
}

int decodeAndExecute(GameBoy* gameBoy, const uint8_t rawInstruction) {
    switch(rawInstruction) {
        case 0x00: {
            // NOP
            printf("NOP\n");
            return 4;
        }
        case 0x01: {
            // LD BC, u16
            printf("LD BC, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.c, &gameBoy->cpu.b, lower, upper);
            return 12;
        }
        case 0x02: {
            // LD (BC), A
            printf("LD (BC), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.c, gameBoy->cpu.b), gameBoy->cpu.a);
            return 8;
        }
        case 0x03: {
            // INC BC
            printf("INC BC\n");
            inc_word(&gameBoy->cpu.c, &gameBoy->cpu.b);
            return 8;
        }
        case 0x04: {
            // INC B
            printf("INC B\n");
            inc_byte(gameBoy, &gameBoy->cpu.b);
            return 4;
        }
        case 0x05: {
            // DEC B
            printf("DEC B\n");
            dec_byte(gameBoy, &gameBoy->cpu.b);
            return 4;
        }
        case 0x06: {
            // LD B, u8
            printf("LD B, u8\n");
            ld_byte(&gameBoy->cpu.b, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x07: {
            // RLCA
            printf("RLCA\n");
            rlc(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x08: {
            // LD (u16), SP
            printf("LD (u16), SP\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t address = compose_bytes(lower, upper);
            writeToMemory(gameBoy, address, (uint8_t) (gameBoy->cpu.sp));
            writeToMemory(gameBoy, address + 1, (uint8_t) (gameBoy->cpu.sp >> 8));
            return 20;
        }
        case 0x09: {
            // ADD HL, BC
            printf("ADD HL, BC\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.c, gameBoy->cpu.b);
            return 8;
        }
        case 0x0a: {
            // LD A, (BC)
            printf("LD A, (BC)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.c, gameBoy->cpu.b)));
            return 8;
        }
        case 0x0b: {
            // DEC BC
            printf("DEC BC\n");
            dec_word(&gameBoy->cpu.c, &gameBoy->cpu.b);
            return 8;
        }
        case 0x0c: {
            // INC C
            printf("INC C\n");
            inc_byte(gameBoy, &gameBoy->cpu.c);
            return 4;
        }
        case 0x0d: {
            // DEC C
            printf("DEC C\n");
            dec_byte(gameBoy, &gameBoy->cpu.c);
            return 4;
        }
        case 0x0e: {
            // LD C, u8
            printf("LD C, u8\n");
            ld_byte(&gameBoy->cpu.c, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x0f: {
            // RRCA
            printf("RRCA\n");
            rrc(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x10: {
            // STOP
            printf("STOP\n");
            /*
            gameBoy->cpu.halted = true;
            gameBoy->cpu.pc++;
            */
            return 4;
        }
        case 0x11: {
            // LD DE, u16
            printf("LD DE, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.e, &gameBoy->cpu.d, lower, upper);
            return 12;
        }
        case 0x12: {
            // LD (DE), A
            printf("LD (DE), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.e, gameBoy->cpu.d), gameBoy->cpu.a);
            return 8;
        }
        case 0x13: {
            // INC DE
            printf("INC DE\n");
            inc_word(&gameBoy->cpu.e, &gameBoy->cpu.d);
            return 8;
        }
        case 0x14: {
            // INC D
            printf("INC D\n");
            inc_byte(gameBoy, &gameBoy->cpu.d);
            return 4;
        }
        case 0x15: {
            // DEC D
            printf("DEC D\n");
            dec_byte(gameBoy, &gameBoy->cpu.d);
            return 4;
        }
        case 0x16: {
            // LD D, u8
            printf("LD D, u8\n");
            ld_byte(&gameBoy->cpu.d, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x17: {
            // RLA
            printf("RLA\n");
            rl(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x18: {
            // JR i8
            printf("JR i8\n");
            jr(gameBoy);
            return 12;
        }
        case 0x19: {
            // ADD HL, DE
            printf("ADD HL, DE\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.e, gameBoy->cpu.d);
            return 8;
        }
        case 0x1a: {
            // LD A, (DE)
            printf("LD A, (DE)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.e, gameBoy->cpu.d)));
            return 8;
        }
        case 0x1b: {
            // DEC DE
            printf("DEC DE\n");
            dec_word(&gameBoy->cpu.e, &gameBoy->cpu.d);
            return 8;
        }
        case 0x1c: {
            // INC E
            printf("INC E\n");
            inc_byte(gameBoy, &gameBoy->cpu.e);
            return 4;
        }
        case 0x1d: {
            // DEC E
            printf("DEC E\n");
            dec_byte(gameBoy, &gameBoy->cpu.e);
            return 4;
        }
        case 0x1e: {
            // LD E, u8
            printf("LD E, u8\n");
            ld_byte(&gameBoy->cpu.e, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x1f: {
            // RRA
            printf("RRA\n");
            rr(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x20: {
            // JR NZ, i8
            printf("JR NZ, i8\n");
            if(!gameBoy->cpu.zero) {
                jr(gameBoy);
                return 12;
            } else {
                gameBoy->cpu.pc++;
                return 8;
            }
        }
        case 0x21: {
            // LD HL, u16
            printf("LD HL, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.l, &gameBoy->cpu.h, lower, upper);
            return 12;
        }
        case 0x22: {
            // LD (HL+), A
            printf("LD (HL+), A\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl++, gameBoy->cpu.a);
            gameBoy->cpu.h = (uint8_t) (hl >> 8);
            gameBoy->cpu.l = (uint8_t) (hl);
            return 8;
        }
        case 0x23: {
            // INC HL
            printf("INC HL\n");
            inc_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x24: {
            // INC H
            printf("INC H\n");
            inc_byte(gameBoy, &gameBoy->cpu.h);
            return 4;
        }
        case 0x25: {
            // DEC H
            printf("DEC H\n");
            dec_byte(gameBoy, &gameBoy->cpu.h);
            return 4;
        }
        case 0x26: {
            // LD H, u8
            printf("LD H, u8\n");
            ld_byte(&gameBoy->cpu.h, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x27: {
            // DAA
            printf("DAA\n");
            if(!gameBoy->cpu.subtract) {
                if(gameBoy->cpu.carry || gameBoy->cpu.a > 0x99) {
                    gameBoy->cpu.a += 0x60;
                    gameBoy->cpu.carry = true;
                }
                if(gameBoy->cpu.halfCarry || (gameBoy->cpu.a & 0xf) > 0x9) {
                    gameBoy->cpu.a += 0x06;
                    gameBoy->cpu.halfCarry = false;
                }
            } else if(gameBoy->cpu.carry && gameBoy->cpu.halfCarry) {
                gameBoy->cpu.a += 0x9a;
                gameBoy->cpu.halfCarry = false;
            } else if(gameBoy->cpu.carry) {
                gameBoy->cpu.a += 0xa0;
            } else if(gameBoy->cpu.halfCarry) {
                gameBoy->cpu.a += 0xfa;
                gameBoy->cpu.halfCarry = false;
            }
            gameBoy->cpu.zero = (gameBoy->cpu.a == 0);
            /*
            uint16_t correction = (gameBoy->cpu.carry ? 0x60 : 0x00);

            if(gameBoy->cpu.halfCarry || (!gameBoy->cpu.subtract && ((gameBoy->cpu.a & 0x0f) > 9)))
                correction |= 0x06;
            if(gameBoy->cpu.carry || (!gameBoy->cpu.subtract && (gameBoy->cpu.a > 0x99)))
                correction |= 0x60;

            if(gameBoy->cpu.subtract)
                gameBoy->cpu.a = ((uint8_t) (gameBoy->cpu.a - correction));
            else
                gameBoy->cpu.a = ((uint8_t) (gameBoy->cpu.a + correction));

            gameBoy->cpu.zero = (gameBoy->cpu.a == 0);
            gameBoy->cpu.halfCarry = false;
            gameBoy->cpu.carry = (((correction << 2) & 0x100) != 0);
            */
            return 4;
        }
        case 0x28: {
            // JR Z, i8
            printf("JR Z, i8\n");
            if(gameBoy->cpu.zero) {
                jr(gameBoy);
                return 12;
            } else {
                gameBoy->cpu.pc++;
                return 8;
            }
        }
        case 0x29: {
            // ADD HL, HL
            printf("ADD HL, HL\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.l, gameBoy->cpu.h);
            return 8;
        }
        case 0x2a: {
            // LD A, (HL+)
            printf("LD A, (HL+)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, hl++));
            gameBoy->cpu.h = (uint8_t) (hl >> 8);
            gameBoy->cpu.l = (uint8_t) (hl);
            return 8;
        }
        case 0x2b: {
            // DEC HL
            printf("DEC HL\n");
            dec_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x2c: {
            // INC L
            printf("INC L\n");
            inc_byte(gameBoy, &gameBoy->cpu.l);
            return 4;
        }
        case 0x2d: {
            // DEC L
            printf("DEC L\n");
            dec_byte(gameBoy, &gameBoy->cpu.l);
            return 4;
        }
        case 0x2e: {
            // LD L, u8
            printf("LD L, u8\n");
            ld_byte(&gameBoy->cpu.l, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x2f: {
            // CPL
            printf("CPL\n");
            gameBoy->cpu.a = ~gameBoy->cpu.a;
            gameBoy->cpu.subtract = true;
            gameBoy->cpu.halfCarry = true;
            return 4;
        }
        case 0x30: {
            // JR NC, i8
            printf("JR NC, i8\n");
            if(!gameBoy->cpu.carry) {
                jr(gameBoy);
                return 12;
            } else {
                gameBoy->cpu.pc++;
                return 8;
            }
        }
        case 0x31: {
            // LD SP, u16
            printf("LD SP, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            gameBoy->cpu.sp = compose_bytes(lower, upper);
            return 12;
        }
        case 0x32: {
            // LD (HL-), A
            printf("LD (HL-), A\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl--, gameBoy->cpu.a);
            gameBoy->cpu.h = (uint8_t) (hl >> 8);
            gameBoy->cpu.l = (uint8_t) (hl);
            return 8;
        }
        case 0x33: {
            // INC SP
            printf("INC SP\n");
            gameBoy->cpu.sp++;
            return 8;
        }
        case 0x34: {
            // INC (HL)
            printf("INC (HL)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl, readFromMemory(gameBoy, hl) + 1);
            gameBoy->cpu.zero = (readFromMemory(gameBoy, hl) == 0);
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = ((readFromMemory(gameBoy, hl) & 0x0f) == 0x00);
            return 12;
        }
        case 0x35: {
            // DEC (HL)
            printf("DEC (HL)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl, readFromMemory(gameBoy, hl) - 1);
            gameBoy->cpu.zero = (readFromMemory(gameBoy, hl) == 0);
            gameBoy->cpu.subtract = true;
            gameBoy->cpu.halfCarry = ((readFromMemory(gameBoy, hl) & 0x0f) == 0x0f);
            return 12;
        }
        case 0x36: {
            // LD (HL), u8
            printf("LD (HL), u8\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 12;
        }
        case 0x37: {
            // SCF
            printf("SCF\n");
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = false;
            gameBoy->cpu.carry = true;
            return 4;
        }
        case 0x38: {
            // JR C, i8
            printf("JR C, i8\n");
            if(gameBoy->cpu.carry) {
                jr(gameBoy);
                return 12;
            } else {
                gameBoy->cpu.pc++;
                return 8;
            }
        }
        case 0x39: {
            // ADD HL, SP
            printf("ADD HL, SP\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint result = (hl + gameBoy->cpu.sp);
            gameBoy->cpu.h = (uint8_t) (result >> 8);
            gameBoy->cpu.l = (uint8_t) result;
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = ((hl & 0xfff) + (gameBoy->cpu.sp & 0xfff) > 0xfff);
            gameBoy->cpu.carry = ((result & 0x10000) != 0);
            return 8;
        }
        case 0x3a: {
            // LD A, (HL-)
            printf("LD A, (HL-)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, hl--));
            gameBoy->cpu.h = (uint8_t) (hl >> 8);
            gameBoy->cpu.l = (uint8_t) (hl);
            return 8;
        }
        case 0x3b: {
            // DEC SP
            printf("DEC SP\n");
            gameBoy->cpu.sp--;
            return 8;
        }
        case 0x3c: {
            // INC A
            printf("INC A\n");
            inc_byte(gameBoy, &gameBoy->cpu.a);
            return 4;
        }
        case 0x3d: {
            // DEC A
            printf("DEC A\n");
            dec_byte(gameBoy, &gameBoy->cpu.a);
            return 4;
        }
        case 0x3e: {
            // LD A, u8
            printf("LD A, u8\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x3f: {
            // CCF
            printf("CCF\n");
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = false;
            gameBoy->cpu.carry = !gameBoy->cpu.carry;
            return 4;
        }
        case 0x40: {
            // LD B, B
            printf("LD B, B\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.b);
            return 4;
        }
        case 0x41: {
            // LD B, C
            printf("LD B, C\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.c);
            return 4;
        }
        case 0x42: {
            // LD B, D
            printf("LD B, D\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.d);
            return 4;
        }
        case 0x43: {
            // LD B, E
            printf("LD B, E\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.e);
            return 4;
        }
        case 0x44: {
            // LD B, H
            printf("LD B, H\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.h);
            return 4;
        }
        case 0x45: {
            // LD B, L
            printf("LD B, L\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.l);
            return 4;
        }
        case 0x46: {
            // LD B, (HL)
            printf("LD B, (HL)\n");
            ld_byte(&gameBoy->cpu.b, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x47: {
            // LD B, A
            printf("LD B, A\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.a);
            return 4;
        }
        case 0x48: {
            // LD C, B
            printf("LD C, B\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.b);
            return 4;
        }
        case 0x49: {
            // LD C, C
            printf("LD C, C\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.c);
            return 4;
        }
        case 0x4a: {
            // LD C, D
            printf("LD C, D\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.d);
            return 4;
        }
        case 0x4b: {
            // LD C, E
            printf("LD C, E\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.e);
            return 4;
        }
        case 0x4c: {
            // LD C, H
            printf("LD C, H\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.h);
            return 4;
        }
        case 0x4d: {
            // LD C, L
            printf("LD C, L\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.l);
            return 4;
        }
        case 0x4e: {
            // LD C, (HL)
            printf("LD C, (HL)\n");
            ld_byte(&gameBoy->cpu.c, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x4f: {
            // LD C, A
            printf("LD C, A\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.a);
            return 4;
        }
        case 0x50: {
            // LD D, B
            printf("LD D, B\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.b);
            return 4;
        }
        case 0x51: {
            // LD D, C
            printf("LD D, C\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.c);
            return 4;
        }
        case 0x52: {
            // LD D, D
            printf("LD D, D\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.d);
            return 4;
        }
        case 0x53: {
            // LD D, E
            printf("LD D, E\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.e);
            return 4;
        }
        case 0x54: {
            // LD D, H
            printf("LD D, H\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.h);
            return 4;
        }
        case 0x55: {
            // LD D, L
            printf("LD D, L\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.l);
            return 4;
        }
        case 0x56: {
            // LD D, (HL)
            printf("LD D, (HL)\n");
            ld_byte(&gameBoy->cpu.d, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x57: {
            // LD D, A
            printf("LD D, A\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.a);
            return 4;
        }
        case 0x58: {
            // LD E, B
            printf("LD E, B\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.b);
            return 4;
        }
        case 0x59: {
            // LD E, C
            printf("LD E, C\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.c);
            return 4;
        }
        case 0x5a: {
            // LD E, D
            printf("LD E, D\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.d);
            return 4;
        }
        case 0x5b: {
            // LD E, E
            printf("LD E, E\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.e);
            return 4;
        }
        case 0x5c: {
            // LD E, H
            printf("LD E, H\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.h);
            return 4;
        }
        case 0x5d: {
            // LD E, L
            printf("LD E, L\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.l);
            return 4;
        }
        case 0x5e: {
            // LD E, (HL)
            printf("LD E, (HL)\n");
            ld_byte(&gameBoy->cpu.e, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x5f: {
            // LD E, A
            printf("LD E, A\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.a);
            return 4;
        }
        case 0x60: {
            // LD H, B
            printf("LD H, B\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.b);
            return 4;
        }
        case 0x61: {
            // LD H, C
            printf("LD H, C\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.c);
            return 4;
        }
        case 0x62: {
            // LD H, D
            printf("LD H, D\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.d);
            return 4;
        }
        case 0x63: {
            // LD H, E
            printf("LD H, E\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.e);
            return 4;
        }
        case 0x64: {
            // LD H, H
            printf("LD H, H\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.h);
            return 4;
        }
        case 0x65: {
            // LD H, L
            printf("LD H, L\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.l);
            return 4;
        }
        case 0x66: {
            // LD H, (HL)
            printf("LD H, (HL)\n");
            ld_byte(&gameBoy->cpu.h, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x67: {
            // LD H, A
            printf("LD H, A\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.a);
            return 4;
        }
        case 0x68: {
            // LD L, B
            printf("LD L, B\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.b);
            return 4;
        }
        case 0x69: {
            // LD L, C
            printf("LD L, C\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.c);
            return 4;
        }
        case 0x6a: {
            // LD L, D
            printf("LD L, D\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.d);
            return 4;
        }
        case 0x6b: {
            // LD L, E
            printf("LD L, E\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.e);
            return 4;
        }
        case 0x6c: {
            // LD L, H
            printf("LD L, H\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.h);
            return 4;
        }
        case 0x6d: {
            // LD L, L
            printf("LD L, L\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.l);
            return 4;
        }
        case 0x6e: {
            // LD L, (HL)
            printf("LD L, (HL)\n");
            ld_byte(&gameBoy->cpu.l, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x6f: {
            // LD L, A
            printf("LD L, A\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.a);
            return 4;
        }
        case 0x70: {
            // LD (HL), B
            printf("LD (HL), B\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.b);
            return 8;
        }
        case 0x71: {
            // LD (HL), C
            printf("LD (HL), C\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.c);
            return 8;
        }
        case 0x72: {
            // LD (HL), D
            printf("LD (HL), D\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.d);
            return 8;
        }
        case 0x73: {
            // LD (HL), E
            printf("LD (HL), E\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.e);
            return 8;
        }
        case 0x74: {
            // LD (HL), H
            printf("LD (HL), H\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.h);
            return 8;
        }
        case 0x75: {
            // LD (HL), L
            printf("LD (HL), L\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.l);
            return 8;
        }
        case 0x76: {
            // HALT
            printf("HALT\n");
            gameBoy->cpu.halted = true;
            return 4;
        }
        case 0x77: {
            // LD (HL), A
            printf("LD (HL), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.a);
            return 8;
        }
        case 0x78: {
            // LD A, B
            printf("LD A, B\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x79: {
            // LD A, C
            printf("LD A, C\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x7a: {
            // LD A, D
            printf("LD A, D\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x7b: {
            // LD A, E
            printf("LD A, E\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x7c: {
            // LD A, H
            printf("LD A, H\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x7d: {
            // LD A, L
            printf("LD A, L\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x7e: {
            // LD A, (HL)
            printf("LD A, (HL)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x7f: {
            // LD A, A
            printf("LD A, A\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x80: {
            // ADD A, B
            printf("ADD A, B\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x81: {
            // ADD A, C
            printf("ADD A, C\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x82: {
            // ADD A, D
            printf("ADD A, D\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x83: {
            // ADD A, E
            printf("ADD A, E\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x84: {
            // ADD A, H
            printf("ADD A, H\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x85: {
            // ADD A, L
            printf("ADD A, L\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x86: {
            // ADD A, (HL)
            printf("ADD A, (HL)\n");
            add_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x87: {
            // ADD A, A
            printf("ADD A, A\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x88: {
            // ADC A, B
            printf("ADC A, B\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x89: {
            // ADC A, C
            printf("ADC A, C\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x8a: {
            // ADC A, D
            printf("ADC A, D\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x8b: {
            // ADC A, E
            printf("ADC A, E\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x8c: {
            // ADC A, H
            printf("ADC A, H\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x8d: {
            // ADC A, L
            printf("ADC A, L\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x8e: {
            // ADC A, (HL)
            printf("ADC A, (HL)\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x8f: {
            // ADC A, A
            printf("ADC A, A\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x90: {
            // SUB B
            printf("SUB B\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x91: {
            // SUB C
            printf("SUB C\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x92: {
            // SUB D
            printf("SUB D\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x93: {
            // SUB E
            printf("SUB E\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x94: {
            // SUB H
            printf("SUB H\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x95: {
            // SUB L
            printf("SUB L\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x96: {
            // SUB (HL)
            printf("SUB (HL)\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x97: {
            // SUB A
            printf("SUB A\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x98: {
            // SBC A, B
            printf("SBC A, B\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x99: {
            // SBC A, C
            printf("SBC A, C\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x9a: {
            // SBC A, D
            printf("SBC A, D\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x9b: {
            // SBC A, E
            printf("SBC A, E\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x9c: {
            // SBC A, H
            printf("SBC A, H\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x9d: {
            // SBC A, L
            printf("SBC A, L\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x9e: {
            // SBC A, (HL)
            printf("SBC A, (HL)\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x9f: {
            // SBC A, A
            printf("SBC A, A\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xa0: {
            // AND B
            printf("AND B\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xa1: {
            // AND C
            printf("AND C\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xa2: {
            // AND D
            printf("AND D\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xa3: {
            // AND E
            printf("AND E\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xa4: {
            // AND H
            printf("AND H\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xa5: {
            // AND L
            printf("AND L\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xa6: {
            // AND (HL)
            printf("AND (HL)\n");
            and_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xa7: {
            // AND A
            printf("AND A\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xa8: {
            // XOR B
            printf("XOR B\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);            
            return 4;
        }
        case 0xa9: {
            // XOR C
            printf("XOR C\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xaa: {
            // XOR D
            printf("XOR D\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xab: {
            // XOR E
            printf("XOR E\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xac: {
            // XOR H
            printf("XOR H\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xad: {
            // XOR L
            printf("XOR L\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xae: {
            // XOR (HL)
            printf("XOR (HL)\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xaf: {
            // XOR A
            printf("XOR A\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xb0: {
            // OR B
            printf("OR B\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xb1: {
            // OR C
            printf("OR C\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xb2: {
            // OR D
            printf("OR D\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xb3: {
            // OR E
            printf("OR E\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xb4: {
            // OR H
            printf("OR H\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xb5: {
            // OR L
            printf("OR L\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xb6: {
            // OR (HL)
            printf("OR (HL)\n");
            or_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xb7: {
            // OR A
            printf("OR A\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xb8: {
            // CP B
            printf("CP B\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xb9: {
            // CP C
            printf("CP C\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xba: {
            // CP D
            printf("CP D\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xbb: {
            // CP E
            printf("CP E\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xbc: {
            // CP H
            printf("CP H\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xbd: {
            // CP L
            printf("CP L\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xbe: {
            // CP (HL)
            printf("CP (HL)\n");
            cp_byte(gameBoy, gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xbf: {
            // CP A
            printf("CP A\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xc0: {
            // RET NZ
            printf("RET NZ\n");
            if(!gameBoy->cpu.zero) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xc1: {
            // POP BC
            printf("POP BC\n");
            pop(gameBoy, &gameBoy->cpu.c, &gameBoy->cpu.b);
            return 12;
        }
        case 0xc2: {
            // JP NZ, u16
            printf("JP NZ, u16\n");
            if(!gameBoy->cpu.zero) {
                jp_from_pc(gameBoy);
                return 16;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xc3: {
            // JP u16
            printf("JP u16\n");
            jp_from_pc(gameBoy);
            return 16;
        }
        case 0xc4: {
            // CALL NZ, u16
            printf("CALL NZ, u16\n");
            if(!gameBoy->cpu.zero) {
                call(gameBoy);
                return 24;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xc5: {
            // PUSH BC
            printf("PUSH BC\n");
            push(gameBoy, gameBoy->cpu.c, gameBoy->cpu.b);
            return 16;
        }
        case 0xc6: {
            // ADD A, u8
            printf("ADD A, u8\n");
            add_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xc7: {
            // RST 00H
            printf("RST 00H\n");
            rst(gameBoy, 0x00);
            return 16;
        }
        case 0xc8: {
            // RET Z
            printf("RET Z\n");
            if(gameBoy->cpu.zero) {
                ret(gameBoy);
                return 20;
            } else {
                return 8;
            }
        }
        case 0xc9: {
            // RET
            printf("RET\n");
            ret(gameBoy);
            return 16;
        }
        case 0xca: {
            // JP Z, u16
            printf("JP Z, u16\n");
            if(gameBoy->cpu.zero) {
                jp_from_pc(gameBoy);
                return 16;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xcb: {
            // PREFIX CB
            printf("CB\n");
            int cycles = 4;
            uint8_t instruction = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            return cycles + decodeAndExecuteCB(gameBoy, instruction);
        }
        case 0xcc: {
            // CALL Z, u16
            printf("CALL Z, u16\n");
            if(gameBoy->cpu.zero) {
                call(gameBoy);
                return 24;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xcd: {
            // CALL u16
            printf("CALL u16\n");
            call(gameBoy);
            return 24;
        }
        case 0xce: {
            // ADC A, u8
            printf("ADC A, u8\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xcf: {
            // RST 08H
            printf("RST 08H\n");
            rst(gameBoy, 0x08);
            return 16;
        }
        case 0xd0: {
            // RET NC
            printf("RET NC\n");
            if(!gameBoy->cpu.carry) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xd1: {
            // POP DE
            printf("POP DE\n");
            pop(gameBoy, &gameBoy->cpu.e, &gameBoy->cpu.d);
            return 12;
        }
        case 0xd2: {
            // JP NC, u16
            printf("JP NC, u16\n");
            if(!gameBoy->cpu.carry) {
                jp_from_pc(gameBoy);
                return 16;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xd3: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xd3\n");
            return 0;
        }
        case 0xd4: {
            // CALL NC, u16
            printf("CALL NC, u16\n");
            if(!gameBoy->cpu.carry) {
                call(gameBoy);
                return 24;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xd5: {
            // PUSH DE
            printf("PUSH DE\n");
            push(gameBoy, gameBoy->cpu.e, gameBoy->cpu.d);
            return 16;
        }
        case 0xd6: {
            // SUB u8
            printf("SUB u8\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xd7: {
            // RST 10H
            printf("RST 10H\n");
            rst(gameBoy, 0x10);
            return 16;
        }
        case 0xd8: {
            // RET C
            printf("RET C\n");
            if(gameBoy->cpu.carry) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xd9: {
            // RETI
            printf("RETI\n");
            ret(gameBoy);
            gameBoy->cpu.interrupts = true;
            return 16;
        }
        case 0xda: {
            // JP C, u16
            printf("JP C, u16\n");
            if(gameBoy->cpu.carry) {
                jp_from_pc(gameBoy);
                return 16;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xdb: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xdb\n");
            return 0;
        }
        case 0xdc: {
            // CALL C, u16
            printf("CALL C, u16\n");
            if(gameBoy->cpu.carry) {
                call(gameBoy);
                return 24;
            } else {
                gameBoy->cpu.pc += 2;
                return 12;
            }
        }
        case 0xdd: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xdd\n");
            return 0;
        }
        case 0xde: {
            // SBC A, u8
            printf("SBC A, u8\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xdf: {
            // RST 18H
            printf("RST 18H\n");
            rst(gameBoy, 0x18);
            return 16;
        }
        case 0xe0: {
            // LDH (u8), A
            printf("LDH (u8), A\n");
            writeToMemory(gameBoy, 0xff00 + readFromMemory(gameBoy, gameBoy->cpu.pc++), gameBoy->cpu.a);
            return 12;
        }
        case 0xe1: {
            // POP HL
            printf("POP HL\n");
            pop(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h);
            return 12;
        }
        case 0xe2: {
            // LD (C), A
            printf("LD (C), A\n");
            writeToMemory(gameBoy, 0xff00 + gameBoy->cpu.c, gameBoy->cpu.a);
            return 8;
        }
        case 0xe3: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xe3\n");
            return 0;
        }
        case 0xe4: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xe4\n");
            return 0;
        }
        case 0xe5: {
            // PUSH HL
            printf("PUSH HL\n");
            push(gameBoy, gameBoy->cpu.l, gameBoy->cpu.h);
            return 16;
        }
        case 0xe6: {
            // AND u8
            printf("AND u8\n");
            and_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xe7: {
            // RST 20H
            printf("RST 20H\n");
            rst(gameBoy, 0x20);
            return 16;
        }
        case 0xe8: {
            // ADD SP, i8
            printf("ADD SP, i8\n");
            uint16_t sp = gameBoy->cpu.sp;
            int8_t value = (int8_t) readFromMemory(gameBoy, gameBoy->cpu.pc++);
            int result = sp + value;
            gameBoy->cpu.sp = (uint16_t) result;
            gameBoy->cpu.zero = false;
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = (((sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
            gameBoy->cpu.carry = (((sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
            return 16;
        }
        case 0xe9: {
            // JP HL
            printf("JP HL\n");
            jp_from_bytes(gameBoy, gameBoy->cpu.l, gameBoy->cpu.h);
            return 4;
        }
        case 0xea: {
            // LD (u16), A
            printf("LD (u16), A\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            writeToMemory(gameBoy, compose_bytes(lower, upper), gameBoy->cpu.a);
            return 16;
        }
        case 0xeb: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xeb\n");
            return 0;
        }
        case 0xec: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xec\n");
            return 0;
        }
        case 0xed: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xed\n");
            return 0;
        }
        case 0xee: {
            // XOR u8
            printf("XOR u8\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xef: {
            // RST 28H
            printf("RST 28H\n");
            rst(gameBoy, 0x28);
            return 16;
        }
        case 0xf0: {
            // LDH A, (u8)
            uint8_t offset = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t address = 0xff00 + offset;
            printf("LDH A, (%u)\n", offset);
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, address));
            return 12;
        }
        case 0xf1: {
            // POP AF
            printf("POP AF\n");
            uint8_t f = 0;
            pop(gameBoy, &f, &gameBoy->cpu.a);
            setFFlagsFromByte(&gameBoy->cpu, f);
            return 12;
        }
        case 0xf2: {
            // LD A, (C)
            printf("LD A, (C)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, 0xff00 + gameBoy->cpu.c));
            return 8;
        }
        case 0xf3: {
            // DI
            printf("DI\n");
            //gameBoy->cpu.interrupts = false;
            gameBoy->cpu.pendingInterruptDisable = true;
            return 4;
        }
        case 0xf4: {
            // Blank Instruction
            printf("BLANK INSTRUCTION AT 0xf4\n");
            return 0;
        }
        case 0xf5: {
            // PUSH AF
            printf("PUSH AF\n");
            push(gameBoy, getFFlagsAsByte(&gameBoy->cpu), gameBoy->cpu.a);
            return 16;
        }
        case 0xf6: {
            // OR u8
            printf("OR u8\n");
            or_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xf7: {
            // RST 30H
            printf("RST 30H\n");
            rst(gameBoy, 0x30);
            return 16;
        }
        case 0xf8: {
            // LD HL, SP + i8
            printf("LD HL, SP + i8");
            int8_t value = (int8_t) readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t result = gameBoy->cpu.sp + value;
            gameBoy->cpu.h = (uint8_t) (result >> 8);
            gameBoy->cpu.l = (uint8_t) result;
            gameBoy->cpu.zero = false;
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = (((gameBoy->cpu.sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
            gameBoy->cpu.carry = (((gameBoy->cpu.sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
            return 12;
        }
        case 0xf9: {
            // LD SP, HL
            printf("LD SP, HL\n");
            gameBoy->cpu.sp = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            return 8;
        }
        case 0xfa: {
            // LD A, (u16)
            printf("LD A, (u16)\n");
            uint8_t lower = gameBoy->cpu.pc++;
            uint8_t upper = gameBoy->cpu.pc++;
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(lower, upper)));
            return 16;
        }
        case 0xfb: {
            // EI
            printf("EI\n");
            //gameBoy->cpu.interrupts = true;
            gameBoy->cpu.pendingInterruptEnable = true;
            return 4;
        }
        case 0xfc: {
            // Blank Instruction
            printf("BLANK INSTRUCTION at 0xfc\n");
            return 0;
        }
        case 0xfd: {
            // Blank Instruction
            printf("BLANK INSTRUCTION at 0xfd\n");
            return 0;
        }
        case 0xfe: {
            // CP u8
            printf("CP u8\n");
            cp_byte(gameBoy, gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 4;
        }
        case 0xff: {
            // RST 38H
            printf("RST 38H\n");
            rst(gameBoy, 0x38);
            return 16;
        }
    }
}

void doDMATransfer(GameBoy* gameBoy, uint8_t value) {
    uint16_t address = ((uint16_t) value) << 8;
    for(uint8_t i = 0; i < 0xa0; i++)
        writeToMemory(gameBoy, 0xfe00 + i, readFromMemory(gameBoy, address + i));
}

void doRAMBankEnable(GameBoy* gameBoy, uint16_t address, uint8_t value) {
    if(gameBoy->mBC2)
        if(bit_value(address, 4) == 1)
            return;
    uint8_t testData = (value & 0xf);
    if(testData == 0xa)
        gameBoy->enableRAM = true;
    else if(testData == 0x0)
        gameBoy->enableRAM = false;
}

void doChangeLoROMBank(GameBoy* gameBoy, uint8_t value) {
    if(gameBoy->mBC2) {
        gameBoy->currentROMBank = (value & 0xf);
        if(gameBoy->currentROMBank == 0)
            gameBoy->currentROMBank++;
        return;
    }
    uint8_t lower5 = value & 31;
    gameBoy->currentROMBank &= 224;
    gameBoy->currentROMBank |= lower5;
    if(gameBoy->currentROMBank == 0)
        gameBoy->currentROMBank++;
}

void doChangeHiROMBank(GameBoy* gameBoy, uint8_t value) {
    gameBoy->currentROMBank &= 31;
    value &= 224;
    gameBoy->currentROMBank |= value;
    if(gameBoy->currentROMBank == 0)
        gameBoy->currentROMBank++;
}

void doRAMBankChange(GameBoy* gameBoy, uint8_t value) {
    gameBoy->currentRAMBank = (value & 0x3);
}

void doChangeROMRAMMode(GameBoy* gameBoy, uint8_t value) {
    uint8_t newValue = value & 0x1;
    gameBoy->romBanking = (newValue == 0);
    if(gameBoy->romBanking)
        gameBoy->currentRAMBank = 0;
}

void handleBanking(GameBoy* gameBoy, uint16_t address, uint8_t value) {
    if(address < 0x2000) {
        if(gameBoy->mBC1 || gameBoy->mBC2)
            doRAMBankEnable(gameBoy, address, value);
    } else if((address >= 0x2000) && (address < 0x4000)) {
        if(gameBoy->mBC1 || gameBoy->mBC2)
            doChangeLoROMBank(gameBoy, value);
    } else if((address >= 0x4000) && (address < 0x6000)) {
        if(gameBoy->mBC1) {
            if(gameBoy->romBanking)
                doChangeHiROMBank(gameBoy, value);
            else
                doRAMBankChange(gameBoy, value);
        }
    } else if((address >= 0x6000) && (address < 0x8000)) {
        if(gameBoy->mBC1)
            doChangeROMRAMMode(gameBoy, value);
    }
}

uint8_t readFromMemory(GameBoy* gameBoy, uint16_t address) {
    /*
    if((address >= 0x4000) && (address <= 0x7fff)) {
        uint16_t newAddress = address - 0x4000;
        return gameBoy->cartridge[newAddress + (gameBoy->currentROMBank * 0x4000)];
    } else if((address >= 0xa000) && (address <= 0xbfff)) {
        uint16_t newAddress = address - 0xa000;
        return gameBoy->ramBanks[newAddress + (gameBoy->currentRAMBank * 0x2000)];
    } else */if((address >= 0xfea0) && (address < 0xfeff)) {
        return 0xff;
    } else if(address == 0xffff) {
        return gameBoy->cpu.interrupts;
    } else
        return gameBoy->rom[address];
}

void writeToMemory(GameBoy* gameBoy, uint16_t address, uint8_t value) {
    /*
    if(address < 0x8000) {
        handleBanking(gameBoy, address, value);
    } else if((address >= 0xa000) && (address < 0xc000)) {
        if(gameBoy->enableRAM) {
            uint16_t newAddress = address - 0xa000;
            gameBoy->ramBanks[newAddress + (gameBoy->currentRAMBank * 0x2000)] = value;
        }
    } else */if((address >= 0xfea0) && (address < 0xfeff)) {
    } else if((address >= 0xc000) && (address < 0xe000)) {
        gameBoy->rom[address] = value;
        gameBoy->rom[address + 0x2000] = value;
    } else if((address >= 0xe000) && (address < 0xfe00)) {
        gameBoy->rom[address] = value;
        gameBoy->rom[address - 0x2000] = value;
    } else if(address == TMC) {
        uint8_t currentFreq = getClockFreq(gameBoy);
        gameBoy->rom[address] = value;
        uint8_t newFreq = getClockFreq(gameBoy);
        if(currentFreq != newFreq)
            setClockFreq(gameBoy);
    } else if((address == 0xff04) || (address == 0xff44)) {
        gameBoy->rom[address] = 0;
    } else if(address == 0xff46) {
        doDMATransfer(gameBoy, value);
    } else if(address == 0xffff) {
        gameBoy->cpu.interrupts = value;
    } else
        gameBoy->rom[address] = value;
}

void setFFlagsFromByte(CPU* cpu, uint8_t newF) {
    cpu->carry = check_bit(newF, 4);
    cpu->halfCarry = check_bit(newF, 5);
    cpu->subtract = check_bit(newF, 6);
    cpu->zero = check_bit(newF, 7);
}

uint8_t getFFlagsAsByte(CPU* cpu) {
    uint8_t flag = 0x0;
    flag = set_bit_to(flag, 7, cpu->zero);
    flag = set_bit_to(flag, 6, cpu->subtract);
    flag = set_bit_to(flag, 5, cpu->halfCarry);
    flag = set_bit_to(flag, 4, cpu->carry);
    return flag;
}

int update(GameBoy* gameBoy) {
    int cycles = 0;
    if(gameBoy->cpu.pendingInterruptEnable) {
        uint8_t instruction = readFromMemory(gameBoy, gameBoy->cpu.pc++);
        printf("Instruction: %x\n", instruction);
        cycles += decodeAndExecute(gameBoy, instruction);
        if(!gameBoy->cpu.interrupts)
            gameBoy->cpu.interrupts = true;
        gameBoy->cpu.pendingInterruptEnable = false;
    }else if(gameBoy->cpu.pendingInterruptDisable) {
        uint8_t instruction = readFromMemory(gameBoy, gameBoy->cpu.pc++);
        printf("Instruction: %x\n", instruction);
        cycles += decodeAndExecute(gameBoy, instruction);
        if(gameBoy->cpu.interrupts)
            gameBoy->cpu.interrupts = false;
        gameBoy->cpu.pendingInterruptDisable = false;
    } else {
        uint8_t instruction = readFromMemory(gameBoy, gameBoy->cpu.pc++);
        printf("Instruction: %x\n", instruction);
        cycles += decodeAndExecute(gameBoy, instruction);
    }
    return cycles;
}

void updateTimer(GameBoy* gameBoy, int cycles) {
    doDividerRegister(gameBoy, cycles);
    if(isClockEnabled(gameBoy)) {
        gameBoy->timerCounter -= cycles;
        if(gameBoy->timerCounter <= 0) {
            setClockFreq(gameBoy);
            if(readFromMemory(gameBoy, TIMA) == 255) {
                writeToMemory(gameBoy, TIMA, readFromMemory(gameBoy, TMA));
                requestInterrupt(gameBoy, 2);
            } else
                writeToMemory(gameBoy, TIMA, readFromMemory(gameBoy, TIMA) + 1);
        }
    }
}

bool isClockEnabled(GameBoy* gameBoy) { return bit_value(readFromMemory(gameBoy, TMC), 2) ? true : false; }

uint8_t getClockFreq(GameBoy* gameBoy) { return readFromMemory(gameBoy, TMC) & 0x3; }

void setClockFreq(GameBoy* gameBoy) {
    uint8_t freq = getClockFreq(gameBoy);
    switch(freq) {
        case 0: gameBoy->timerCounter = 1024 ; break ; // freq 4096
        case 1: gameBoy->timerCounter = 16 ; break ;// freq 262144
        case 2: gameBoy->timerCounter = 64 ; break ;// freq 65536
        case 3: gameBoy->timerCounter = 256 ; break ;// freq 16382
    }
}

void doDividerRegister(GameBoy* gameBoy, int cycles) {
    gameBoy->dividerCounter += cycles;
    if(gameBoy->dividerCounter >= 255) {
        gameBoy->dividerCounter = 0;
        gameBoy->rom[0xff04]++;
    }
}

void requestInterrupt(GameBoy* gameBoy, int id) {
    uint8_t req = readFromMemory(gameBoy, 0xff0f);
    req = set_bit(req, id);
    writeToMemory(gameBoy, 0xff0f, req);
}

void serviceInterrupt(GameBoy* gameBoy, int interrupt) {
    if(!gameBoy->cpu.interrupts && gameBoy->cpu.halted) {
        gameBoy->cpu.halted = false;
        return;
    }
    gameBoy->cpu.interrupts = false;
    gameBoy->cpu.halted = false;
    uint8_t req = readFromMemory(gameBoy, 0xff0f);
    req = reset_bit(req, interrupt);
    writeToMemory(gameBoy, 0xff0f, req);

    push(gameBoy, (uint8_t) gameBoy->cpu.pc, (uint8_t) (gameBoy->cpu.pc >> 8));

    switch(interrupt) {
        case 0: gameBoy->cpu.pc = 0x0040; break;
        case 1: gameBoy->cpu.pc = 0x0048; break;
        case 2: gameBoy->cpu.pc = 0x0050; break;
        case 3: gameBoy->cpu.pc = 0x0058; break;
        case 4: gameBoy->cpu.pc = 0x0060; break;
    }
}

int doInterrupts(GameBoy* gameBoy) {
    if(!gameBoy->cpu.interrupts && !gameBoy->cpu.halted)
        return 0;

    if(gameBoy->cpu.interrupts || gameBoy->cpu.halted) {
        uint8_t req = readFromMemory(gameBoy, 0xff0f);
        uint8_t enabled = readFromMemory(gameBoy, 0xffff);
        if(req > 0)
            for(int i = 0; i < 5; i++)
                if(bit_value(req, i))
                    if(bit_value(enabled, i)) {
                        serviceInterrupt(gameBoy, i);
                        return 20;
                    }
    }
    /*
    if(!gameBoy->cpu.interrupts && gameBoy->cpu.halted) {
        gameBoy->cpu.halted = false;
        return 0;
    }
    */
    return 0;
}

bool isLCDEnabled(GameBoy* gameBoy) { return bit_value(readFromMemory(gameBoy, 0xff40), 7); }

void setLCDStatus(GameBoy* gameBoy) {
    uint8_t status = readFromMemory(gameBoy, 0xff41);
    if(!isLCDEnabled(gameBoy)) {
        gameBoy->scanlineCounter = SCANLINE_COUNTER_START;
        gameBoy->rom[0xff44] = 0;
        status &= 252;
        status = set_bit(status, 0);
        writeToMemory(gameBoy, 0xff41, status);
        return;
    }

    uint8_t currentLine = readFromMemory(gameBoy, 0xff44);
    uint8_t currentMode = (status & 0x3);

    uint8_t mode = 0;
    bool reqInt = false;

    if(currentLine >= 144) {
        mode = 1;
        status = set_bit(status, 0);
        status = reset_bit(status, 1);
        reqInt = bit_value(status, 4);
    } else {
        int mode2bounds = 456 - 80;
        int mode3bounds = mode2bounds - 172;
        if(gameBoy->scanlineCounter >= mode2bounds) {
            mode = 2;
            status = set_bit(status, 1);
            status = reset_bit(status, 0);
            reqInt = bit_value(status, 5);
        } else if(gameBoy->scanlineCounter >= mode3bounds) {
            mode = 3;
            status = set_bit(status, 1);
            status = set_bit(status, 0);
        } else {
            mode = 0;
            status = reset_bit(status, 1);
            status = reset_bit(status, 0);
            reqInt = bit_value(status, 3);
        }
    }
    if(reqInt && (currentMode != mode))
        requestInterrupt(gameBoy, 1);
    if(currentLine == readFromMemory(gameBoy, 0xff45)) {
        status = set_bit(status, 2);
        if(bit_value(status, 6))
            requestInterrupt(gameBoy, 1);
    } else
        status = reset_bit(status, 2);
    writeToMemory(gameBoy, 0xff41, status);
}

void updateGraphics(GameBoy* gameBoy, int cycles) {
    setLCDStatus(gameBoy);
    if(isLCDEnabled(gameBoy))
        gameBoy->scanlineCounter -= cycles;
    else
        return;

    if(gameBoy->scanlineCounter <= 0) {
        gameBoy->rom[0xff44]++;
        uint8_t currentLine = readFromMemory(gameBoy, 0xff44);
        gameBoy->scanlineCounter = SCANLINE_COUNTER_START;
        if(currentLine == VERTICAL_BLANK_SCAN_LINE)
            requestInterrupt(gameBoy, 0);
        else if(currentLine > VERTICAL_BLANK_SCAN_LINE_MAX)
            gameBoy->rom[0xff44] = 0;
        else if(currentLine < VERTICAL_BLANK_SCAN_LINE)
            drawScanline(gameBoy);
    }
}

void drawScanline(GameBoy* gameBoy) {
    uint8_t control = readFromMemory(gameBoy, 0xff40);
    if(bit_value(control, 0))
        renderTiles(gameBoy);
    if(bit_value(control, 1))
        renderSprites(gameBoy);
}

void renderSprites(GameBoy* gameBoy) {
    bool use8x16 = false;
    uint8_t lcdControl = readFromMemory(gameBoy, 0xff40);
    if(bit_value(lcdControl, 2))
        use8x16 = true;
    for(int sprite = 0; sprite < 40; sprite++) {
        uint8_t index = sprite * 4;
        uint8_t yPos = readFromMemory(gameBoy, 0xfe00 + index) - 16;
        uint8_t xPos = readFromMemory(gameBoy, 0xfe00 + index + 1) - 8;
        uint8_t tileLocation = readFromMemory(gameBoy, 0xfe00 + index + 2);
        uint8_t attributes = readFromMemory(gameBoy, 0xfe00 + index + 3);

        bool yFlip = bit_value(attributes, 6);
        bool xFlip = bit_value(attributes, 5);
        //bool priority = !bit_value(attributes, 7);
        int scanline = readFromMemory(gameBoy, 0xff44);

        int ySize = use8x16 ? 16 : 8;
        /*
        if(use8x16)
            ySize = 16;
        */
        if((scanline >= yPos) && (scanline < (yPos + ySize))) {
            int line = scanline - yPos;

            if(yFlip) {
                line -= ySize;
                line *= -1;
            }

            line *= 2;
            uint16_t dataAddress = (0x8000 + (tileLocation * 16)) + line;
            uint8_t data1 = readFromMemory(gameBoy, dataAddress);
            uint8_t data2 = readFromMemory(gameBoy, dataAddress + 1);

            for(int tilePixel = 7; tilePixel >= 0; tilePixel--) {
                int colorBit = tilePixel;
                if(xFlip) {
                    colorBit -= 7;
                    colorBit *= -1;
                }
                int colorNum = check_bit(data2, colorBit);
                colorNum <<= 1;
                colorNum |= check_bit(data1, colorBit);

                uint16_t colorAddress = bit_value(attributes, 4) ? 0xff49 : 0xff48;
                Color col = getColor(gameBoy, colorAddress, colorNum);

                if(col == WHITE)
                    continue;
                
                int red = 0;
                int green = 0;
                int blue = 0;

                switch(col) {
                    case WHITE: red = 255; green = 255; blue = 255; break;
                    case LIGHT_GRAY: red = 0xcc; green = 0xcc; blue = 0xcc; break;
                    case DARK_GRAY: red = 0x77; green = 0x77; blue = 0x77; break;
                }

                int xPix = 0 - tilePixel;
                xPix += 7;

                int pixel = xPos + xPix;
                if((scanline < 0) || (scanline > 143) || (pixel < 0) || (pixel > 159))
                    continue;

                /*
                if(!priority)
                    continue;
                */

                gameBoy->screenData[pixel][scanline][0] = red;
                gameBoy->screenData[pixel][scanline][1] = green;
                gameBoy->screenData[pixel][scanline][2] = blue;
            }
        }
    }
}

void renderTiles(GameBoy* gameBoy) {
    uint16_t tileData = 0;
    uint16_t backgroundMemory = 0;
    bool unsig = true;
    uint8_t lcdControl = readFromMemory(gameBoy, 0xff40);

    uint8_t scrollY = readFromMemory(gameBoy, 0xff42);
    uint8_t scrollX = readFromMemory(gameBoy, 0xff43);
    uint8_t windowY = readFromMemory(gameBoy, 0xff4a);
    uint8_t windowX = readFromMemory(gameBoy, 0xff4b) - 7;

    bool usingWindow = false;

    if(bit_value(lcdControl, 5))
        if(windowY <= readFromMemory(gameBoy, 0xff44))
            usingWindow = true;
    if(bit_value(lcdControl, 4))
        tileData = 0x8000;
    else {
        tileData = 0x8800;
        unsig = false;
    }

    if(!usingWindow) {
        if(bit_value(lcdControl, 3))
            backgroundMemory = 0x9c00;
        else
            backgroundMemory = 0x9800;
    } else {
        if(bit_value(lcdControl, 6))
            backgroundMemory = 0x9c00;
        else
            backgroundMemory = 0x9800;
    }

    uint8_t yPos = 0;

    if(!usingWindow)
        yPos = scrollY + readFromMemory(gameBoy, 0xff44);
    else
        yPos = readFromMemory(gameBoy, 0xff44) - windowY;

    uint16_t tileRow = (((uint8_t) (yPos / 8)) * 32);
    for(uint pixel = 0; pixel < WIDTH; pixel++) {
        uint8_t xPos = pixel + scrollX;
        if(usingWindow)
            if(pixel >= windowX)
                xPos = pixel - windowX;
        uint16_t tileCol = (xPos / 8);
        int16_t tileNum;
        uint16_t tileAddress = backgroundMemory + tileRow + tileCol;
        if(unsig)
            tileNum = readFromMemory(gameBoy, tileAddress);
        else
            tileNum = (int8_t) readFromMemory(gameBoy, tileAddress);
        uint16_t tileLocation = tileData;
        if(unsig)
            tileLocation += (tileNum * 16);
        else
            tileLocation += ((tileNum + 128) * 16); 
        uint8_t line = yPos % 8;
        line *= 2;
        uint8_t data1 = readFromMemory(gameBoy, tileLocation + line);
        uint8_t data2 = readFromMemory(gameBoy, tileLocation + line + 1);

        int colorBit = xPos % 8;
        colorBit -= 7;
        colorBit *= -1;

        int colorNum = check_bit(data2, colorBit);
        colorNum <<= 1; 
        colorNum |= check_bit(data1, colorBit);

        Color col = getColor(gameBoy, 0xff47, colorNum);
        int red = 0;
        int green = 0;
        int blue = 0;

        switch(col) {
            case WHITE: red = 255; green = 255; blue = 255; break;
            case LIGHT_GRAY: red = 0xcc; green = 0xcc; blue = 0xcc; break;
            case DARK_GRAY: red = 0x77; green = 0x77; blue = 0x77; break;
        }

        int finally = readFromMemory(gameBoy, 0xff44);
        if((finally < 0) || (finally > 143) || (pixel < 0) || (pixel > 159))
            continue;

        gameBoy->screenData[pixel][finally][0] = red;
        gameBoy->screenData[pixel][finally][1] = green;
        gameBoy->screenData[pixel][finally][2] = blue;
    }
}

Color getColor(GameBoy* gameBoy, uint16_t address, uint8_t colorNum) {
    Color res = WHITE;
    uint8_t palette = readFromMemory(gameBoy, address);
    int hi = 0;
    int lo = 0;

    switch(colorNum) {
        case 0: hi = 1; lo = 0; break;
        case 1: hi = 3; lo = 2; break;
        case 2: hi = 5; lo = 4; break;
        case 3: hi = 7; lo = 6; break;
    }

    int color = 0;
    color = check_bit(palette, hi) << 1;
    color |= check_bit(palette, lo);
    
    switch(color) {
        case 0: res = WHITE; break;
        case 1: res = LIGHT_GRAY; break;
        case 2: res = DARK_GRAY; break;
        case 3: res = BLACK; break;
    }

    return res;
}

void printCPU(GameBoy* gameBoy) {
    printf("Zero: %s\n", gameBoy->cpu.zero ? "true" : "false");
    printf("Subtract: %s\n", gameBoy->cpu.subtract ? "true" : "false");
    printf("Half Carry: %s\n", gameBoy->cpu.halfCarry ? "true" : "false");
    printf("Carry: %s\n", gameBoy->cpu.carry ? "true" : "false");
    printf("\n");
    printf("halted: %s\n", gameBoy->cpu.halted ? "true" : "false");
    printf("Interrupts Enabled: %s\n", gameBoy->cpu.interrupts ? "true" : "false");
    printf("Pending Interrupt Enable: %s\n", gameBoy->cpu.pendingInterruptEnable ? "true" : "false");
    printf("\n");
    printf("PC: %x\n", gameBoy->cpu.pc);
    printf("SP: %x\n", gameBoy->cpu.sp);
    printf("A: %x\n", gameBoy->cpu.a);
    printf("B: %x\n", gameBoy->cpu.b);
    printf("C: %x\n", gameBoy->cpu.c);
    printf("D: %x\n", gameBoy->cpu.d);
    printf("E: %x\n", gameBoy->cpu.e);
    printf("H: %x\n", gameBoy->cpu.h);
    printf("L: %x\n", gameBoy->cpu.l);
    printf("\n");
}

int main() {

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* screen = SDL_CreateWindow("AnotherGBEmu",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        0
    );

    if(!screen) {
        fprintf(stderr, "Could not create window\n");
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    if(!renderer) {
        fprintf(stderr, "Could not create renderer\n");
        return 1;
    }

    /*
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT
    );
    */

    GameBoy gameBoy;

    gameBoy.scanlineCounter = SCANLINE_COUNTER_START;
    gameBoy.timerCounter = 1024;
    //gameBoy.timerCounter = 0;
    gameBoy.dividerCounter = 0;
    gameBoy.romBanking = false;
    gameBoy.enableRAM = false;
    gameBoy.mBC1 = false;
    gameBoy.mBC2 = false;
    gameBoy.currentROMBank = 1;
    gameBoy.currentRAMBank = 0;
    
    memset(gameBoy.ramBanks, 0, sizeof(gameBoy.ramBanks));
    memset(gameBoy.cartridge, 0, sizeof(gameBoy.cartridge));
    memset(gameBoy.rom, 0, sizeof(gameBoy.rom));
    memset(gameBoy.screenData, 0, sizeof(gameBoy.screenData));
    
    CPU cpu;

    cpu.halted = false;
    cpu.interrupts = false;
    cpu.pendingInterruptEnable = false;
    cpu.pendingInterruptDisable = false;
    cpu.pc = 0x0100;
    cpu.sp = 0xfffe;
    cpu.a = 0x01;
    setFFlagsFromByte(&cpu, 0xb0);
    cpu.b = 0x00;
    cpu.c = 0x13;
    cpu.d = 0x00;
    cpu.e = 0xd8;
    cpu.h = 0x01;
    cpu.l = 0x4d;

    gameBoy.cpu = cpu;

    gameBoy.rom[0xff05] = 0x00;
    gameBoy.rom[0xff06] = 0x00;
    gameBoy.rom[0xff07] = 0x00;
    gameBoy.rom[0xff10] = 0x80;
    gameBoy.rom[0xff11] = 0xbf;
    gameBoy.rom[0xff12] = 0xf3;
    gameBoy.rom[0xff14] = 0xbf;
    gameBoy.rom[0xff16] = 0x3f;
    gameBoy.rom[0xff17] = 0x00;
    gameBoy.rom[0xff19] = 0xbf;
    gameBoy.rom[0xff1a] = 0x7f;
    gameBoy.rom[0xff1b] = 0xff;
    gameBoy.rom[0xff1c] = 0x9f;
    gameBoy.rom[0xff1e] = 0xbf;
    gameBoy.rom[0xff20] = 0xff;
    gameBoy.rom[0xff21] = 0x00;
    gameBoy.rom[0xff22] = 0x00;
    gameBoy.rom[0xff23] = 0xbf;
    gameBoy.rom[0xff24] = 0x77;
    gameBoy.rom[0xff25] = 0xf3;
    gameBoy.rom[0xff26] = 0xf1;
    gameBoy.rom[0xff40] = 0x91;
    gameBoy.rom[0xff42] = 0x00;
    gameBoy.rom[0xff43] = 0x00;
    gameBoy.rom[0xff45] = 0x00;
    gameBoy.rom[0xff47] = 0xfc;
    gameBoy.rom[0xff48] = 0xff;
    gameBoy.rom[0xff49] = 0xff;
    gameBoy.rom[0xff4a] = 0x00;
    gameBoy.rom[0xff4b] = 0x00;
    gameBoy.rom[0xffff] = 0x00;

    FILE* gameFile = fopen("tetris.gb", "rb");
    fread(gameBoy.cartridge, 0x2000000, 1, gameFile);
    fclose(gameFile);

    memcpy(gameBoy.rom, gameBoy.cartridge, 0x8000);

/*
    switch(gameBoy.cartridge[0x147]) {
        case 1: gameBoy.mBC1 = true; break;
        case 2: gameBoy.mBC1 = true; break;
        case 3: gameBoy.mBC1 = true; break;
        case 5: gameBoy.mBC2 = true; break;
        case 6: gameBoy.mBC2 = true; break;
        default: break;
    }
*/
    // 4.194304 MHz = 4194304 cycles per second
    // 59.727500569606 Hz = 59.727500569606 Frames per second
    // ~70224 cycles to 1 frame
    //uint32_t cyclesPerFrame = 4194304 / 59.727500569606;

    // START TESTING SECTION
    /*
    FILE* log = fopen("log.txt", "ab+");
    */
   /*
    printCPU(&gameBoy);
    
    printf("PRESS ENTER TO CONTINUE\n");
    char test[80];
    fgets(test, sizeof test, stdin);
    */
    /**/
    // END TESTING SECTION

    bool shouldClose = false;

    while(!shouldClose) {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0) {
            switch(e.type) {
                case  SDL_QUIT:
                    shouldClose = true;
                    break;
            }
            SDL_UpdateWindowSurface(screen);
        }

        clock_t startTime = clock();
        int cyclesThisFrame = 0;
        while(cyclesThisFrame <= CYCLES_PER_FRAME) {
            int cycles = 0;
            if(!gameBoy.cpu.halted)
                cycles = update(&gameBoy);
            // START TESTING SECTION
            printCPU(&gameBoy);
            /*
            printf("PRESS ENTER TO CONTINUE\n");
            char test[80];
            fgets(test, sizeof test, stdin);
            */
            // END TESTING SECTION
            cyclesThisFrame += cycles;
            updateTimer(&gameBoy, cycles);
            updateGraphics(&gameBoy, cycles);
            cyclesThisFrame += doInterrupts(&gameBoy);
        }

        /*
        for(int x = 0; x < WIDTH; x++)
            for(int y = 0; y < HEIGHT; y++)
                printf("%u - %u - %u\n", gameBoy.screenData[x][y][0], gameBoy.screenData[x][y][1], gameBoy.screenData[x][y][2]);
        */

        for(int x = 0; x < WIDTH; x++)
            for(int y = 0; y < HEIGHT; y++) {
                SDL_SetRenderDrawColor(renderer, gameBoy.screenData[x][y][0], gameBoy.screenData[x][y][1], gameBoy.screenData[x][y][2], 255);
                SDL_RenderDrawPoint(renderer, x, y);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            }
        /*
        SDL_UpdateTexture(texture, NULL, gameBoy.screenData, WIDTH * sizeof(uint8_t) * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        */
        SDL_RenderPresent(renderer);
        double elapsedTime = clock() - startTime;
        if(elapsedTime <= TIME_BETWEEN_FRAMES) {
            struct timespec remainingTime = { 0 , TIME_BETWEEN_FRAMES - elapsedTime };
            nanosleep(&remainingTime, NULL);
        }
    }

    //SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 0;
}