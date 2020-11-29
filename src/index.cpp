#ifdef __cplusplus
extern "C" {
#endif

// #define DEBUG

#include "index.h"

static void* arguments[maxArguments];
static uint32_t slot[maxSlots];
static uint32_t programCounter = 0;
static os_timer_t cycleTimer;
static uint8_t next = 0;

#define read_argument(index) (*((volatile uint8_t *)(arguments[index])))
#define read_argument32(index) (*((volatile uint32_t *)(arguments[index])))

#define arg_byte(index)      arguments[index] = (void*)readByte();
#define arg_integer(index)   arguments[index] = (void*)readInt();
#define arg_string(index)    arguments[index] = (void*)readString();

void reset() {
  programCounter = 0;

  int i = 0;
  while (i < maxSlots) {
    slot[i++] = 0;
  }

  i = 0;
  while (i < maxArguments) {
    arguments[i++] = 0;
  }
}

void ICACHE_FLASH_ATTR dump() {
  int i;
  LOG("Slots\n");
  for (i = 0; i < maxSlots; i++) {
    LOG("[%d]: %d\n", i, slot[i]);
  }
  LOG("\n");
}

void ICACHE_FLASH_ATTR tick() {
  os_timer_arm(&cycleTimer, 1, 0);
}

static uint8_t program[] = {
  c_push_b,
    slot1,
    7,
  c_not,
    slot0,
    slot0,
  c_iowrite,
    pin0,
    slot0,
  c_iowrite,
    pin2,
    slot0,
  c_dec,
    slot1,
  c_delay,
    0x40,
    0x00,
    0x00,
    0x00,
  c_jumpif,
    slot1,
    0x03,
    0x00,
    0x00,
    0x00,
  c_delay,
    0xe8,
    0x03,
    0x00,
    0x00,
  c_jump,
    0x00,
    0x00,
    0x00,
    0x00,
};

void ICACHE_FLASH_ATTR advance(int howMuch) {
  programCounter += howMuch;
}

uint8_t* readByte() {
  uint8_t* pointer = &(program[programCounter]);
  advance(1);

  return pointer;
}

uint32_t* readInt() {
  uint32_t* pointer = (uint32_t*)&(program[programCounter]);
  advance(4);

  return pointer;
}

uint32_t* readString() {
  uint32_t* pointer = (uint32_t*)&(program[programCounter]);

  while (program[programCounter] != '\0') {
    advance(1);
  }

  advance(1);

  return pointer;
}

uint32_t readStringLength() {
  uint32_t cursor = 0;
  while (program[programCounter + cursor] != '\0') {
    cursor++;
  }

  return cursor + 1;
}

uint8_t instructionArgumentsLength(uint8_t instruction) {
  switch (instruction) {
    case c_noop:
    case c_halt:
      return 0;

    case c_print:
      return readStringLength() - 1;

    case c_inc:
    case c_dec:
      return 1;

    case c_ioread:
    case c_iowrite:
    case c_iomode:
    case c_push_b:
    case c_not:
    case c_add:
    case c_rm:
      return 2;

    case c_xor:
    case c_and:
    case c_or:
    case c_gt:
    case c_gte:
    case c_lt:
    case c_lte:
    case c_equal:
    case c_notequal:
      return 3;

    case c_delay:
    case c_jump:
      return 4;

    case c_memget:
    case c_memset:
    case c_push_i:
    case c_jumpif:
      return 5;
  }
}

void ICACHE_FLASH_ATTR cycle() {
  uint32_t cycleDelay = 0;
  bool skipNext = false;
  next = program[programCounter];

  if (next == c_halt) {
    LOG("halt\n");
    reset();
    return;
  }

  advance(1);

  switch(next) {
    case c_noop:
      LOG("NOOP\n");
      break;

    case c_print:
      arg_string(0);
      LOG("print '...'");
      os_printf("%s", (uint8_t*)arguments[0]);
      break;

    case c_ioread:
      arg_byte(0)        // slot
      arg_byte(1)        // pin
      LOG("ioread [%d], %d\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = (uint32_t)pinRead(read_argument(1));
      break;

    case c_iowrite:
    case c_iomode:
    case c_iotype:
      arg_byte(0)        // pin
      arg_byte(1)        // slot

      switch (next) {
        case c_iowrite:
          LOG("iowrite %d, [%d]\n", read_argument(0), read_argument(1));
          pinWrite(read_argument(0), (bool)slot[read_argument(1)]);
          break;

        case c_iomode:
          LOG("iomode %d, [%d]\n", read_argument(0), read_argument(1));
          pinMode(read_argument(0), (PinMode)slot[read_argument(1)]);
          break;

        case c_iotype:
          LOG("iotype %d, [%d]\n", read_argument(0), read_argument(1));
          pinType(read_argument(0), (uint8_t)slot[read_argument(1)]);
          break;
      }
      break;

    case c_memget:
      arg_byte(0)        // slot
      arg_integer(1)     // *ptr (address)
      LOG("memget [%d], *%ld\n", read_argument(0), read_argument32(1));
      slot[read_argument(0)] = *((volatile uint32_t *)(arguments[1]));
      break;

    case c_memset:
      arg_integer(0)     // *ptr (address)
      arg_byte(1)        // slot
      LOG("memset @%ld, [%d]\n", read_argument32(0), read_argument(1));
      *((volatile uint32_t *)(arguments[0])) = slot[read_argument(1)];
      break;

    case c_push_b:
      arg_byte(0)        // slot
      arg_byte(1)        // byte
      LOG("push_b [%d], %d\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = read_argument(1);
      break;

    case c_push_i:
      arg_byte(0)        // slot
      arg_integer(1)     // int

      LOG("push_i [%d], %ld\n", read_argument(0), read_argument32(1));
      slot[read_argument(0)] = read_argument32(1);
      break;

    case c_xor:
    case c_and:
    case c_or:
      arg_byte(0)        // slot X
      arg_byte(1)        // slot A
      arg_byte(2)        // slot B

      switch (next) {
        case c_xor:
          LOG("xor [%d], [%d], [%d]\n", read_argument(0), read_argument(1), read_argument(2));
          slot[read_argument(0)] = slot[read_argument(1)] ^ slot[read_argument(2)];
          break;

        case c_and:
          LOG("and [%d], [%d], [%d]\n", read_argument(0), read_argument(1), read_argument(2));
          slot[read_argument(0)] = slot[read_argument(1)] & slot[read_argument(2)];
          break;

        case c_or:
          LOG("or [%d], [%d], [%d]\n", read_argument(0), read_argument(1), read_argument(2));
          slot[read_argument(0)] = slot[read_argument(1)] | slot[read_argument(2)];
          break;
      }
      break;

    case c_not:
      arg_byte(0)        // slot X
      arg_byte(1)        // slot A
      LOG("not [%d], [%d]\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = !slot[read_argument(1)];
      break;

    case c_inc:
      arg_byte(0)        // slot
      LOG("inc [%d]\n", read_argument(0));
      slot[read_argument(0)]++;
      break;

    case c_dec:
      arg_byte(0)        // slot
      LOG("dec [%d]\n", read_argument(0));
      slot[read_argument(0)]--;
      break;

    case c_add:
      arg_byte(0)        // slot X
      arg_byte(1)        // slot A
      LOG("add [%d], [%d]\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] += slot[read_argument(1)];
      break;

    case c_rm:
      arg_byte(0)        // slot X
      arg_byte(1)        // slot A
      LOG("rm [%d], [%d]\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] -= slot[read_argument(1)];
      break;

    case c_gt:
    case c_lt:
    case c_gte:
    case c_lte:
    case c_equal:
      arg_integer(0)        // slot X
      arg_integer(1)        // slot B
      arg_integer(2)        // slot C

      switch (next) {
        case c_gt:
          LOG("gt [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] > slot[read_argument(2)];
          break;

        case c_lt:
          LOG("lt [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] < slot[read_argument(2)];
          break;

        case c_gte:
          LOG("gte [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] >= slot[read_argument(2)];
          break;

        case c_lte:
          LOG("lte [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] <= slot[read_argument(2)];
          break;

        case c_equal:
          LOG("equal [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] == slot[read_argument(2)];

        case c_notequal:
          LOG("notequal [%d], [%d], [%d]\n", read_argument(0), read_argument(1));
          slot[read_argument(0)] = slot[read_argument(1)] != slot[read_argument(2)];
          break;
      }
      break;

    case c_delay:
      arg_integer(0)      // milliseconds
      ets_memcpy(&cycleDelay, arguments[0], 4);

      if (cycleDelay > MAX_DELAY) {
        cycleDelay = MAX_DELAY;
      }

      LOG("delay %ld\n", cycleDelay);
      break;

    case c_jump:
      arg_integer(0)      // program address from zero
      ets_memcpy(&programCounter, arguments[0], 4);
      LOG("jump %ld\n", programCounter);
      break;

    case c_jumpif:
      arg_byte(0)         // slot
      arg_integer(1)      // program address from zero

      if (slot[read_argument(0)] != 0) {
        ets_memcpy(&programCounter, arguments[1], 4);
        LOG("jumpif [%d], %ld\n", read_argument(0), programCounter);
      }
      break;

    default:
      os_printf("\n {!} Invalid opcode 0x%.2x!\n", next);
      reset();
  }

  if (skipNext) {
    programCounter += instructionArgumentsLength(next);
  }

  os_timer_arm(&cycleTimer, cycleDelay, 0);
}

void ICACHE_FLASH_ATTR setup() {
  #ifndef DEBUG
  system_uart_swap();
  #endif

  system_update_cpu_freq(SYS_CPU_160MHZ);
  reset();

  os_timer_setfn(&cycleTimer, (os_timer_func_t*)&cycle, NULL);
  os_timer_arm(&cycleTimer, 1, 0);
}

#ifdef __cplusplus
}
#endif
