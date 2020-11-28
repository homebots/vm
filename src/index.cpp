#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG

#include "index.h"
#include "homebots.h"
#include "vm.h"

static void* arguments[6];
static uint32_t slot[16];
static uint32_t programCounter = 0;
static os_timer_t cycleTimer;
static uint8_t next = 0;

#define read_argument(x) (*((volatile uint8_t *)(arguments[x])))
#define read_argument32(x) (*((volatile uint32_t *)(arguments[x])))

#define read_byte(x)      arguments[x] = (void*)readByte();
#define read_integer(x)   arguments[x] = (void*)readInt();
#define read_string(x)    arguments[x] = (void*)readString();

void reset() {
  programCounter = 0;

  int i = 0;
  while (i < 16) {
    slot[i++] = 0;
  }

  i = 0;
  while (i < 6) {
    arguments[i++] = 0;
  }
}

void ICACHE_FLASH_ATTR dump() {
  int i;
  os_printf("Slots\n");
  for (i = 0; i < 4; i++) {
    os_printf("%d: %d \n", i, slot[i]);
  }
  os_printf("\n");
}

void ICACHE_FLASH_ATTR tick() {
  os_timer_arm(&cycleTimer, 1, 0);
}

static uint8_t program[] = {
  c_push_b, slot0, 1,
  c_iomode, pin0, PinOutput,
  c_iomode, pin2, PinOutput,
  c_iowrite, pin0, slot0,
  c_iowrite, pin0, slot0,
  c_ioread, slot1, pin2,
  c_not, slot2, slot1,
  c_iowrite, pin0, slot1,
  c_iowrite, pin2, slot2,
  c_delay, 0x80, 0x01, 0x00, 0x00,
  c_jumpto, 0x0f, 0x00, 0x00, 0x00,
  c_halt
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

void ICACHE_FLASH_ATTR cycle() {
  uint32_t cycleDelay = 0;
  next = program[programCounter];

  if (next == c_halt) {
    os_printf("HALT\n");
    reset();
    return;
  }

  if (programCounter == sizeof(program)) {
    return;
  }

  switch(next) {
    case c_noop:
      LOG("NOOP\n");
      advance(1);
      break;

    case c_print:
      advance(1);
      read_string(0);
      os_printf("%s", (uint8_t*)arguments[0]);
      break;

    case c_ioread:
      advance(1);
      read_byte(0)        // slot
      read_byte(1)        // pin
      slot[read_argument(0)] = (uint32_t)vm_pinRead((uint8_t*)arguments[1]);
      LOG("[%d] = io[%d]\n", read_argument(0), read_argument(1));
      break;

    case c_iowrite:
      advance(1);
      read_byte(0)        // pin
      read_byte(1)        // value
      vm_pinWrite((uint8_t*)arguments[0], (uint8_t*)&slot[read_argument(1)]);
      LOG("io[%d] = [%d]\n", read_argument(0), read_argument(1));
      break;

    case c_memget:
      advance(1);
      read_byte(0)        // slot
      read_integer(1)     // *ptr (address)
      LOG("[%d] = @%ld\n", read_argument(0), read_argument32(1));
      slot[read_argument(0)] = *((volatile uint32_t *)(arguments[1]));
      break;

    case c_memset:
      advance(1);
      read_integer(0)     // *ptr (address)
      read_integer(1)     // slot
      LOG("@%ld = [%d]\n", read_argument32(0), read_argument(1));
      *((volatile uint32_t *)(arguments[0])) = slot[read_argument(1)];
      break;

    case c_push_b:
      advance(1);
      read_byte(0)        // slot
      read_byte(1)        // value
      LOG("[%d] = %d\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = read_argument(1);
      break;

    case c_push_i:
      advance(1);
      read_byte(0)        // slot
      read_integer(1)     // value

      LOG("[%d] = %ld\n", read_argument(0), read_argument32(1));
      slot[read_argument(0)] = read_argument32(1);
      break;

    case c_xor:
      advance(1);
      read_byte(0)        // slot X
      read_byte(1)        // slot A
      read_byte(2)        // slot B
      LOG("[%d] = [%d] ^ [%d]\n", read_argument(0), read_argument(1), read_argument(2));
      slot[read_argument(0)] = slot[read_argument(1)] ^ slot[read_argument(2)];
      break;

    case c_and:
      advance(1);
      read_byte(0)        // slot X
      read_byte(1)        // slot A
      read_byte(2)        // slot B
      LOG("[%d] = [%d] & [%d]\n", read_argument(0), read_argument(1), read_argument(2));
      slot[read_argument(0)] = slot[read_argument(1)] & slot[read_argument(2)];
      break;

    case c_or:
      advance(1);
      read_byte(0)        // slot X
      read_byte(1)        // slot A
      read_byte(2)        // slot B
      LOG("[%d] = [%d] | [%d]\n", read_argument(0), read_argument(1), read_argument(2));
      slot[read_argument(0)] = slot[read_argument(1)] | slot[read_argument(2)];
      break;

    case c_not:
      advance(1);
      read_byte(0)        // slot X
      read_byte(1)        // slot A
      LOG("[%d] = !%d\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = !slot[read_argument(1)];
      break;

    case c_if:
      advance(1);
      read_integer(0)        // slot A
      read_integer(1)        // slot B
      LOG("if [%d] = [%d]\n", read_argument(0), read_argument(1));
      slot[read_argument(0)] = !slot[read_argument(1)];
      break;

    case c_delay:
      advance(1);
      read_integer(0)      // milliseconds
      ets_memcpy(&cycleDelay, arguments[0], 4);
      LOG("delay %ld\n", cycleDelay);
      break;

    case c_jumpto:
      advance(1);
      read_integer(0)      // program address from zero
      ets_memcpy(&programCounter, arguments[0], 4);
      LOG("jump %ld\n", programCounter);
      break;

    default:
      os_printf("\n {!} Invalid opcode 0x%.2x!\n", next);
      reset();
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
