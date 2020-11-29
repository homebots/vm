#define SERIAL_SPEED      115200
#define MAX_DELAY         6871000

#ifdef __cplusplus
extern "C" {
#endif

#include "homebots.h"
#include "vm.h"

void setup();
void cycle();
void advance(int howMuch);
void reset();
void dump();
void tick();

uint8_t* readByte();
uint32_t* readInt();
uint32_t* readString();
uint32_t readStringLength();
uint8_t instructionArgumentsLength(uint8_t instruction);

#ifdef __cplusplus
}
#endif
