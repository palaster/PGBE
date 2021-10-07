#include "gameboy.h"
#include <time.h>

#include <SDL2/SDL.h>

#define VERTICAL_BLANK_SCAN_LINE 144
#define VERTICAL_BLANK_SCAN_LINE_MAX 153
#define SCANLINE_COUNTER_START 456

bool gameboyDebug() { return false; }

void doDMATransfer(GameBoy* gameBoy, const uint8_t value) {
    uint16_t address = ((uint16_t) value) << 8;
    for(int i = 0; i < 0xa0; i++)
        writeToMemory(gameBoy, 0xfe00 + i, readFromMemory(gameBoy, address + i));
}

void doRAMBankEnable(GameBoy* gameBoy, const uint16_t address, const uint8_t value) {
    if(gameBoy->mBC2)
        if(bit_value(address, 4) == 1)
            return;
    uint8_t testData = (value & 0xf);
    if(testData == 0xa)
        gameBoy->enableRAM = true;
    else if(testData == 0x0)
        gameBoy->enableRAM = false;
}

void doChangeLoROMBank(GameBoy* gameBoy, const uint8_t value) {
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

void doChangeHiROMBank(GameBoy* gameBoy, const uint8_t value) {
    uint8_t newValue = value & 224;
    gameBoy->currentROMBank &= 31;
    gameBoy->currentROMBank |= newValue;
    if(gameBoy->currentROMBank == 0)
        gameBoy->currentROMBank++;
}

void doRAMBankChange(GameBoy* gameBoy, const uint8_t value) {
    gameBoy->currentRAMBank = (value & 0x3);
}

void doChangeROMRAMMode(GameBoy* gameBoy, const uint8_t value) {
    uint8_t newValue = value & 0x1;
    gameBoy->romBanking = (newValue == 0);
    if(gameBoy->romBanking)
        gameBoy->currentRAMBank = 0;
}

void handleBanking(GameBoy* gameBoy, const uint16_t address, const uint8_t value) {
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

uint8_t readFromMemory(GameBoy* gameBoy, const uint16_t address) {
    if((address >= 0x4000) && (address <= 0x7fff)) {
        uint16_t newAddress = address - 0x4000;
        return gameBoy->cartridge[newAddress + (gameBoy->currentROMBank * 0x4000)];
    } else if((address >= 0xa000) && (address <= 0xbfff)) {
        uint16_t newAddress = address - 0xa000;
        return gameBoy->ramBanks[newAddress + (gameBoy->currentRAMBank * 0x2000)];
    } else if((address >= 0xfea0) && (address < 0xff00)) {
        return 0xff;
    } else if(address == 0xff00) {
        return getGamepadState(gameBoy);
    } else
        return gameBoy->rom[address];
}

void writeToMemory(GameBoy* gameBoy, const uint16_t address, const uint8_t value) {
    if(address < 0x8000) {
        handleBanking(gameBoy, address, value);
    } else if((address >= 0xa000) && (address < 0xc000)) {
        if(gameBoy->enableRAM) {
            uint16_t newAddress = address - 0xa000;
            gameBoy->ramBanks[newAddress + (gameBoy->currentRAMBank * 0x2000)] = value;
        }
    } else if((address >= 0xfea0) && (address <= 0xff00) || ((address >= 0xff4c) && (address <= 0xff7f))) {
        // RESTRICTED
    } else if((address >= 0xc000) && (address < 0xe000)) {
        gameBoy->rom[address] = value;
        if(address + 0x2000 <= 0xfdd)
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
        if(address == 0xff04)
            gameBoy->dividerCounter = 0;
        gameBoy->rom[address] = 0;
    } else if(address == 0xff46) {
        doDMATransfer(gameBoy, value);
    } else
        gameBoy->rom[address] = value;
}

void updateTimer(GameBoy* gameBoy, const int cycles) {
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

void doDividerRegister(GameBoy* gameBoy, const int cycles) {
    gameBoy->dividerCounter += cycles;
    if(gameBoy->dividerCounter >= 255) {
        gameBoy->dividerCounter = 0;
        gameBoy->rom[0xff04]++;
    }
}

void requestInterrupt(GameBoy* gameBoy, const int interrupt_id) {
    uint8_t req = readFromMemory(gameBoy, 0xff0f);
    req = set_bit(req, interrupt_id);
    writeToMemory(gameBoy, 0xff0f, req);
}

int doInterrupts(GameBoy* gameBoy) {
    uint8_t req = readFromMemory(gameBoy, 0xff0f);
    uint8_t enabled = readFromMemory(gameBoy, 0xffff);
    uint8_t potentialForInterrupts = req & enabled;
    if(potentialForInterrupts == 0)
        return 0;
    else
        gameBoy->cpu.halted = false;
    gameBoy->cpu.halted = false;
    if(gameBoy->cpu.interruptsEnabled) {
        if(req > 0)
            for(int i = 0; i < 5; i++)
                if(bit_value(req, i))
                    if(bit_value(enabled, i)) {
                        serviceInterrupt(gameBoy, i);
                        return 20;
                    }
    }
    return 0;
}

void serviceInterrupt(GameBoy* gameBoy, const int interrupt_id) {
    gameBoy->cpu.interruptsEnabled = false;
    uint8_t req = readFromMemory(gameBoy, 0xff0f);
    req = reset_bit(req, interrupt_id);
    writeToMemory(gameBoy, 0xff0f, req);

    push(gameBoy, (uint8_t) gameBoy->cpu.pc, (uint8_t) (gameBoy->cpu.pc >> 8));

    switch(interrupt_id) {
        case 0: gameBoy->cpu.pc = 0x40; break;
        case 1: gameBoy->cpu.pc = 0x48; break;
        case 2: gameBoy->cpu.pc = 0x50; break;
        case 4: gameBoy->cpu.pc = 0x60; break;
    }
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

void updateGraphics(GameBoy* gameBoy, const int cycles) {
    setLCDStatus(gameBoy);
    if(isLCDEnabled(gameBoy))
        gameBoy->scanlineCounter -= cycles;
    else
        return;

    if(gameBoy->scanlineCounter <= 0) {
        gameBoy->rom[0xff44]++;
        uint8_t currentLine = readFromMemory(gameBoy, 0xff44);
        gameBoy->scanlineCounter = SCANLINE_COUNTER_START;
        if(currentLine == VERTICAL_BLANK_SCAN_LINE) {
            requestInterrupt(gameBoy, 0);
        } else if(currentLine > VERTICAL_BLANK_SCAN_LINE_MAX) {
            gameBoy->rom[0xff44] = 0;
        } else if(currentLine < VERTICAL_BLANK_SCAN_LINE)
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

Color getColor(GameBoy* gameBoy, const uint16_t address, const uint8_t colorNum) {
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

uint8_t getGamepadState(GameBoy* gameBoy) {
    uint8_t res = gameBoy->rom[0xff00];
    res ^= 0xff;

    if(!bit_value(res, 4)) {
        uint8_t topGamepad = gameBoy->gamepadState >> 4;
        topGamepad |= 0xf0;
        res &= topGamepad;
    } else if(!bit_value(res, 5)) {
        uint8_t bottomGamepad = gameBoy->gamepadState & 0xf;
        bottomGamepad |= 0xf0;
        res &= bottomGamepad;
    }

    return res;
}

void keyPressed(GameBoy* gameBoy, const int key) {
    bool previoslyUnset = false;

    if(bit_value(gameBoy->gamepadState, key) == false)
        previoslyUnset = true;

    gameBoy->gamepadState = reset_bit(gameBoy->gamepadState, key);

    bool button = key > 3;

    uint8_t keyReq = gameBoy->rom[0xff00];
    bool shouldRequestInterrupt = false;

    if(button && !bit_value(keyReq, 5))
        shouldRequestInterrupt = true;
    else if(!button && !bit_value(keyReq, 4))
        shouldRequestInterrupt = true;

    if(shouldRequestInterrupt && !previoslyUnset)
        requestInterrupt(gameBoy, 4);
}

void keyReleased(GameBoy* gameBoy, const int key) { gameBoy->gamepadState = set_bit(gameBoy->gamepadState, key); }

int main(int argc, char *argv[]) {
    if(argc != 2)
        return 1;

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
    gameBoy.dividerCounter = 0;
    gameBoy.romBanking = false;
    gameBoy.enableRAM = false;
    gameBoy.mBC1 = false;
    gameBoy.mBC2 = false;
    gameBoy.haltBug = false;
    gameBoy.gamepadState = 0xff;
    gameBoy.currentROMBank = 1;
    gameBoy.currentRAMBank = 0;
    
    memset(gameBoy.ramBanks, 0, sizeof(gameBoy.ramBanks));
    memset(gameBoy.cartridge, 0, sizeof(gameBoy.cartridge));
    memset(gameBoy.rom, 0, sizeof(gameBoy.rom));
    memset(gameBoy.screenData, 0, sizeof(gameBoy.screenData));
    
    CPU cpu;

    cpu.halted = false;
    cpu.interruptsEnabled = false;
    cpu.pendingInterruptEnable = false;
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

    FILE* gameFile = fopen(argv[1], "rb");
    fread(gameBoy.cartridge, 0x2000000, 1, gameFile);
    fclose(gameFile);

    memcpy(gameBoy.rom, gameBoy.cartridge, 0x8000);

    switch(gameBoy.cartridge[0x147]) {
        case 1: gameBoy.mBC1 = true; break;
        case 2: gameBoy.mBC1 = true; break;
        case 3: gameBoy.mBC1 = true; break;
        case 5: gameBoy.mBC2 = true; break;
        case 6: gameBoy.mBC2 = true; break;
        default: break;
    }

    // 4.194304 MHz = 4194304 cycles per second
    // 59.727500569606 Hz = 59.727500569606 Frames per second
    // ~70224 cycles to 1 frame
    //uint32_t cyclesPerFrame = 4194304 / 59.727500569606;
    // Time between frames = 16.7427062988 milliseconds

    // START TESTING SECTION
    /*
    FILE* log = fopen("log.txt", "ab+");
    */
    bool willRunUntilPC = false;
    int pcToRunTo = 0x0;

    if(gameboyDebug()) {
        printCPU(&gameBoy.cpu);
        printf("PRESS ENTER TO CONTINUE or PC to run to\n");
        char test[80];
        fgets(test, sizeof test, stdin);
        if(strlen(test) > 0 && test[0] != '\0') {
            sscanf(test, "%x", &pcToRunTo);
            if(pcToRunTo > 0x0)
                willRunUntilPC = true;
        }
    }
    // END TESTING SECTION

    bool shouldClose = false;

    while(!shouldClose) {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0) {
            switch(e.type) {
                case  SDL_QUIT: {
                    shouldClose = true;
                    break;
                }
                case SDL_KEYDOWN: {
                    if(e.key.repeat) break;
                    int key = -1;
                    switch(e.key.keysym.sym) {
                        case SDLK_w: key = 2; break; // UP
                        case SDLK_a: key = 1; break; // Left
                        case SDLK_s: key = 3; break; // Down
                        case SDLK_d: key = 0; break; // Right
                        case SDLK_h: key = 4; break; // B
                        case SDLK_u: key = 5; break; // A
                        case SDLK_b: key = 7; break; // Select
                        case SDLK_n: key = 6; break; // Start
                    }
                    if(key >= 0)
                        keyPressed(&gameBoy, key);
                    break;
                }
                case SDL_KEYUP: {
                    if(e.key.repeat) break;
                    int key = -1;
                    switch(e.key.keysym.sym) {
                        case SDLK_w: key = 2; break; // UP
                        case SDLK_a: key = 1; break; // Left
                        case SDLK_s: key = 3; break; // Down
                        case SDLK_d: key = 0; break; // Right
                        case SDLK_h: key = 4; break; // B
                        case SDLK_u: key = 5; break; // A
                        case SDLK_b: key = 7; break; // Select
                        case SDLK_n: key = 6; break; // Start
                    }
                    if(key != -1)
                        keyReleased(&gameBoy, key);
                    break;
                }
            }
        }

        clock_t startTime = clock();
        int cyclesThisFrame = 0;
        while(cyclesThisFrame <= CYCLES_PER_FRAME) {
            int cycles = 4;
            if(!gameBoy.cpu.halted)
                cycles = updateCPU(&gameBoy) * 4;
            // START TESTING SECTION
            if(gameboyDebug()) {
                if(gameBoy.rom[0xff02] == 0x81) {
                    char c = gameBoy.rom[0xff01];
                    printf("%c", c);
                    gameBoy.rom[0xff02] = 0x0;
                }
                if(willRunUntilPC) {
                    if(gameBoy.cpu.pc == pcToRunTo) {
                        willRunUntilPC = false;
                        pcToRunTo = 0x0;
                    }
                }
                if(!willRunUntilPC) {
                    printCPU(&gameBoy.cpu);
                    printf("PRESS ENTER TO CONTINUE or PC to run to\n");
                    char test[80];
                    fgets(test, sizeof test, stdin);
                    if(strlen(test) > 0 && test[0] != '\0') {
                        sscanf(test, "%x", &pcToRunTo);
                        if(pcToRunTo > 0x0)
                            willRunUntilPC = true;
                    }
                }
            }
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
        if(elapsedTime <= TIME_BETWEEN_FRAMES_IN_NANOSECONDS) {
            struct timespec remainingTime = { 0 , TIME_BETWEEN_FRAMES_IN_NANOSECONDS - elapsedTime };
            nanosleep(&remainingTime, NULL);
        }
    }

    //SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 0;
}