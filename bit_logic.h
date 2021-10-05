#pragma once

#include <stdint.h>

// BIT CHECKING
// FIRST METHOD byte, bitPosition = byte & (1 << bitPosition)
// SECOND METHOD byte, bitPosition = (byte >> bitposition) & 1

static bool check_bit(uint8_t byte, uint8_t position) { return (byte & (1 << position)) != 0; } // aka GetVal
//static bool check_bit(uint8_t byte, uint8_t position) { return byte & (1 << position); } // aka GetVal
static uint8_t bit_value(uint8_t byte, uint8_t position) { return (byte >> position) & 1; } // aka TestBit
static uint8_t set_bit(uint8_t byte, uint8_t position) { return byte | (1 << position); }
static uint8_t reset_bit(uint8_t byte, uint8_t position) { return byte & (~(1 << position)); }
static uint8_t set_bit_to(uint8_t byte, uint8_t position, bool on) { return on ? set_bit(byte, position) : reset_bit(byte, position); }

// Make 16 bit from two 8 bits = ((uint16_t) upper << 8) | lower
// Make two 8 bits from 16 bit = upper = (uint8_t) (word >> 8)
//                               lower = (uint8_t) word

static uint16_t compose_bytes(uint8_t lower, uint8_t upper) {
    uint16_t upper16 = (uint16_t) upper;
    upper16 <<= 8;
    return (upper16 | lower);
}

static void decompose_bytes(uint16_t bytes, uint8_t* lower, uint8_t* upper) {
    *lower = (uint8_t) bytes;
    *upper = (uint8_t) (bytes >> 8);
}