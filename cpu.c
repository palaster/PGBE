#include "cpu.h"
#include <stdlib.h>
#include "bit_logic.h"
#include "gameboy.h"

bool cpuDebug() { return false; }

uint8_t fetch(GameBoy* gameBoy) { return readFromMemory(gameBoy, gameBoy->cpu.pc++); }

void pop(GameBoy* gameBoy, uint8_t* lower, uint8_t* upper) {
    *lower = readFromMemory(gameBoy, gameBoy->cpu.sp++);
    *upper = readFromMemory(gameBoy, gameBoy->cpu.sp++);
}

void push(GameBoy* gameBoy, const uint8_t lower, const uint8_t upper) {
    writeToMemory(gameBoy, --gameBoy->cpu.sp, upper);
    writeToMemory(gameBoy, --gameBoy->cpu.sp, lower);
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

void bit(GameBoy* gameBoy, const uint8_t bit, const uint8_t reg) {
    gameBoy->cpu.zero = !check_bit(reg, bit);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = true;
}

void set(GameBoy* gameBoy, const uint8_t bit, uint8_t* reg) {
    *reg = set_bit(*reg, bit);
}

void res(GameBoy* gameBoy, const uint8_t bit, uint8_t* reg) {
    *reg = reset_bit(*reg, bit);
}

int decodeAndExecuteCB(GameBoy* gameBoy, const uint8_t instruction) {
    switch(instruction) {
        case 0x00: {
            // RLC B
            if(cpuDebug()) printf("RLC B\n");
            rlc(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x01: {
            // RLC C
            if(cpuDebug()) printf("RLC C\n");
            rlc(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x02: {
            // RLC D
            if(cpuDebug()) printf("RLC D\n");
            rlc(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x03: {
            // RLC E
            if(cpuDebug()) printf("RLC E\n");
            rlc(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x04: {
            // RLC H
            if(cpuDebug()) printf("RLC H\n");
            rlc(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x05: {
            // RLC L
            if(cpuDebug()) printf("RLC L\n");
            rlc(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x06: {
            // RLC (HL)
            if(cpuDebug()) printf("RLC (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rlc(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x07: {
            // RLC A
            if(cpuDebug()) printf("RLC A\n");
            rlc(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x08: {
            // RRC B
            if(cpuDebug()) printf("RRC B\n");
            rrc(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x09: {
            // RRC C
            if(cpuDebug()) printf("RRC C\n");
            rrc(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x0a: {
            // RRC D
            if(cpuDebug()) printf("RRC D\n");
            rrc(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x0b: {
            // RRC E
            if(cpuDebug()) printf("RRC E\n");
            rrc(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x0c: {
            // RRC H
            if(cpuDebug()) printf("RRC H\n");
            rrc(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x0d: {
            // RRC L
            if(cpuDebug()) printf("RRC L\n");
            rrc(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x0e: {
            // RRC (HL)
            if(cpuDebug()) printf("RRC (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rrc(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x0f: {
            // RRC A
            if(cpuDebug()) printf("RRC A\n");
            rrc(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x10: {
            // RL B
            if(cpuDebug()) printf("RL B\n");
            rl(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x11: {
            // RL C
            if(cpuDebug()) printf("RL C\n");
            rl(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x12: {
            // RL D
            if(cpuDebug()) printf("RL D\n");
            rl(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x13: {
            // RL E
            if(cpuDebug()) printf("RL E\n");
            rl(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x14: {
            // RL H
            if(cpuDebug()) printf("RL H\n");
            rl(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x15: {
            // RL L
            if(cpuDebug()) printf("RL L\n");
            rl(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x16: {
            // RL (HL)
            if(cpuDebug()) printf("RL (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rl(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x17: {
            // RL A
            if(cpuDebug()) printf("RL A\n");
            rl(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x18: {
            // RR B
            if(cpuDebug()) printf("RR B\n");
            rr(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x19: {
            // RR C
            if(cpuDebug()) printf("RR C\n");
            rr(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x1a: {
            // RR D
            if(cpuDebug()) printf("RR D\n");
            rr(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x1b: {
            // RR E
            if(cpuDebug()) printf("RR E\n");
            rr(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x1c: {
            // RR H
            if(cpuDebug()) printf("RR H\n");
            rr(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x1d: {
            // RR L
            if(cpuDebug()) printf("RR L\n");
            rr(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x1e: {
            // RR (HL)
            if(cpuDebug()) printf("RR (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            rr(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x1f: {
            // RR A
            if(cpuDebug()) printf("RR A\n");
            rr(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x20: {
            // SLA B
            if(cpuDebug()) printf("SLA B\n");
            sla(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x21: {
            // SLA C
            if(cpuDebug()) printf("SLA C\n");
            sla(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x22: {
            // SLA D
            if(cpuDebug()) printf("SLA D\n");
            sla(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x23: {
            // SLA E
            if(cpuDebug()) printf("SLA E\n");
            sla(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x24: {
            // SLA H
            if(cpuDebug()) printf("SLA H\n");
            sla(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x25: {
            // SLA L
            if(cpuDebug()) printf("SLA L\n");
            sla(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x26: {
            // SLA (HL)
            if(cpuDebug()) printf("SLA (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            sla(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x27: {
            // SLA A
            if(cpuDebug()) printf("SLA A\n");
            sla(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x28: {
            // SRA B
            if(cpuDebug()) printf("SRA B\n");
            sra(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x29: {
            // SRA C
            if(cpuDebug()) printf("SRA C\n");
            sra(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x2a: {
            // SRA D
            if(cpuDebug()) printf("SRA D\n");
            sra(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x2b: {
            // SRA E
            if(cpuDebug()) printf("SRA E\n");
            sra(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x2c: {
            // SRA H
            if(cpuDebug()) printf("SRA H\n");
            sra(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x2d: {
            // SRA L
            if(cpuDebug()) printf("SRA L\n");
            sra(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x2e: {
            // SRA (HL)
            if(cpuDebug()) printf("SRA (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            sra(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x2f: {
            // SRA A
            if(cpuDebug()) printf("SRA A\n");
            sra(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x30: {
            // SWAP B
            if(cpuDebug()) printf("SWAP B\n");
            swap(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x31: {
            // SWAP C
            if(cpuDebug()) printf("SWAP C\n");
            swap(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x32: {
            // SWAP D
            if(cpuDebug()) printf("SWAP D\n");
            swap(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x33: {
            // SWAP E
            if(cpuDebug()) printf("SWAP E\n");
            swap(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x34: {
            // SWAP H
            if(cpuDebug()) printf("SWAP H\n");
            swap(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x35: {
            // SWAP L
            if(cpuDebug()) printf("SWAP L\n");
            swap(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x36: {
            // SWAP (HL)
            if(cpuDebug()) printf("SWAP (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            swap(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x37: {
            // SWAP A
            if(cpuDebug()) printf("SWAP A\n");
            swap(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x38: {
            // SRL B
            if(cpuDebug()) printf("SRL B\n");
            srl(gameBoy, &gameBoy->cpu.b);
            return 8;
        }
        case 0x39: {
            // SRL C
            if(cpuDebug()) printf("SRL C\n");
            srl(gameBoy, &gameBoy->cpu.c);
            return 8;
        }
        case 0x3a: {
            // SRL D
            if(cpuDebug()) printf("SRL D\n");
            srl(gameBoy, &gameBoy->cpu.d);
            return 8;
        }
        case 0x3b: {
            // SRL E
            if(cpuDebug()) printf("SRL E\n");
            srl(gameBoy, &gameBoy->cpu.e);
            return 8;
        }
        case 0x3c: {
            // SRL H
            if(cpuDebug()) printf("SRL H\n");
            srl(gameBoy, &gameBoy->cpu.h);
            return 8;
        }
        case 0x3d: {
            // SRL L
            if(cpuDebug()) printf("SRL L\n");
            srl(gameBoy, &gameBoy->cpu.l);
            return 8;
        }
        case 0x3e: {
            // SRL (HL)
            if(cpuDebug()) printf("SRL (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            srl(gameBoy, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x3f: {
            // SRL A
            if(cpuDebug()) printf("SRL A\n");
            srl(gameBoy, &gameBoy->cpu.a);
            return 8;
        }
        case 0x40: {
            // BIT 0 B
            if(cpuDebug()) printf("BIT 0 B\n");
            bit(gameBoy, 0, gameBoy->cpu.b);
            return 8;
        }
        case 0x41: {
            // BIT 0 C
            if(cpuDebug()) printf("BIT 0 C\n");
            bit(gameBoy, 0, gameBoy->cpu.c);
            return 8;
        }
        case 0x42: {
            // BIT 0 D
            if(cpuDebug()) printf("BIT 0 D\n");
            bit(gameBoy, 0, gameBoy->cpu.d);
            return 8;
        }
        case 0x43: {
            // BIT 0 E
            if(cpuDebug()) printf("BIT 0 E\n");
            bit(gameBoy, 0, gameBoy->cpu.e);
            return 8;
        }
        case 0x44: {
            // BIT 0 H
            if(cpuDebug()) printf("BIT 0 H\n");
            bit(gameBoy, 0, gameBoy->cpu.h);
            return 8;
        }
        case 0x45: {
            // BIT 0 L
            if(cpuDebug()) printf("BIT 0 L\n");
            bit(gameBoy, 0, gameBoy->cpu.l);
            return 8;
        }
        case 0x46: {
            // BIT 0 (HL)
            if(cpuDebug()) printf("BIT 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 0, value);
            return 12;
        }
        case 0x47: {
            // BIT 0 A
            if(cpuDebug()) printf("BIT 0 A\n");
            bit(gameBoy, 0, gameBoy->cpu.a);
            return 8;
        }
        case 0x48: {
            // BIT 1 B
            if(cpuDebug()) printf("BIT 1 B\n");
            bit(gameBoy, 1, gameBoy->cpu.b);
            return 8;
        }
        case 0x49: {
            // BIT 1 C
            if(cpuDebug()) printf("BIT 1 C\n");
            bit(gameBoy, 1, gameBoy->cpu.c);
            return 8;
        }
        case 0x4a: {
            // BIT 1 D
            if(cpuDebug()) printf("BIT 1 D\n");
            bit(gameBoy, 1, gameBoy->cpu.d);
            return 8;
        }
        case 0x4b: {
            // BIT 1 E
            if(cpuDebug()) printf("BIT 1 E\n");
            bit(gameBoy, 1, gameBoy->cpu.e);
            return 8;
        }
        case 0x4c: {
            // BIT 1 H
            if(cpuDebug()) printf("BIT 1 H\n");
            bit(gameBoy, 1, gameBoy->cpu.h);
            return 8;
        }
        case 0x4d: {
            // BIT 1 L
            if(cpuDebug()) printf("BIT 1 L\n");
            bit(gameBoy, 1, gameBoy->cpu.l);
            return 8;
        }
        case 0x4e: {
            // BIT 1 (HL)
            if(cpuDebug()) printf("BIT 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 1, value);
            return 12;
        }
        case 0x4f: {
            // BIT 1 A
            if(cpuDebug()) printf("BIT 1 A\n");
            bit(gameBoy, 1, gameBoy->cpu.a);
            return 8;
        }
        case 0x50: {
            // BIT 2 B
            if(cpuDebug()) printf("BIT 2 B\n");
            bit(gameBoy, 2, gameBoy->cpu.b);
            return 8;
        }
        case 0x51: {
            // BIT 2 C
            if(cpuDebug()) printf("BIT 2 C\n");
            bit(gameBoy, 2, gameBoy->cpu.c);
            return 8;
        }
        case 0x52: {
            // BIT 2 D
            if(cpuDebug()) printf("BIT 2 D\n");
            bit(gameBoy, 2, gameBoy->cpu.d);
            return 8;
        }
        case 0x53: {
            // BIT 2 E
            if(cpuDebug()) printf("BIT 2 E\n");
            bit(gameBoy, 2, gameBoy->cpu.e);
            return 8;
        }
        case 0x54: {
            // BIT 2 H
            if(cpuDebug()) printf("BIT 2 H\n");
            bit(gameBoy, 2, gameBoy->cpu.h);
            return 8;
        }
        case 0x55: {
            // BIT 2 L
            if(cpuDebug()) printf("BIT 2 L\n");
            bit(gameBoy, 2, gameBoy->cpu.l);
            return 8;
        }
        case 0x56: {
            // BIT 2 (HL)
            if(cpuDebug()) printf("BIT 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 2, value);
            return 12;
        }
        case 0x57: {
            // BIT 2 A
            if(cpuDebug()) printf("BIT 2 A\n");
            bit(gameBoy, 2, gameBoy->cpu.a);
            return 8;
        }
        case 0x58: {
            // BIT 3 B
            if(cpuDebug()) printf("BIT 3 B\n");
            bit(gameBoy, 3, gameBoy->cpu.b);
            return 8;
        }
        case 0x59: {
            // BIT 3 C
            if(cpuDebug()) printf("BIT 3 C\n");
            bit(gameBoy, 3, gameBoy->cpu.c);
            return 8;
        }
        case 0x5a: {
            // BIT 3 D
            if(cpuDebug()) printf("BIT 3 D\n");
            bit(gameBoy, 3, gameBoy->cpu.d);
            return 8;
        }
        case 0x5b: {
            // BIT 3 E
            if(cpuDebug()) printf("BIT 3 E\n");
            bit(gameBoy, 3, gameBoy->cpu.e);
            return 8;
        }
        case 0x5c: {
            // BIT 3 H
            if(cpuDebug()) printf("BIT 3 H\n");
            bit(gameBoy, 3, gameBoy->cpu.h);
            return 8;
        }
        case 0x5d: {
            // BIT 3 L
            if(cpuDebug()) printf("BIT 3 L\n");
            bit(gameBoy, 3, gameBoy->cpu.l);
            return 8;
        }
        case 0x5e: {
            // BIT 3 (HL)
            if(cpuDebug()) printf("BIT 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 3, value);
            return 12;
        }
        case 0x5f: {
            // BIT 3 A
            if(cpuDebug()) printf("BIT 3 A\n");
            bit(gameBoy, 3, gameBoy->cpu.a);
            return 8;
        }
        case 0x60: {
            // BIT 4 B
            if(cpuDebug()) printf("BIT 4 B\n");
            bit(gameBoy, 4, gameBoy->cpu.b);
            return 8;
        }
        case 0x61: {
            // BIT 4 C
            if(cpuDebug()) printf("BIT 4 C\n");
            bit(gameBoy, 4, gameBoy->cpu.c);
            return 8;
        }
        case 0x62: {
            // BIT 4 D
            if(cpuDebug()) printf("BIT 4 D\n");
            bit(gameBoy, 4, gameBoy->cpu.d);
            return 8;
        }
        case 0x63: {
            // BIT 4 E
            if(cpuDebug()) printf("BIT 4 E\n");
            bit(gameBoy, 4, gameBoy->cpu.e);
            return 8;
        }
        case 0x64: {
            // BIT 4 H
            if(cpuDebug()) printf("BIT 4 H\n");
            bit(gameBoy, 4, gameBoy->cpu.h);
            return 8;
        }
        case 0x65: {
            // BIT 4 L
            if(cpuDebug()) printf("BIT 4 L\n");
            bit(gameBoy, 4, gameBoy->cpu.l);
            return 8;
        }
        case 0x66: {
            // BIT 4 (HL)
            if(cpuDebug()) printf("BIT 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 4, value);
            return 12;
        }
        case 0x67: {
            // BIT 4 A
            if(cpuDebug()) printf("BIT 4 A\n");
            bit(gameBoy, 4, gameBoy->cpu.a);
            return 8;
        }
        case 0x68: {
            // BIT 5 B
            if(cpuDebug()) printf("BIT 5 B\n");
            bit(gameBoy, 5, gameBoy->cpu.b);
            return 8;
        }
        case 0x69: {
            // BIT 5 C
            if(cpuDebug()) printf("BIT 5 C\n");
            bit(gameBoy, 5, gameBoy->cpu.c);
            return 8;
        }
        case 0x6a: {
            // BIT 5 D
            if(cpuDebug()) printf("BIT 5 D\n");
            bit(gameBoy, 5, gameBoy->cpu.d);
            return 8;
        }
        case 0x6b: {
            // BIT 5 E
            if(cpuDebug()) printf("BIT 5 E\n");
            bit(gameBoy, 5, gameBoy->cpu.e);
            return 8;
        }
        case 0x6c: {
            // BIT 5 H
            if(cpuDebug()) printf("BIT 5 H\n");
            bit(gameBoy, 5, gameBoy->cpu.h);
            return 8;
        }
        case 0x6d: {
            // BIT 5 L
            if(cpuDebug()) printf("BIT 5 L\n");
            bit(gameBoy, 5, gameBoy->cpu.l);
            return 8;
        }
        case 0x6e: {
            // BIT 5 (HL)
            if(cpuDebug()) printf("BIT 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 5, value);
            return 12;
        }
        case 0x6f: {
            // BIT 5 A
            if(cpuDebug()) printf("BIT 5 A\n");
            bit(gameBoy, 5, gameBoy->cpu.a);
            return 8;
        }
        case 0x70: {
            // BIT 6 B
            if(cpuDebug()) printf("BIT 6 B\n");
            bit(gameBoy, 6, gameBoy->cpu.b);
            return 8;
        }
        case 0x71: {
            // BIT 6 C
            if(cpuDebug()) printf("BIT 6 C\n");
            bit(gameBoy, 6, gameBoy->cpu.c);
            return 8;
        }
        case 0x72: {
            // BIT 6 D
            if(cpuDebug()) printf("BIT 6 D\n");
            bit(gameBoy, 6, gameBoy->cpu.d);
            return 8;
        }
        case 0x73: {
            // BIT 6 E
            if(cpuDebug()) printf("BIT 6 E\n");
            bit(gameBoy, 6, gameBoy->cpu.e);
            return 8;
        }
        case 0x74: {
            // BIT 6 H
            if(cpuDebug()) printf("BIT 6 H\n");
            bit(gameBoy, 6, gameBoy->cpu.h);
            return 8;
        }
        case 0x75: {
            // BIT 6 L
            if(cpuDebug()) printf("BIT 6 L\n");
            bit(gameBoy, 6, gameBoy->cpu.l);
            return 8;
        }
        case 0x76: {
            // BIT 6 (HL)
            if(cpuDebug()) printf("BIT 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 6, value);
            return 12;
        }
        case 0x77: {
            // BIT 6 A
            if(cpuDebug()) printf("BIT 6 A\n");
            bit(gameBoy, 6, gameBoy->cpu.a);
            return 8;
        }
        case 0x78: {
            // BIT 7 B
            if(cpuDebug()) printf("BIT 7 B\n");
            bit(gameBoy, 7, gameBoy->cpu.b);
            return 8;
        }
        case 0x79: {
            // BIT 7 C
            if(cpuDebug()) printf("BIT 7 C\n");
            bit(gameBoy, 7, gameBoy->cpu.c);
            return 8;
        }
        case 0x7a: {
            // BIT 7 D
            if(cpuDebug()) printf("BIT 7 D\n");
            bit(gameBoy, 7, gameBoy->cpu.d);
            return 8;
        }
        case 0x7b: {
            // BIT 7 E
            if(cpuDebug()) printf("BIT 7 E\n");
            bit(gameBoy, 7, gameBoy->cpu.e);
            return 8;
        }
        case 0x7c: {
            // BIT 7 H
            if(cpuDebug()) printf("BIT 7 H\n");
            bit(gameBoy, 7, gameBoy->cpu.h);
            return 8;
        }
        case 0x7d: {
            // BIT 7 L
            if(cpuDebug()) printf("BIT 7 L\n");
            bit(gameBoy, 7, gameBoy->cpu.l);
            return 8;
        }
        case 0x7e: {
            // BIT 7 (HL)
            if(cpuDebug()) printf("BIT 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            bit(gameBoy, 7, value);
            return 12;
        }
        case 0x7f: {
            // BIT 7 A
            if(cpuDebug()) printf("BIT 7 A\n");
            bit(gameBoy, 7, gameBoy->cpu.a);
            return 8;
        }
        case 0x80: {
            // RES 0 B
            if(cpuDebug()) printf("RES 0 B\n");
            res(gameBoy, 0, &gameBoy->cpu.b);
            return 8;
        }
        case 0x81: {
            // RES 0 C
            if(cpuDebug()) printf("RES 0 C\n");
            res(gameBoy, 0, &gameBoy->cpu.c);
            return 8;
        }
        case 0x82: {
            // RES 0 D
            if(cpuDebug()) printf("RES 0 D\n");
            res(gameBoy, 0, &gameBoy->cpu.d);
            return 8;
        }
        case 0x83: {
            // RES 0 E
            if(cpuDebug()) printf("RES 0 E\n");
            res(gameBoy, 0, &gameBoy->cpu.e);
            return 8;
        }
        case 0x84: {
            // RES 0 H
            if(cpuDebug()) printf("RES 0 H\n");
            res(gameBoy, 0, &gameBoy->cpu.h);
            return 8;
        }
        case 0x85: {
            // RES 0 L
            if(cpuDebug()) printf("RES 0 L\n");
            res(gameBoy, 0, &gameBoy->cpu.l);
            return 8;
        }
        case 0x86: {
            // RES 0 (HL)
            if(cpuDebug()) printf("RES 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 0, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x87: {
            // RES 0 A
            if(cpuDebug()) printf("RES 0 A\n");
            res(gameBoy, 0, &gameBoy->cpu.a);
            return 8;
        }
        case 0x88: {
            // RES 1 B
            if(cpuDebug()) printf("RES 1 B\n");
            res(gameBoy, 1, &gameBoy->cpu.b);
            return 8;
        }
        case 0x89: {
            // RES 1 C
            if(cpuDebug()) printf("RES 1 C\n");
            res(gameBoy, 1, &gameBoy->cpu.c);
            return 8;
        }
        case 0x8a: {
            // RES 1 D
            if(cpuDebug()) printf("RES 1 D\n");
            res(gameBoy, 1, &gameBoy->cpu.d);
            return 8;
        }
        case 0x8b: {
            // RES 1 E
            if(cpuDebug()) printf("RES 1 E\n");
            res(gameBoy, 1, &gameBoy->cpu.e);
            return 8;
        }
        case 0x8c: {
            // RES 1 H
            if(cpuDebug()) printf("RES 1 H\n");
            res(gameBoy, 1, &gameBoy->cpu.h);
            return 8;
        }
        case 0x8d: {
            // RES 1 L
            if(cpuDebug()) printf("RES 1 L\n");
            res(gameBoy, 1, &gameBoy->cpu.l);
            return 8;
        }
        case 0x8e: {
            // RES 1 (HL)
            if(cpuDebug()) printf("RES 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 1, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x8f: {
            // RES 1 A
            if(cpuDebug()) printf("RES 1 A\n");
            res(gameBoy, 1, &gameBoy->cpu.a);
            return 8;
        }
        case 0x90: {
            // RES 2 B
            if(cpuDebug()) printf("RES 2 B\n");
            res(gameBoy, 2, &gameBoy->cpu.b);
            return 8;
        }
        case 0x91: {
            // RES 2 C
            if(cpuDebug()) printf("RES 2 C\n");
            res(gameBoy, 2, &gameBoy->cpu.c);
            return 8;
        }
        case 0x92: {
            // RES 2 D
            if(cpuDebug()) printf("RES 2 D\n");
            res(gameBoy, 2, &gameBoy->cpu.d);
            return 8;
        }
        case 0x93: {
            // RES 2 E
            if(cpuDebug()) printf("RES 2 E\n");
            res(gameBoy, 2, &gameBoy->cpu.e);
            return 8;
        }
        case 0x94: {
            // RES 2 H
            if(cpuDebug()) printf("RES 2 H\n");
            res(gameBoy, 2, &gameBoy->cpu.h);
            return 8;
        }
        case 0x95: {
            // RES 2 L
            if(cpuDebug()) printf("RES 2 L\n");
            res(gameBoy, 2, &gameBoy->cpu.l);
            return 8;
        }
        case 0x96: {
            // RES 2 (HL)
            if(cpuDebug()) printf("RES 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 2, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x97: {
            // RES 2 A
            if(cpuDebug()) printf("RES 2 A\n");
            res(gameBoy, 2, &gameBoy->cpu.a);
            return 8;
        }
        case 0x98: {
            // RES 3 B
            if(cpuDebug()) printf("RES 3 B\n");
            res(gameBoy, 3, &gameBoy->cpu.b);
            return 8;
        }
        case 0x99: {
            // RES 3 C
            if(cpuDebug()) printf("RES 3 C\n");
            res(gameBoy, 3, &gameBoy->cpu.c);
            return 8;
        }
        case 0x9a: {
            // RES 3 D
            if(cpuDebug()) printf("RES 3 D\n");
            res(gameBoy, 3, &gameBoy->cpu.d);
            return 8;
        }
        case 0x9b: {
            // RES 3 E
            if(cpuDebug()) printf("RES 3 E\n");
            res(gameBoy, 3, &gameBoy->cpu.e);
            return 8;
        }
        case 0x9c: {
            // RES 3 H
            if(cpuDebug()) printf("RES 3 H\n");
            res(gameBoy, 3, &gameBoy->cpu.h);
            return 8;
        }
        case 0x9d: {
            // RES 3 L
            if(cpuDebug()) printf("RES 3 L\n");
            res(gameBoy, 3, &gameBoy->cpu.l);
            return 8;
        }
        case 0x9e: {
            // RES 3 (HL)
            if(cpuDebug()) printf("RES 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 3, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0x9f: {
            // RES 3 A
            if(cpuDebug()) printf("RES 3 A\n");
            res(gameBoy, 3, &gameBoy->cpu.a);
            return 8;
        }
        case 0xa0: {
            // RES 4 B
            if(cpuDebug()) printf("RES 4 B\n");
            res(gameBoy, 4, &gameBoy->cpu.b);
            return 8;
        }
        case 0xa1: {
            // RES 4 C
            if(cpuDebug()) printf("RES 4 C\n");
            res(gameBoy, 4, &gameBoy->cpu.c);
            return 8;
        }
        case 0xa2: {
            // RES 4 D
            if(cpuDebug()) printf("RES 4 D\n");
            res(gameBoy, 4, &gameBoy->cpu.d);
            return 8;
        }
        case 0xa3: {
            // RES 4 E
            if(cpuDebug()) printf("RES 4 E\n");
            res(gameBoy, 4, &gameBoy->cpu.e);
            return 8;
        }
        case 0xa4: {
            // RES 4 H
            if(cpuDebug()) printf("RES 4 H\n");
            res(gameBoy, 4, &gameBoy->cpu.h);
            return 8;
        }
        case 0xa5: {
            // RES 4 L
            if(cpuDebug()) printf("RES 4 L\n");
            res(gameBoy, 4, &gameBoy->cpu.l);
            return 8;
        }
        case 0xa6: {
            // RES 4 (HL)
            if(cpuDebug()) printf("RES 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 4, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xa7: {
            // RES 4 A
            if(cpuDebug()) printf("RES 4 A\n");
            res(gameBoy, 4, &gameBoy->cpu.a);
            return 8;
        }
        case 0xa8: {
            // RES 5 B
            if(cpuDebug()) printf("RES 5 B\n");
            res(gameBoy, 5, &gameBoy->cpu.b);
            return 8;
        }
        case 0xa9: {
            // RES 5 C
            if(cpuDebug()) printf("RES 5 C\n");
            res(gameBoy, 5, &gameBoy->cpu.c);
            return 8;
        }
        case 0xaa: {
            // RES 5 D
            if(cpuDebug()) printf("RES 5 D\n");
            res(gameBoy, 5, &gameBoy->cpu.d);
            return 8;
        }
        case 0xab: {
            // RES 5 E
            if(cpuDebug()) printf("RES 5 E\n");
            res(gameBoy, 5, &gameBoy->cpu.e);
            return 8;
        }
        case 0xac: {
            // RES 5 H
            if(cpuDebug()) printf("RES 5 H\n");
            res(gameBoy, 5, &gameBoy->cpu.h);
            return 8;
        }
        case 0xad: {
            // RES 5 L
            if(cpuDebug()) printf("RES 5 L\n");
            res(gameBoy, 5, &gameBoy->cpu.l);
            return 8;
        }
        case 0xae: {
            // RES 5 (HL)
            if(cpuDebug()) printf("RES 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 5, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xaf: {
            // RES 5 A
            if(cpuDebug()) printf("RES 5 A\n");
            res(gameBoy, 5, &gameBoy->cpu.a);
            return 8;
        }
        case 0xb0: {
            // RES 6 B
            if(cpuDebug()) printf("RES 6 B\n");
            res(gameBoy, 6, &gameBoy->cpu.b);
            return 8;
        }
        case 0xb1: {
            // RES 6 C
            if(cpuDebug()) printf("RES 6 C\n");
            res(gameBoy, 6, &gameBoy->cpu.c);
            return 8;
        }
        case 0xb2: {
            // RES 6 D
            if(cpuDebug()) printf("RES 6 D\n");
            res(gameBoy, 6, &gameBoy->cpu.d);
            return 8;
        }
        case 0xb3: {
            // RES 6 E
            if(cpuDebug()) printf("RES 6 E\n");
            res(gameBoy, 6, &gameBoy->cpu.e);
            return 8;
        }
        case 0xb4: {
            // RES 6 H
            if(cpuDebug()) printf("RES 6 H\n");
            res(gameBoy, 6, &gameBoy->cpu.h);
            return 8;
        }
        case 0xb5: {
            // RES 6 L
            if(cpuDebug()) printf("RES 6 L\n");
            res(gameBoy, 6, &gameBoy->cpu.l);
            return 8;
        }
        case 0xb6: {
            // RES 6 (HL)
            if(cpuDebug()) printf("RES 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 6, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xb7: {
            // RES 6 A
            if(cpuDebug()) printf("RES 6 A\n");
            res(gameBoy, 6, &gameBoy->cpu.a);
            return 8;
        }
        case 0xb8: {
            // RES 7 B
            if(cpuDebug()) printf("RES 7 B\n");
            res(gameBoy, 7, &gameBoy->cpu.b);
            return 8;
        }
        case 0xb9: {
            // RES 7 C
            if(cpuDebug()) printf("RES 7 C\n");
            res(gameBoy, 7, &gameBoy->cpu.c);
            return 8;
        }
        case 0xba: {
            // RES 7 D
            if(cpuDebug()) printf("RES 7 D\n");
            res(gameBoy, 7, &gameBoy->cpu.d);
            return 8;
        }
        case 0xbb: {
            // RES 7 E
            if(cpuDebug()) printf("RES 7 E\n");
            res(gameBoy, 7, &gameBoy->cpu.e);
            return 8;
        }
        case 0xbc: {
            // RES 7 H
            if(cpuDebug()) printf("RES 7 H\n");
            res(gameBoy, 7, &gameBoy->cpu.h);
            return 8;
        }
        case 0xbd: {
            // RES 7 L
            if(cpuDebug()) printf("RES 7 L\n");
            res(gameBoy, 7, &gameBoy->cpu.l);
            return 8;
        }
        case 0xbe: {
            // RES 7 (HL)
            if(cpuDebug()) printf("RES 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            res(gameBoy, 7, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xbf: {
            // RES 7 A
            if(cpuDebug()) printf("RES 7 A\n");
            res(gameBoy, 7, &gameBoy->cpu.a);
            return 8;
        }
        case 0xc0: {
            // SET 0 B
            if(cpuDebug()) printf("SET 0 B\n");
            set(gameBoy, 0, &gameBoy->cpu.b);
            return 8;
        }
        case 0xc1: {
            // SET 0 C
            if(cpuDebug()) printf("SET 0 C\n");
            set(gameBoy, 0, &gameBoy->cpu.c);
            return 8;
        }
        case 0xc2: {
            // SET 0 D
            if(cpuDebug()) printf("SET 0 D\n");
            set(gameBoy, 0, &gameBoy->cpu.d);
            return 8;
        }
        case 0xc3: {
            // SET 0 E
            if(cpuDebug()) printf("SET 0 E\n");
            set(gameBoy, 0, &gameBoy->cpu.e);
            return 8;
        }
        case 0xc4: {
            // SET 0 H
            if(cpuDebug()) printf("SET 0 H\n");
            set(gameBoy, 0, &gameBoy->cpu.h);
            return 8;
        }
        case 0xc5: {
            // SET 0 L
            if(cpuDebug()) printf("SET 0 L\n");
            set(gameBoy, 0, &gameBoy->cpu.l);
            return 8;
        }
        case 0xc6: {
            // SET 0 (HL)
            if(cpuDebug()) printf("SET 0 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 0, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xc7: {
            // SET 0 A
            if(cpuDebug()) printf("SET 0 A\n");
            set(gameBoy, 0, &gameBoy->cpu.a);
            return 8;
        }
        case 0xc8: {
            // SET 1 B
            if(cpuDebug()) printf("SET 1 B\n");
            set(gameBoy, 1, &gameBoy->cpu.b);
            return 8;
        }
        case 0xc9: {
            // SET 1 C
            if(cpuDebug()) printf("SET 1 C\n");
            set(gameBoy, 1, &gameBoy->cpu.c);
            return 8;
        }
        case 0xca: {
            // SET 1 D
            if(cpuDebug()) printf("SET 1 D\n");
            set(gameBoy, 1, &gameBoy->cpu.d);
            return 8;
        }
        case 0xcb: {
            // SET 1 E
            if(cpuDebug()) printf("SET 1 E\n");
            set(gameBoy, 1, &gameBoy->cpu.e);
            return 8;
        }
        case 0xcc: {
            // SET 1 H
            if(cpuDebug()) printf("SET 1 H\n");
            set(gameBoy, 1, &gameBoy->cpu.h);
            return 8;
        }
        case 0xcd: {
            // SET 1 L
            if(cpuDebug()) printf("SET 1 L\n");
            set(gameBoy, 1, &gameBoy->cpu.l);
            return 8;
        }
        case 0xce: {
            // SET 1 (HL)
            if(cpuDebug()) printf("SET 1 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 1, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xcf: {
            // SET 1 A
            if(cpuDebug()) printf("SET 1 A\n");
            set(gameBoy, 1, &gameBoy->cpu.a);
            return 8;
        }
        case 0xd0: {
            // SET 2 B
            if(cpuDebug()) printf("SET 2 B\n");
            set(gameBoy, 2, &gameBoy->cpu.b);
            return 8;
        }
        case 0xd1: {
            // SET 2 C
            if(cpuDebug()) printf("SET 2 C\n");
            set(gameBoy, 2, &gameBoy->cpu.c);
            return 8;
        }
        case 0xd2: {
            // SET 2 D
            if(cpuDebug()) printf("SET 2 D\n");
            set(gameBoy, 2, &gameBoy->cpu.d);
            return 8;
        }
        case 0xd3: {
            // SET 2 E
            if(cpuDebug()) printf("SET 2 E\n");
            set(gameBoy, 2, &gameBoy->cpu.e);
            return 8;
        }
        case 0xd4: {
            // SET 2 H
            if(cpuDebug()) printf("SET 2 H\n");
            set(gameBoy, 2, &gameBoy->cpu.h);
            return 8;
        }
        case 0xd5: {
            // SET 2 L
            if(cpuDebug()) printf("SET 2 L\n");
            set(gameBoy, 2, &gameBoy->cpu.l);
            return 8;
        }
        case 0xd6: {
            // SET 2 (HL)
            if(cpuDebug()) printf("SET 2 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 2, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xd7: {
            // SET 2 A
            if(cpuDebug()) printf("SET 2 A\n");
            set(gameBoy, 2, &gameBoy->cpu.a);
            return 8;
        }
        case 0xd8: {
            // SET 3 B
            if(cpuDebug()) printf("SET 3 B\n");
            set(gameBoy, 3, &gameBoy->cpu.b);
            return 8;
        }
        case 0xd9: {
            // SET 3 C
            if(cpuDebug()) printf("SET 3 C\n");
            set(gameBoy, 3, &gameBoy->cpu.c);
            return 8;
        }
        case 0xda: {
            // SET 3 D
            if(cpuDebug()) printf("SET 3 D\n");
            set(gameBoy, 3, &gameBoy->cpu.d);
            return 8;
        }
        case 0xdb: {
            // SET 3 E
            if(cpuDebug()) printf("SET 3 E\n");
            set(gameBoy, 3, &gameBoy->cpu.e);
            return 8;
        }
        case 0xdc: {
            // SET 3 H
            if(cpuDebug()) printf("SET 3 H\n");
            set(gameBoy, 3, &gameBoy->cpu.h);
            return 8;
        }
        case 0xdd: {
            // SET 3 L
            if(cpuDebug()) printf("SET 3 L\n");
            set(gameBoy, 3, &gameBoy->cpu.l);
            return 8;
        }
        case 0xde: {
            // SET 3 (HL)
            if(cpuDebug()) printf("SET 3 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 3, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xdf: {
            // SET 3 A
            if(cpuDebug()) printf("SET 3 A\n");
            set(gameBoy, 3, &gameBoy->cpu.a);
            return 8;
        }
        case 0xe0: {
            // SET 4 B
            if(cpuDebug()) printf("SET 4 B\n");
            set(gameBoy, 4, &gameBoy->cpu.b);
            return 8;
        }
        case 0xe1: {
            // SET 4 C
            if(cpuDebug()) printf("SET 4 C\n");
            set(gameBoy, 4, &gameBoy->cpu.c);
            return 8;
        }
        case 0xe2: {
            // SET 4 D
            if(cpuDebug()) printf("SET 4 D\n");
            set(gameBoy, 4, &gameBoy->cpu.d);
            return 8;
        }
        case 0xe3: {
            // SET 4 E
            if(cpuDebug()) printf("SET 4 E\n");
            set(gameBoy, 4, &gameBoy->cpu.e);
            return 8;
        }
        case 0xe4: {
            // SET 4 H
            if(cpuDebug()) printf("SET 4 H\n");
            set(gameBoy, 4, &gameBoy->cpu.h);
            return 8;
        }
        case 0xe5: {
            // SET 4 L
            if(cpuDebug()) printf("SET 4 L\n");
            set(gameBoy, 4, &gameBoy->cpu.l);
            return 8;
        }
        case 0xe6: {
            // SET 4 (HL)
            if(cpuDebug()) printf("SET 4 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 4, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xe7: {
            // SET 4 A
            if(cpuDebug()) printf("SET 4 A\n");
            set(gameBoy, 4, &gameBoy->cpu.a);
            return 8;
        }
        case 0xe8: {
            // SET 5 B
            if(cpuDebug()) printf("SET 5 B\n");
            set(gameBoy, 5, &gameBoy->cpu.b);
            return 8;
        }
        case 0xe9: {
            // SET 5 C
            if(cpuDebug()) printf("SET 5 C\n");
            set(gameBoy, 5, &gameBoy->cpu.c);
            return 8;
        }
        case 0xea: {
            // SET 5 D
            if(cpuDebug()) printf("SET 5 D\n");
            set(gameBoy, 5, &gameBoy->cpu.d);
            return 8;
        }
        case 0xeb: {
            // SET 5 E
            if(cpuDebug()) printf("SET 5 E\n");
            set(gameBoy, 5, &gameBoy->cpu.e);
            return 8;
        }
        case 0xec: {
            // SET 5 H
            if(cpuDebug()) printf("SET 5 H\n");
            set(gameBoy, 5, &gameBoy->cpu.h);
            return 8;
        }
        case 0xed: {
            // SET 5 L
            if(cpuDebug()) printf("SET 5 L\n");
            set(gameBoy, 5, &gameBoy->cpu.l);
            return 8;
        }
        case 0xee: {
            // SET 5 (HL)
            if(cpuDebug()) printf("SET 5 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 5, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xef: {
            // SET 5 A
            if(cpuDebug()) printf("SET 5 A\n");
            set(gameBoy, 5, &gameBoy->cpu.a);
            return 8;
        }
        case 0xf0: {
            // SET 6 B
            if(cpuDebug()) printf("SET 6 B\n");
            set(gameBoy, 6, &gameBoy->cpu.b);
            return 8;
        }
        case 0xf1: {
            // SET 6 C
            if(cpuDebug()) printf("SET 6 C\n");
            set(gameBoy, 6, &gameBoy->cpu.c);
            return 8;
        }
        case 0xf2: {
            // SET 6 D
            if(cpuDebug()) printf("SET 6 D\n");
            set(gameBoy, 6, &gameBoy->cpu.d);
            return 8;
        }
        case 0xf3: {
            // SET 6 E
            if(cpuDebug()) printf("SET 6 E\n");
            set(gameBoy, 6, &gameBoy->cpu.e);
            return 8;
        }
        case 0xf4: {
            // SET 6 H
            if(cpuDebug()) printf("SET 6 H\n");
            set(gameBoy, 6, &gameBoy->cpu.h);
            return 8;
        }
        case 0xf5: {
            // SET 6 L
            if(cpuDebug()) printf("SET 6 L\n");
            set(gameBoy, 6, &gameBoy->cpu.l);
            return 8;
        }
        case 0xf6: {
            // SET 6 (HL)
            if(cpuDebug()) printf("SET 6 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 6, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xf7: {
            // SET 6 A
            if(cpuDebug()) printf("SET 6 A\n");
            set(gameBoy, 6, &gameBoy->cpu.a);
            return 8;
        }
        case 0xf8: {
            // SET 7 B
            if(cpuDebug()) printf("SET 7 B\n");
            set(gameBoy, 7, &gameBoy->cpu.b);
            return 8;
        }
        case 0xf9: {
            // SET 7 C
            if(cpuDebug()) printf("SET 7 C\n");
            set(gameBoy, 7, &gameBoy->cpu.c);
            return 8;
        }
        case 0xfa: {
            // SET 7 D
            if(cpuDebug()) printf("SET 7 D\n");
            set(gameBoy, 7, &gameBoy->cpu.d);
            return 8;
        }
        case 0xfb: {
            // SET 7 E
            if(cpuDebug()) printf("SET 7 E\n");
            set(gameBoy, 7, &gameBoy->cpu.e);
            return 8;
        }
        case 0xfc: {
            // SET 7 H
            if(cpuDebug()) printf("SET 7 H\n");
            set(gameBoy, 7, &gameBoy->cpu.h);
            return 8;
        }
        case 0xfd: {
            // SET 7 L
            if(cpuDebug()) printf("SET 7 L\n");
            set(gameBoy, 7, &gameBoy->cpu.l);
            return 8;
        }
        case 0xfe: {
            // SET 7 (HL)
            if(cpuDebug()) printf("SET 7 (HL)\n");
            uint16_t address = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, address);
            set(gameBoy, 7, &value);
            writeToMemory(gameBoy, address, value);
            return 16;
        }
        case 0xff: {
            // SET 7 A
            if(cpuDebug()) printf("SET 7 A\n");
            set(gameBoy, 7, &gameBoy->cpu.a);
            return 8;
        }
        default: {
            printf("CB Instruction Not Found: %x\n", instruction);
            exit(1);
        }
    }
}

void ld_word(uint8_t* lowerDes, uint8_t* upperDes, const uint8_t lower, const uint8_t upper) {
    *lowerDes = lower;
    *upperDes = upper;
}

void ld_byte(uint8_t* des, const uint8_t src) { *des = src; }

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

void add_word(GameBoy* gameBoy, uint8_t* lowerDes, uint8_t* upperDes, const uint8_t lower, const uint8_t upper) {
    uint16_t desWord = compose_bytes(*lowerDes, *upperDes);
    uint16_t srcWord = compose_bytes(lower, upper);
    unsigned int result = (desWord + srcWord);
    *upperDes = (uint8_t) (result >> 8);
    *lowerDes = (uint8_t) (result);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = ((desWord & 0xfff) + (srcWord & 0xfff) > 0xfff);
    gameBoy->cpu.carry = ((result & 0x10000) != 0);
}

void add_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t addend) {
    uint8_t first = *des;
    uint8_t second = addend;
    unsigned int result = first + second;
    *des = (uint8_t) result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = ((first & 0xf) + (second & 0xf) > 0xf);
    gameBoy->cpu.carry = ((result & 0x100) != 0);
}

void adc_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t addend) {
    uint8_t first = *des;
    uint8_t second = addend;
    uint8_t carry = gameBoy->cpu.carry;
    unsigned int result = first + second + carry;
    *des = (uint8_t) result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = (((first & 0xf) + (second & 0xf) + carry) > 0xf);
    gameBoy->cpu.carry = (result > 0xff);
}

void sub_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t subtrahend) {
    uint8_t first = *des;
    uint8_t second = subtrahend;
    uint8_t result = first - second;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = (((first & 0xf) - (second & 0xf)) < 0);
    gameBoy->cpu.carry = (first < second);
}

void sbc_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t subtrahend) {
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

void and_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t value) {
    uint8_t result = *des & value;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = true;
    gameBoy->cpu.carry = false;
}

void xor_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t value) {
    uint8_t result = *des ^ value;
    *des = result;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = false;
}

void or_byte(GameBoy* gameBoy, uint8_t* des, const uint8_t value) {
    uint8_t result = *des | value;
    *des = result;
    gameBoy->cpu.zero = (*des == 0);
    gameBoy->cpu.subtract = false;
    gameBoy->cpu.halfCarry = false;
    gameBoy->cpu.carry = false;
}

void cp_byte(GameBoy* gameBoy, const uint8_t des, const uint8_t value) {
    uint8_t first = des;
    uint8_t second = value;
    uint8_t result = first - second;
    gameBoy->cpu.zero = (result == 0);
    gameBoy->cpu.subtract = true;
    gameBoy->cpu.halfCarry = (((first & 0xf) - (second & 0xf)) < 0);
    gameBoy->cpu.carry = (first < second);
}

void ret(GameBoy* gameBoy) {
    uint8_t lower = 0;
    uint8_t upper = 0;
    pop(gameBoy, &lower, &upper);
    jp_from_bytes(gameBoy, lower, upper);
}

void jp_from_word(GameBoy* gameBoy, const uint16_t address) { gameBoy->cpu.pc = address; }

void jp_from_bytes(GameBoy* gameBoy, const uint8_t lower, const uint8_t upper) { jp_from_word(gameBoy, compose_bytes(lower, upper)); }

void jp_from_pc(GameBoy* gameBoy) {
    uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    jp_from_bytes(gameBoy, lower, upper);
}

void call(GameBoy* gameBoy) {
    uint8_t lowerNew = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint8_t upperNew = readFromMemory(gameBoy, gameBoy->cpu.pc++);
    uint8_t upperNext = (uint8_t) (gameBoy->cpu.pc >> 8);
    uint8_t lowerNext = (uint8_t) (gameBoy->cpu.pc);
    push(gameBoy, lowerNext, upperNext);
    jp_from_bytes(gameBoy, lowerNew, upperNew);
}

void rst(GameBoy* gameBoy, const uint8_t value) {
    uint8_t upper = (uint8_t) (gameBoy->cpu.pc >> 8);
    uint8_t lower = (uint8_t) (gameBoy->cpu.pc);
    push(gameBoy, lower, upper);
    jp_from_word(gameBoy, 0x0000 + value);
}

void jr(GameBoy* gameBoy) {
    int8_t value = (int8_t) readFromMemory(gameBoy, gameBoy->cpu.pc++);
    jp_from_word(gameBoy, gameBoy->cpu.pc + value);
}

int decodeAndExecute(GameBoy* gameBoy, const uint8_t instruction) {
    switch(instruction) {
        case 0x00: {
            // NOP
            if(cpuDebug()) printf("NOP\n");
            return 4;
        }
        case 0x01: {
            // LD BC, u16
            if(cpuDebug()) printf("LD BC, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.c, &gameBoy->cpu.b, lower, upper);
            return 12;
        }
        case 0x02: {
            // LD (BC), A
            if(cpuDebug()) printf("LD (BC), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.c, gameBoy->cpu.b), gameBoy->cpu.a);
            return 8;
        }
        case 0x03: {
            // INC BC
            if(cpuDebug()) printf("INC BC\n");
            inc_word(&gameBoy->cpu.c, &gameBoy->cpu.b);
            return 8;
        }
        case 0x04: {
            // INC B
            if(cpuDebug()) printf("INC B\n");
            inc_byte(gameBoy, &gameBoy->cpu.b);
            return 4;
        }
        case 0x05: {
            // DEC B
            if(cpuDebug()) printf("DEC B\n");
            dec_byte(gameBoy, &gameBoy->cpu.b);
            return 4;
        }
        case 0x06: {
            // LD B, u8
            if(cpuDebug()) printf("LD B, u8\n");
            ld_byte(&gameBoy->cpu.b, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x07: {
            // RLCA
            if(cpuDebug()) printf("RLCA\n");
            rlc(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x08: {
            // LD (u16), SP
            if(cpuDebug()) printf("LD (u16), SP\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t address = compose_bytes(lower, upper);
            writeToMemory(gameBoy, address, (uint8_t) (gameBoy->cpu.sp));
            writeToMemory(gameBoy, address + 1, (uint8_t) (gameBoy->cpu.sp >> 8));
            return 20;
        }
        case 0x09: {
            // ADD HL, BC
            if(cpuDebug()) printf("ADD HL, BC\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.c, gameBoy->cpu.b);
            return 8;
        }
        case 0x0a: {
            // LD A, (BC)
            if(cpuDebug()) printf("LD A, (BC)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.c, gameBoy->cpu.b)));
            return 8;
        }
        case 0x0b: {
            // DEC BC
            if(cpuDebug()) printf("DEC BC\n");
            dec_word(&gameBoy->cpu.c, &gameBoy->cpu.b);
            return 8;
        }
        case 0x0c: {
            // INC C
            if(cpuDebug()) printf("INC C\n");
            inc_byte(gameBoy, &gameBoy->cpu.c);
            return 4;
        }
        case 0x0d: {
            // DEC C
            if(cpuDebug()) printf("DEC C\n");
            dec_byte(gameBoy, &gameBoy->cpu.c);
            return 4;
        }
        case 0x0e: {
            // LD C, u8
            if(cpuDebug()) printf("LD C, u8\n");
            ld_byte(&gameBoy->cpu.c, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x0f: {
            // RRCA
            if(cpuDebug()) printf("RRCA\n");
            rrc(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x10: {
            // STOP
            if(cpuDebug()) printf("STOP\n");
            /*
            gameBoy->cpu.halted = true;
            gameBoy->cpu.pc++;
            */
            return 4;
        }
        case 0x11: {
            // LD DE, u16
            if(cpuDebug()) printf("LD DE, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.e, &gameBoy->cpu.d, lower, upper);
            return 12;
        }
        case 0x12: {
            // LD (DE), A
            if(cpuDebug()) printf("LD (DE), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.e, gameBoy->cpu.d), gameBoy->cpu.a);
            return 8;
        }
        case 0x13: {
            // INC DE
            if(cpuDebug()) printf("INC DE\n");
            inc_word(&gameBoy->cpu.e, &gameBoy->cpu.d);
            return 8;
        }
        case 0x14: {
            // INC D
            if(cpuDebug()) printf("INC D\n");
            inc_byte(gameBoy, &gameBoy->cpu.d);
            return 4;
        }
        case 0x15: {
            // DEC D
            if(cpuDebug()) printf("DEC D\n");
            dec_byte(gameBoy, &gameBoy->cpu.d);
            return 4;
        }
        case 0x16: {
            // LD D, u8
            if(cpuDebug()) printf("LD D, u8\n");
            ld_byte(&gameBoy->cpu.d, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x17: {
            // RLA
            if(cpuDebug()) printf("RLA\n");
            rl(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x18: {
            // JR i8
            if(cpuDebug()) printf("JR i8\n");
            jr(gameBoy);
            return 12;
        }
        case 0x19: {
            // ADD HL, DE
            if(cpuDebug()) printf("ADD HL, DE\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.e, gameBoy->cpu.d);
            return 8;
        }
        case 0x1a: {
            // LD A, (DE)
            if(cpuDebug()) printf("LD A, (DE)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.e, gameBoy->cpu.d)));
            return 8;
        }
        case 0x1b: {
            // DEC DE
            if(cpuDebug()) printf("DEC DE\n");
            dec_word(&gameBoy->cpu.e, &gameBoy->cpu.d);
            return 8;
        }
        case 0x1c: {
            // INC E
            if(cpuDebug()) printf("INC E\n");
            inc_byte(gameBoy, &gameBoy->cpu.e);
            return 4;
        }
        case 0x1d: {
            // DEC E
            if(cpuDebug()) printf("DEC E\n");
            dec_byte(gameBoy, &gameBoy->cpu.e);
            return 4;
        }
        case 0x1e: {
            // LD E, u8
            if(cpuDebug()) printf("LD E, u8\n");
            ld_byte(&gameBoy->cpu.e, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x1f: {
            // RRA
            if(cpuDebug()) printf("RRA\n");
            rr(gameBoy, &gameBoy->cpu.a);
            gameBoy->cpu.zero = false;
            return 4;
        }
        case 0x20: {
            // JR NZ, i8
            if(cpuDebug()) printf("JR NZ, i8\n");
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
            if(cpuDebug()) printf("LD HL, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            ld_word(&gameBoy->cpu.l, &gameBoy->cpu.h, lower, upper);
            return 12;
        }
        case 0x22: {
            // LD (HL+), A
            if(cpuDebug()) printf("LD (HL+), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.a);
            inc_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x23: {
            // INC HL
            if(cpuDebug()) printf("INC HL\n");
            inc_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x24: {
            // INC H
            if(cpuDebug()) printf("INC H\n");
            inc_byte(gameBoy, &gameBoy->cpu.h);
            return 4;
        }
        case 0x25: {
            // DEC H
            if(cpuDebug()) printf("DEC H\n");
            dec_byte(gameBoy, &gameBoy->cpu.h);
            return 4;
        }
        case 0x26: {
            // LD H, u8
            if(cpuDebug()) printf("LD H, u8\n");
            ld_byte(&gameBoy->cpu.h, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x27: {
            // DAA
            if(cpuDebug()) printf("DAA\n");
            uint16_t correction = (gameBoy->cpu.carry ? 0x60 : 0x00);

            if(gameBoy->cpu.halfCarry || (!gameBoy->cpu.subtract && ((gameBoy->cpu.a & 0x0f) > 9)))
                correction |= 0x06;
            if(gameBoy->cpu.carry || (!gameBoy->cpu.subtract && (gameBoy->cpu.a > 0x99)))
                correction |= 0x60;

            if(gameBoy->cpu.subtract)
                gameBoy->cpu.a = ((uint8_t) (gameBoy->cpu.a - correction));
            else
                gameBoy->cpu.a = ((uint8_t) (gameBoy->cpu.a + correction));

            if(((correction << 2) & 0x100) != 0)
                gameBoy->cpu.carry = true;
            gameBoy->cpu.zero = (gameBoy->cpu.a == 0);
            gameBoy->cpu.halfCarry = false;
            return 4;
        }
        case 0x28: {
            // JR Z, i8
            if(cpuDebug()) printf("JR Z, i8\n");
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
            if(cpuDebug()) printf("ADD HL, HL\n");
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, gameBoy->cpu.l, gameBoy->cpu.h);
            return 8;
        }
        case 0x2a: {
            // LD A, (HL+)
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, hl);
            if(cpuDebug()) printf("LD A, (HL+) (HL=%x)=%x\n", hl, value);
            ld_byte(&gameBoy->cpu.a, value);
            inc_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x2b: {
            // DEC HL
            if(cpuDebug()) printf("DEC HL\n");
            dec_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x2c: {
            // INC L
            if(cpuDebug()) printf("INC L\n");
            inc_byte(gameBoy, &gameBoy->cpu.l);
            return 4;
        }
        case 0x2d: {
            // DEC L
            if(cpuDebug()) printf("DEC L\n");
            dec_byte(gameBoy, &gameBoy->cpu.l);
            return 4;
        }
        case 0x2e: {
            // LD L, u8
            if(cpuDebug()) printf("LD L, u8\n");
            ld_byte(&gameBoy->cpu.l, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x2f: {
            // CPL
            if(cpuDebug()) printf("CPL\n");
            gameBoy->cpu.a = ~gameBoy->cpu.a;
            gameBoy->cpu.subtract = true;
            gameBoy->cpu.halfCarry = true;
            return 4;
        }
        case 0x30: {
            // JR NC, i8
            if(cpuDebug()) printf("JR NC, i8\n");
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
            if(cpuDebug()) printf("LD SP, u16\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            gameBoy->cpu.sp = compose_bytes(lower, upper);
            return 12;
        }
        case 0x32: {
            // LD (HL-), A
            if(cpuDebug()) printf("LD (HL-), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.a);
            dec_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x33: {
            // INC SP
            if(cpuDebug()) printf("INC SP\n");
            gameBoy->cpu.sp++;
            return 8;
        }
        case 0x34: {
            // INC (HL)
            if(cpuDebug()) printf("INC (HL)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, hl);
            inc_byte(gameBoy, &value);
            writeToMemory(gameBoy, hl, value);
            return 12;
        }
        case 0x35: {
            // DEC (HL)
            if(cpuDebug()) printf("DEC (HL)\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            uint8_t value = readFromMemory(gameBoy, hl);
            dec_byte(gameBoy, &value);
            writeToMemory(gameBoy, hl, value);
            return 12;
        }
        case 0x36: {
            // LD (HL), u8
            if(cpuDebug()) printf("LD (HL), u8\n");
            uint16_t hl = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            writeToMemory(gameBoy, hl, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 12;
        }
        case 0x37: {
            // SCF
            if(cpuDebug()) printf("SCF\n");
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = false;
            gameBoy->cpu.carry = true;
            return 4;
        }
        case 0x38: {
            // JR C, i8
            if(cpuDebug()) printf("JR C, i8\n");
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
            if(cpuDebug()) printf("ADD HL, SP\n");
            uint8_t lower = 0;
            uint8_t upper = 0;
            decompose_bytes(gameBoy->cpu.sp, &lower, &upper);
            add_word(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h, lower, upper);
            return 8;
        }
        case 0x3a: {
            // LD A, (HL-)
            if(cpuDebug()) printf("LD A, (HL-)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            dec_word(&gameBoy->cpu.l, &gameBoy->cpu.h);
            return 8;
        }
        case 0x3b: {
            // DEC SP
            if(cpuDebug()) printf("DEC SP\n");
            gameBoy->cpu.sp--;
            return 8;
        }
        case 0x3c: {
            // INC A
            if(cpuDebug()) printf("INC A\n");
            inc_byte(gameBoy, &gameBoy->cpu.a);
            return 4;
        }
        case 0x3d: {
            // DEC A
            if(cpuDebug()) printf("DEC A\n");
            dec_byte(gameBoy, &gameBoy->cpu.a);
            return 4;
        }
        case 0x3e: {
            // LD A, u8
            if(cpuDebug()) printf("LD A, u8\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0x3f: {
            // CCF
            if(cpuDebug()) printf("CCF\n");
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = false;
            gameBoy->cpu.carry = !gameBoy->cpu.carry;
            return 4;
        }
        case 0x40: {
            // LD B, B
            if(cpuDebug()) printf("LD B, B\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.b);
            return 4;
        }
        case 0x41: {
            // LD B, C
            if(cpuDebug()) printf("LD B, C\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.c);
            return 4;
        }
        case 0x42: {
            // LD B, D
            if(cpuDebug()) printf("LD B, D\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.d);
            return 4;
        }
        case 0x43: {
            // LD B, E
            if(cpuDebug()) printf("LD B, E\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.e);
            return 4;
        }
        case 0x44: {
            // LD B, H
            if(cpuDebug()) printf("LD B, H\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.h);
            return 4;
        }
        case 0x45: {
            // LD B, L
            if(cpuDebug()) printf("LD B, L\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.l);
            return 4;
        }
        case 0x46: {
            // LD B, (HL)
            if(cpuDebug()) printf("LD B, (HL)\n");
            ld_byte(&gameBoy->cpu.b, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x47: {
            // LD B, A
            if(cpuDebug()) printf("LD B, A\n");
            ld_byte(&gameBoy->cpu.b, gameBoy->cpu.a);
            return 4;
        }
        case 0x48: {
            // LD C, B
            if(cpuDebug()) printf("LD C, B\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.b);
            return 4;
        }
        case 0x49: {
            // LD C, C
            if(cpuDebug()) printf("LD C, C\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.c);
            return 4;
        }
        case 0x4a: {
            // LD C, D
            if(cpuDebug()) printf("LD C, D\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.d);
            return 4;
        }
        case 0x4b: {
            // LD C, E
            if(cpuDebug()) printf("LD C, E\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.e);
            return 4;
        }
        case 0x4c: {
            // LD C, H
            if(cpuDebug()) printf("LD C, H\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.h);
            return 4;
        }
        case 0x4d: {
            // LD C, L
            if(cpuDebug()) printf("LD C, L\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.l);
            return 4;
        }
        case 0x4e: {
            // LD C, (HL)
            if(cpuDebug()) printf("LD C, (HL)\n");
            ld_byte(&gameBoy->cpu.c, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x4f: {
            // LD C, A
            if(cpuDebug()) printf("LD C, A\n");
            ld_byte(&gameBoy->cpu.c, gameBoy->cpu.a);
            return 4;
        }
        case 0x50: {
            // LD D, B
            if(cpuDebug()) printf("LD D, B\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.b);
            return 4;
        }
        case 0x51: {
            // LD D, C
            if(cpuDebug()) printf("LD D, C\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.c);
            return 4;
        }
        case 0x52: {
            // LD D, D
            if(cpuDebug()) printf("LD D, D\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.d);
            return 4;
        }
        case 0x53: {
            // LD D, E
            if(cpuDebug()) printf("LD D, E\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.e);
            return 4;
        }
        case 0x54: {
            // LD D, H
            if(cpuDebug()) printf("LD D, H\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.h);
            return 4;
        }
        case 0x55: {
            // LD D, L
            if(cpuDebug()) printf("LD D, L\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.l);
            return 4;
        }
        case 0x56: {
            // LD D, (HL)
            if(cpuDebug()) printf("LD D, (HL)\n");
            ld_byte(&gameBoy->cpu.d, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x57: {
            // LD D, A
            if(cpuDebug()) printf("LD D, A\n");
            ld_byte(&gameBoy->cpu.d, gameBoy->cpu.a);
            return 4;
        }
        case 0x58: {
            // LD E, B
            if(cpuDebug()) printf("LD E, B\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.b);
            return 4;
        }
        case 0x59: {
            // LD E, C
            if(cpuDebug()) printf("LD E, C\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.c);
            return 4;
        }
        case 0x5a: {
            // LD E, D
            if(cpuDebug()) printf("LD E, D\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.d);
            return 4;
        }
        case 0x5b: {
            // LD E, E
            if(cpuDebug()) printf("LD E, E\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.e);
            return 4;
        }
        case 0x5c: {
            // LD E, H
            if(cpuDebug()) printf("LD E, H\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.h);
            return 4;
        }
        case 0x5d: {
            // LD E, L
            if(cpuDebug()) printf("LD E, L\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.l);
            return 4;
        }
        case 0x5e: {
            // LD E, (HL)
            if(cpuDebug()) printf("LD E, (HL)\n");
            ld_byte(&gameBoy->cpu.e, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x5f: {
            // LD E, A
            if(cpuDebug()) printf("LD E, A\n");
            ld_byte(&gameBoy->cpu.e, gameBoy->cpu.a);
            return 4;
        }
        case 0x60: {
            // LD H, B
            if(cpuDebug()) printf("LD H, B\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.b);
            return 4;
        }
        case 0x61: {
            // LD H, C
            if(cpuDebug()) printf("LD H, C\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.c);
            return 4;
        }
        case 0x62: {
            // LD H, D
            if(cpuDebug()) printf("LD H, D\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.d);
            return 4;
        }
        case 0x63: {
            // LD H, E
            if(cpuDebug()) printf("LD H, E\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.e);
            return 4;
        }
        case 0x64: {
            // LD H, H
            if(cpuDebug()) printf("LD H, H\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.h);
            return 4;
        }
        case 0x65: {
            // LD H, L
            if(cpuDebug()) printf("LD H, L\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.l);
            return 4;
        }
        case 0x66: {
            // LD H, (HL)
            if(cpuDebug()) printf("LD H, (HL)\n");
            ld_byte(&gameBoy->cpu.h, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x67: {
            // LD H, A
            if(cpuDebug()) printf("LD H, A\n");
            ld_byte(&gameBoy->cpu.h, gameBoy->cpu.a);
            return 4;
        }
        case 0x68: {
            // LD L, B
            if(cpuDebug()) printf("LD L, B\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.b);
            return 4;
        }
        case 0x69: {
            // LD L, C
            if(cpuDebug()) printf("LD L, C\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.c);
            return 4;
        }
        case 0x6a: {
            // LD L, D
            if(cpuDebug()) printf("LD L, D\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.d);
            return 4;
        }
        case 0x6b: {
            // LD L, E
            if(cpuDebug()) printf("LD L, E\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.e);
            return 4;
        }
        case 0x6c: {
            // LD L, H
            if(cpuDebug()) printf("LD L, H\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.h);
            return 4;
        }
        case 0x6d: {
            // LD L, L
            if(cpuDebug()) printf("LD L, L\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.l);
            return 4;
        }
        case 0x6e: {
            // LD L, (HL)
            if(cpuDebug()) printf("LD L, (HL)\n");
            ld_byte(&gameBoy->cpu.l, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x6f: {
            // LD L, A
            if(cpuDebug()) printf("LD L, A\n");
            ld_byte(&gameBoy->cpu.l, gameBoy->cpu.a);
            return 4;
        }
        case 0x70: {
            // LD (HL), B
            if(cpuDebug()) printf("LD (HL), B\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.b);
            return 8;
        }
        case 0x71: {
            // LD (HL), C
            if(cpuDebug()) printf("LD (HL), C\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.c);
            return 8;
        }
        case 0x72: {
            // LD (HL), D
            if(cpuDebug()) printf("LD (HL), D\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.d);
            return 8;
        }
        case 0x73: {
            // LD (HL), E
            if(cpuDebug()) printf("LD (HL), E\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.e);
            return 8;
        }
        case 0x74: {
            // LD (HL), H
            if(cpuDebug()) printf("LD (HL), H\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.h);
            return 8;
        }
        case 0x75: {
            // LD (HL), L
            if(cpuDebug()) printf("LD (HL), L\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.l);
            return 8;
        }
        case 0x76: {
            // HALT
            if(cpuDebug()) printf("HALT\n");
            gameBoy->cpu.halted = true;
            return 4;
        }
        case 0x77: {
            // LD (HL), A
            if(cpuDebug()) printf("LD (HL), A\n");
            writeToMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h), gameBoy->cpu.a);
            return 8;
        }
        case 0x78: {
            // LD A, B
            if(cpuDebug()) printf("LD A, B\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x79: {
            // LD A, C
            if(cpuDebug()) printf("LD A, C\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x7a: {
            // LD A, D
            if(cpuDebug()) printf("LD A, D\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x7b: {
            // LD A, E
            if(cpuDebug()) printf("LD A, E\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x7c: {
            // LD A, H
            if(cpuDebug()) printf("LD A, H\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x7d: {
            // LD A, L
            if(cpuDebug()) printf("LD A, L\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x7e: {
            // LD A, (HL)
            if(cpuDebug()) printf("LD A, (HL)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x7f: {
            // LD A, A
            if(cpuDebug()) printf("LD A, A\n");
            ld_byte(&gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x80: {
            // ADD A, B
            if(cpuDebug()) printf("ADD A, B\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x81: {
            // ADD A, C
            if(cpuDebug()) printf("ADD A, C\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x82: {
            // ADD A, D
            if(cpuDebug()) printf("ADD A, D\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x83: {
            // ADD A, E
            if(cpuDebug()) printf("ADD A, E\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x84: {
            // ADD A, H
            if(cpuDebug()) printf("ADD A, H\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x85: {
            // ADD A, L
            if(cpuDebug()) printf("ADD A, L\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x86: {
            // ADD A, (HL)
            if(cpuDebug()) printf("ADD A, (HL)\n");
            add_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x87: {
            // ADD A, A
            if(cpuDebug()) printf("ADD A, A\n");
            add_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x88: {
            // ADC A, B
            if(cpuDebug()) printf("ADC A, B\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x89: {
            // ADC A, C
            if(cpuDebug()) printf("ADC A, C\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x8a: {
            // ADC A, D
            if(cpuDebug()) printf("ADC A, D\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x8b: {
            // ADC A, E
            if(cpuDebug()) printf("ADC A, E\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x8c: {
            // ADC A, H
            if(cpuDebug()) printf("ADC A, H\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x8d: {
            // ADC A, L
            if(cpuDebug()) printf("ADC A, L\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x8e: {
            // ADC A, (HL)
            if(cpuDebug()) printf("ADC A, (HL)\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x8f: {
            // ADC A, A
            if(cpuDebug()) printf("ADC A, A\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x90: {
            // SUB B
            if(cpuDebug()) printf("SUB B\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x91: {
            // SUB C
            if(cpuDebug()) printf("SUB C\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x92: {
            // SUB D
            if(cpuDebug()) printf("SUB D\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x93: {
            // SUB E
            if(cpuDebug()) printf("SUB E\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x94: {
            // SUB H
            if(cpuDebug()) printf("SUB H\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x95: {
            // SUB L
            if(cpuDebug()) printf("SUB L\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x96: {
            // SUB (HL)
            if(cpuDebug()) printf("SUB (HL)\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x97: {
            // SUB A, A
            if(cpuDebug()) printf("SUB A, A\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0x98: {
            // SBC A, B
            if(cpuDebug()) printf("SBC A, B\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0x99: {
            // SBC A, C
            if(cpuDebug()) printf("SBC A, C\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0x9a: {
            // SBC A, D
            if(cpuDebug()) printf("SBC A, D\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0x9b: {
            // SBC A, E
            if(cpuDebug()) printf("SBC A, E\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0x9c: {
            // SBC A, H
            if(cpuDebug()) printf("SBC A, H\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0x9d: {
            // SBC A, L
            if(cpuDebug()) printf("SBC A, L\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0x9e: {
            // SBC A, (HL)
            if(cpuDebug()) printf("SBC A, (HL)\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0x9f: {
            // SBC A, A
            if(cpuDebug()) printf("SBC A, A\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xa0: {
            // AND A, B
            if(cpuDebug()) printf("AND A, B\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xa1: {
            // AND A, C
            if(cpuDebug()) printf("AND A, C\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xa2: {
            // AND A, D
            if(cpuDebug()) printf("AND A, D\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xa3: {
            // AND A, E
            if(cpuDebug()) printf("AND A, E\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xa4: {
            // AND A, H
            if(cpuDebug()) printf("AND A, H\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xa5: {
            // AND A, L
            if(cpuDebug()) printf("AND A, L\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xa6: {
            // AND A, (HL)
            if(cpuDebug()) printf("AND A, (HL)\n");
            and_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xa7: {
            // AND A, A
            if(cpuDebug()) printf("AND A, A\n");
            and_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xa8: {
            // XOR A, B
            if(cpuDebug()) printf("XOR A, B\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);            
            return 4;
        }
        case 0xa9: {
            // XOR A, C
            if(cpuDebug()) printf("XOR A, C\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xaa: {
            // XOR A, D
            if(cpuDebug()) printf("XOR A, D\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xab: {
            // XOR A, E
            if(cpuDebug()) printf("XOR A, E\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xac: {
            // XOR A, H
            if(cpuDebug()) printf("XOR A, H\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xad: {
            // XOR A, L
            if(cpuDebug()) printf("XOR A, L\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xae: {
            // XOR A, (HL)
            if(cpuDebug()) printf("XOR A, (HL)\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xaf: {
            // XOR A, A
            if(cpuDebug()) printf("XOR A, A\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xb0: {
            // OR A, B
            if(cpuDebug()) printf("OR A, B\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xb1: {
            // OR A, C
            if(cpuDebug()) printf("OR A, C\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xb2: {
            // OR A, D
            if(cpuDebug()) printf("OR A, D\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xb3: {
            // OR A, E
            if(cpuDebug()) printf("OR A, E\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xb4: {
            // OR A, H
            if(cpuDebug()) printf("OR A, H\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xb5: {
            // OR A, L
            if(cpuDebug()) printf("OR A, L\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xb6: {
            // OR A, (HL)
            if(cpuDebug()) printf("OR A, (HL)\n");
            or_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xb7: {
            // OR A, A
            if(cpuDebug()) printf("OR A, A\n");
            or_byte(gameBoy, &gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xb8: {
            // CP A, B
            if(cpuDebug()) printf("CP A, B\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.b);
            return 4;
        }
        case 0xb9: {
            // CP A, C
            if(cpuDebug()) printf("CP A, C\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.c);
            return 4;
        }
        case 0xba: {
            // CP A, D
            if(cpuDebug()) printf("CP A, D\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.d);
            return 4;
        }
        case 0xbb: {
            // CP A, E
            if(cpuDebug()) printf("CP A, E\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.e);
            return 4;
        }
        case 0xbc: {
            // CP A, H
            if(cpuDebug()) printf("CP A, H\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.h);
            return 4;
        }
        case 0xbd: {
            // CP A, L
            if(cpuDebug()) printf("CP A, L\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.l);
            return 4;
        }
        case 0xbe: {
            // CP A, (HL)
            if(cpuDebug()) printf("CP A, (HL)\n");
            cp_byte(gameBoy, gameBoy->cpu.a, readFromMemory(gameBoy, compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h)));
            return 8;
        }
        case 0xbf: {
            // CP A, A
            if(cpuDebug()) printf("CP A, A\n");
            cp_byte(gameBoy, gameBoy->cpu.a, gameBoy->cpu.a);
            return 4;
        }
        case 0xc0: {
            // RET NZ
            if(cpuDebug()) printf("RET NZ\n");
            if(!gameBoy->cpu.zero) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xc1: {
            // POP BC
            if(cpuDebug()) printf("POP BC\n");
            pop(gameBoy, &gameBoy->cpu.c, &gameBoy->cpu.b);
            return 12;
        }
        case 0xc2: {
            // JP NZ, u16
            if(cpuDebug()) printf("JP NZ, u16\n");
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
            if(cpuDebug()) printf("JP u16\n");
            jp_from_pc(gameBoy);
            return 16;
        }
        case 0xc4: {
            // CALL NZ, u16
            if(cpuDebug()) printf("CALL NZ, u16\n");
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
            if(cpuDebug()) printf("PUSH BC\n");
            push(gameBoy, gameBoy->cpu.c, gameBoy->cpu.b);
            return 16;
        }
        case 0xc6: {
            // ADD A, u8
            if(cpuDebug()) printf("ADD A, u8\n");
            add_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xc7: {
            // RST 00H
            if(cpuDebug()) printf("RST 00H\n");
            rst(gameBoy, 0x00);
            return 16;
        }
        case 0xc8: {
            // RET Z
            if(cpuDebug()) printf("RET Z\n");
            if(gameBoy->cpu.zero) {
                ret(gameBoy);
                return 20;
            } else {
                return 8;
            }
        }
        case 0xc9: {
            // RET
            if(cpuDebug()) printf("RET\n");
            ret(gameBoy);
            return 16;
        }
        case 0xca: {
            // JP Z, u16
            if(cpuDebug()) printf("JP Z, u16\n");
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
            if(cpuDebug()) printf("CB\n");
            int cycles = 0; // 4 (Check if this matters)
            uint8_t instruction = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            return cycles + decodeAndExecuteCB(gameBoy, instruction);
        }
        case 0xcc: {
            // CALL Z, u16
            if(cpuDebug()) printf("CALL Z, u16\n");
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
            if(cpuDebug()) printf("CALL u16\n");
            call(gameBoy);
            return 24;
        }
        case 0xce: {
            // ADC A, u8
            if(cpuDebug()) printf("ADC A, u8\n");
            adc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xcf: {
            // RST 08H
            if(cpuDebug()) printf("RST 08H\n");
            rst(gameBoy, 0x08);
            return 16;
        }
        case 0xd0: {
            // RET NC
            if(cpuDebug()) printf("RET NC\n");
            if(!gameBoy->cpu.carry) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xd1: {
            // POP DE
            if(cpuDebug()) printf("POP DE\n");
            pop(gameBoy, &gameBoy->cpu.e, &gameBoy->cpu.d);
            return 12;
        }
        case 0xd2: {
            // JP NC, u16
            if(cpuDebug()) printf("JP NC, u16\n");
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
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xd3\n");
            return 0;
        }
        case 0xd4: {
            // CALL NC, u16
            if(cpuDebug()) printf("CALL NC, u16\n");
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
            if(cpuDebug()) printf("PUSH DE\n");
            push(gameBoy, gameBoy->cpu.e, gameBoy->cpu.d);
            return 16;
        }
        case 0xd6: {
            // SUB A, u8
            if(cpuDebug()) printf("SUB A, u8\n");
            sub_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xd7: {
            // RST 10H
            if(cpuDebug()) printf("RST 10H\n");
            rst(gameBoy, 0x10);
            return 16;
        }
        case 0xd8: {
            // RET C
            if(cpuDebug()) printf("RET C\n");
            if(gameBoy->cpu.carry) {
                ret(gameBoy);
                return 20;
            } else
                return 8;
        }
        case 0xd9: {
            // RETI
            if(cpuDebug()) printf("RETI\n");
            ret(gameBoy);
            gameBoy->cpu.interruptsEnabled = true;
            return 16;
        }
        case 0xda: {
            // JP C, u16
            if(cpuDebug()) printf("JP C, u16\n");
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
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xdb\n");
            return 0;
        }
        case 0xdc: {
            // CALL C, u16
            if(cpuDebug()) printf("CALL C, u16\n");
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
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xdd\n");
            return 0;
        }
        case 0xde: {
            // SBC A, u8
            if(cpuDebug()) printf("SBC A, u8\n");
            sbc_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xdf: {
            // RST 18H
            if(cpuDebug()) printf("RST 18H\n");
            rst(gameBoy, 0x18);
            return 16;
        }
        case 0xe0: {
            // LDH (u8), A
            if(cpuDebug()) printf("LDH (u8), A\n");
            writeToMemory(gameBoy, 0xff00 + readFromMemory(gameBoy, gameBoy->cpu.pc++), gameBoy->cpu.a);
            return 12;
        }
        case 0xe1: {
            // POP HL
            if(cpuDebug()) printf("POP HL\n");
            pop(gameBoy, &gameBoy->cpu.l, &gameBoy->cpu.h);
            return 12;
        }
        case 0xe2: {
            // LD (C), A
            if(cpuDebug()) printf("LD (C), A\n");
            writeToMemory(gameBoy, 0xff00 + gameBoy->cpu.c, gameBoy->cpu.a);
            return 8;
        }
        case 0xe3: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xe3\n");
            return 0;
        }
        case 0xe4: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xe4\n");
            return 0;
        }
        case 0xe5: {
            // PUSH HL
            if(cpuDebug()) printf("PUSH HL\n");
            push(gameBoy, gameBoy->cpu.l, gameBoy->cpu.h);
            return 16;
        }
        case 0xe6: {
            // AND A, u8
            if(cpuDebug()) printf("AND A, u8\n");
            and_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xe7: {
            // RST 20H
            if(cpuDebug()) printf("RST 20H\n");
            rst(gameBoy, 0x20);
            return 16;
        }
        case 0xe8: {
            // ADD SP, i8
            if(cpuDebug()) printf("ADD SP, i8\n");
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
            if(cpuDebug()) printf("JP HL\n");
            jp_from_bytes(gameBoy, gameBoy->cpu.l, gameBoy->cpu.h);
            return 4;
        }
        case 0xea: {
            // LD (u16), A
            if(cpuDebug()) printf("LD (u16), A\n");
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            writeToMemory(gameBoy, compose_bytes(lower, upper), gameBoy->cpu.a);
            return 16;
        }
        case 0xeb: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xeb\n");
            return 0;
        }
        case 0xec: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xec\n");
            return 0;
        }
        case 0xed: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xed\n");
            return 0;
        }
        case 0xee: {
            // XOR A, u8
            if(cpuDebug()) printf("XOR A, u8\n");
            xor_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xef: {
            // RST 28H
            if(cpuDebug()) printf("RST 28H\n");
            rst(gameBoy, 0x28);
            return 16;
        }
        case 0xf0: {
            // LDH A, (u8)
            uint8_t offset = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t address = 0xff00 + offset;
            uint8_t value = readFromMemory(gameBoy, address);
            if(cpuDebug()) printf("LDH A, ((0xff00 + %x)=%x)=%x\n", offset, address, value);
            ld_byte(&gameBoy->cpu.a, value);
            return 12;
        }
        case 0xf1: {
            // POP AF
            if(cpuDebug()) printf("POP AF\n");
            uint8_t f = 0;
            pop(gameBoy, &f, &gameBoy->cpu.a);
            setFFlagsFromByte(&gameBoy->cpu, f);
            return 12;
        }
        case 0xf2: {
            // LD A, (C)
            if(cpuDebug()) printf("LD A, (C)\n");
            ld_byte(&gameBoy->cpu.a, readFromMemory(gameBoy, 0xff00 + gameBoy->cpu.c));
            return 8;
        }
        case 0xf3: {
            // DI
            if(cpuDebug()) printf("DI\n");
            gameBoy->cpu.pendingInterruptEnable = false;
            gameBoy->cpu.interruptsEnabled = false;
            return 4;
        }
        case 0xf4: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION AT 0xf4\n");
            return 0;
        }
        case 0xf5: {
            // PUSH AF
            if(cpuDebug()) printf("PUSH AF\n");
            push(gameBoy, getFFlagsAsByte(&gameBoy->cpu), gameBoy->cpu.a);
            return 16;
        }
        case 0xf6: {
            // OR A, u8
            if(cpuDebug()) printf("OR A, u8\n");
            or_byte(gameBoy, &gameBoy->cpu.a, readFromMemory(gameBoy, gameBoy->cpu.pc++));
            return 8;
        }
        case 0xf7: {
            // RST 30H
            if(cpuDebug()) printf("RST 30H\n");
            rst(gameBoy, 0x30);
            return 16;
        }
        case 0xf8: {
            // LD HL, SP + i8
            if(cpuDebug()) printf("LD HL, SP + i8");
            int8_t value = (int8_t) readFromMemory(gameBoy, gameBoy->cpu.pc++);
            int result = gameBoy->cpu.sp + value;
            uint16_t result16 = (uint16_t) result;
            decompose_bytes(result16, &gameBoy->cpu.l, &gameBoy->cpu.h);
            gameBoy->cpu.zero = false;
            gameBoy->cpu.subtract = false;
            gameBoy->cpu.halfCarry = (((gameBoy->cpu.sp ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
            gameBoy->cpu.carry = (((gameBoy->cpu.sp ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
            return 12;
        }
        case 0xf9: {
            // LD SP, HL
            if(cpuDebug()) printf("LD SP, HL\n");
            gameBoy->cpu.sp = compose_bytes(gameBoy->cpu.l, gameBoy->cpu.h);
            return 8;
        }
        case 0xfa: {
            // LD A, (u16)
            uint8_t lower = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint8_t upper = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            uint16_t address = compose_bytes(lower, upper);
            uint8_t value = readFromMemory(gameBoy, address);
            if(cpuDebug()) printf("LD A, (u16=%x)=%x\n", address, value);
            ld_byte(&gameBoy->cpu.a, value);
            return 16;
        }
        case 0xfb: {
            // EI
            if(cpuDebug()) printf("EI\n");
            gameBoy->cpu.pendingInterruptEnable = true;
            return 4;
        }
        case 0xfc: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION at 0xfc\n");
            return 0;
        }
        case 0xfd: {
            // Blank Instruction
            if(cpuDebug()) printf("BLANK INSTRUCTION at 0xfd\n");
            return 0;
        }
        case 0xfe: {
            // CP A, u8
            uint8_t data = readFromMemory(gameBoy, gameBoy->cpu.pc++);
            if(cpuDebug()) printf("CP A, u8(%x)\n", data);
            cp_byte(gameBoy, gameBoy->cpu.a, data);
            return 4;
        }
        case 0xff: {
            // RST 38H
            if(cpuDebug()) printf("RST 38H\n");
            rst(gameBoy, 0x38);
            return 16;
        }
        default: {
            printf("Instruction Not Found: %x\n", instruction);
            exit(1);
        }
    }
}

int updateCPU(GameBoy* gameBoy) {
    int cycles = 0;
    uint8_t instruction = fetch(gameBoy);
    if(cpuDebug()) printf("Instruction: %x\n", instruction);
    cycles += decodeAndExecute(gameBoy, instruction);
    if(gameBoy->cpu.pendingInterruptEnable) {    
        if(!gameBoy->cpu.interruptsEnabled)
            gameBoy->cpu.interruptsEnabled = true;
        gameBoy->cpu.pendingInterruptEnable = false;
    }
    return cycles;
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

void printCPU(CPU* cpu) {
    printf("halted: %s\n", cpu->halted ? "true" : "false");
    printf("Interrupts Enabled: %s\n", cpu->interruptsEnabled ? "true" : "false");
    printf("Pending Interrupt Enable: %s\n", cpu->pendingInterruptEnable ? "true" : "false");
    printf("Zero: %s\n", cpu->zero ? "true" : "false");
    printf("Subtract: %s\n", cpu->subtract ? "true" : "false");
    printf("Half Carry: %s\n", cpu->halfCarry ? "true" : "false");
    printf("Carry: %s\n", cpu->carry ? "true" : "false");
    printf("A: %x\n", cpu->a);
    printf("F: %x\n", getFFlagsAsByte(cpu));
    printf("B: %x\n", cpu->b);
    printf("C: %x\n", cpu->c);
    printf("D: %x\n", cpu->d);
    printf("E: %x\n", cpu->e);
    printf("H: %x\n", cpu->h);
    printf("L: %x\n", cpu->l);
    printf("SP: %x\n", cpu->sp);
    printf("PC: %x\n", cpu->pc);
}