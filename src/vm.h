#define USE_US_TIMER 1
#define MAX_ARGUMENTS 6
#define MAX_SLOTS 16
#define MAX_DELAY 6871000

#include "sdk/sdk.h"

#define arg_byte(vm, index) vm->arguments[index] = (void *)vm_readByte(vm);
#define arg_integer(vm, index) vm->arguments[index] = (void *)vm_readInt(vm);
#define arg_double(vm, index) vm->arguments[index] = (void *)vm_readDouble(vm);
#define arg_string(vm, index) vm->arguments[index] = (void *)vm_readString(vm);

#define copy_int32(dest, src) ets_memcpy(dest, src, 4);
#define copy_int64(dest, src) ets_memcpy(dest, src, 8);

const uint8_t c_halt = 0xfe;           // -
const uint8_t c_sysinfo = 0xfd;        // -
const uint8_t c_restart = 0xfc;        // -
const uint8_t c_debug = 0xfb;          // -
const uint8_t c_yield = 0xfa;          // -
const uint8_t c_dump = 0xf9;           // -
const uint8_t c_noop = 0x01;           // -
const uint8_t c_delay = 0x02;          // uint32 delay
const uint8_t c_print = 0x03;          // uint8[] message
const uint8_t c_jump = 0x04;           // uint32 address
const uint8_t c_memget = 0x05;         // uint8 slot, int32 address
const uint8_t c_memset = 0x06;         // int32 address, uint8 slot
const uint8_t c_push_b = 0x07;         // uint8 slot, int8 value
const uint8_t c_push_i = 0x08;         // uint8 slot, int32 value
const uint8_t c_gt = 0x09;             // uint8 slot, uint8 b, uint8 b
const uint8_t c_gte = 0x0a;            // uint8 slot, uint8 b, uint8 b
const uint8_t c_lt = 0x0b;             // uint8 slot, uint8 b, uint8 b
const uint8_t c_lte = 0x0c;            // uint8 slot, uint8 b, uint8 b
const uint8_t c_equal = 0x0d;          // uint8 slot, uint8 b, uint8 b
const uint8_t c_notequal = 0x0e;       // uint8 slot, uint8 b, uint8 b
const uint8_t c_jumpif = 0x0f;         // uint8 slot, uint32 address
const uint8_t c_xor = 0x10;            // uint8 slot, uint8 a, uint8 b
const uint8_t c_and = 0x11;            // uint8 slot, uint8 a, uint8 b
const uint8_t c_or = 0x12;             // uint8 slot, uint8 a, uint8 b
const uint8_t c_not = 0x13;            // uint8 slot
const uint8_t c_inc = 0x14;            // uint8 slot
const uint8_t c_dec = 0x15;            // uint8 slot
const uint8_t c_add = 0x16;            // uint8 slot, uint8 slot
const uint8_t c_sub = 0x17;            // uint8 slot, uint8 slot
const uint8_t c_mul = 0x18;            // uint8 slot, uint8 slot
const uint8_t c_div = 0x19;            // uint8 slot, uint8 slot
const uint8_t c_mod = 0x1a;            // uint8 slot, uint8 slot
const uint8_t c_copy = 0x1b;           // uint8 dest, uint8 src
const uint8_t c_delay_v = 0x1c;        // uint8 slot
const uint8_t c_iowrite = 0x31;        // uint8 pin, uint8 slot
const uint8_t c_ioread = 0x32;         // uint8 slot, uint8 pin
const uint8_t c_iomode = 0x35;         // uint8 pin, uint8 slot
const uint8_t c_iotype = 0x36;         // uint8 pin, uint8 slot
const uint8_t c_ioallout = 0x37;       // -
const uint8_t c_wificonnect = 0x3a;    // uint8 pin, uint8 slot
const uint8_t c_wifidisconnect = 0x3b; // uint8 pin, uint8 slot
const uint8_t c_wifistatus = 0x3c;     // uint8 pin, uint8 slot
const uint8_t c_wifilist = 0x3e;       // uint8 pin, uint8 slot
const uint8_t c_sleep = 0x3f;          // uint8 mode, uint32 time
const uint8_t c_i2setup = 0x40;        // -
const uint8_t c_i2start = 0x41;        // -
const uint8_t c_i2stop = 0x42;         // -
const uint8_t c_i2write = 0x43;        // uint8 byte
const uint8_t c_i2read = 0x44;         // uint8 slot
const uint8_t c_i2setack = 0x45;       // uint8 byte
const uint8_t c_i2getack = 0x46;       // uint8 byte
const uint8_t c_i2find = 0x48;         // uint8 slot
const uint8_t c_i2writeack = 0x49;     // uint32 length, uint* bytes
const uint8_t c_i2writeack_b = 0x4a;   // uint8 byte

typedef struct
{
  uint32_t programCounter = 0;
  void *arguments[MAX_ARGUMENTS];
  void *pointer = 0;
  uint32_t slot[MAX_SLOTS];
  uint8_t *program = NULL;
  int length;
  os_timer_t cycleTimer;
} EspVM;

static Wifi wifiConnection;

uint8_t read_argument(EspVM *vm, uint8_t index)
{
  return *((volatile uint8_t *)(vm->arguments[index]));
}

uint32_t read_argument32(EspVM *vm, uint8_t index)
{
  return (uint32_t)((((uint8_t *)vm->arguments[index])[3]) & 0xff) << 24 |
         (uint32_t)((((uint8_t *)vm->arguments[index])[2]) & 0xff) << 16 |
         (uint32_t)((((uint8_t *)vm->arguments[index])[1]) & 0xff) << 8 |
         (uint32_t)((((uint8_t *)vm->arguments[index])[0]) & 0xff);
}

bool vm_isWifiConnected()
{
  return wifiConnection.isConnected();
}

void vm_reset(EspVM *vm)
{
  vm->programCounter = 0;

  int i = 0;
  while (i < MAX_SLOTS)
  {
    vm->slot[i++] = 0;
  }

  i = 0;
  while (i < MAX_ARGUMENTS)
  {
    vm->arguments[i++] = 0;
  }
}

void ICACHE_FLASH_ATTR vm_advance(EspVM *vm, uint32_t howMuch)
{
  vm->programCounter += howMuch;
}

uint8_t *vm_readByte(EspVM *vm)
{
  vm->pointer = &(vm->program[vm->programCounter]);
  vm_advance(vm, 1);

  return (uint8_t *)vm->pointer;
}

uint32_t *vm_readInt(EspVM *vm)
{
  uint32_t *pointer = (uint32_t *)&(vm->program[vm->programCounter]);
  vm_advance(vm, 4);

  return pointer;
}

uint32_t *vm_readDouble(EspVM *vm)
{
  uint32_t *pointer = (uint32_t *)&(vm->program[vm->programCounter]);
  vm_advance(vm, 8);

  return pointer;
}

uint32_t *vm_readString(EspVM *vm)
{
  uint32_t *pointer = (uint32_t *)&(vm->program[vm->programCounter]);

  while (vm->program[vm->programCounter] != '\0')
  {
    vm_advance(vm, 1);
  }

  vm_advance(vm, 1);

  return pointer;
}

uint32_t vm_readStringLength(EspVM *vm)
{
  uint32_t cursor = 0;
  while (vm->program[vm->programCounter + cursor] != '\0')
  {
    cursor++;
  }

  return cursor + 1;
}

void ICACHE_FLASH_ATTR vm_dump(EspVM *vm)
{
  int i;
  LOG("PC %ld\n", vm->programCounter);
  LOG("Slots\n");

  for (i = 0; i < MAX_SLOTS; i++)
  {
    LOG("[%d]: %d\n", i, vm->slot[i]);
  }

  LOG("\n");
}

void ICACHE_FLASH_ATTR vm_dump_program(EspVM *vm)
{
  int i = 0;
  while (i < vm->length)
  {
    os_printf("0x%.2x ", vm->program[i++]);
  }
}

void ICACHE_FLASH_ATTR vm_yield(EspVM *vm)
{
  os_timer_arm(&vm->cycleTimer, 1, 0);
}

void ICACHE_FLASH_ATTR vm_tick(EspVM *vm, uint32_t delay)
{
  os_timer_arm(&vm->cycleTimer, delay, 0);
}

void ICACHE_FLASH_ATTR vm_cycle(EspVM *vm)
{
  uint8_t next = vm->program[vm->programCounter];
  uint32_t cycleDelay = 0;

  if (next == c_halt)
  {
    LOG("halt\n");
    vm_advance(vm, 1);
    return;
  }

  if (vm->programCounter >= vm->length - 1)
  {
    return;
  }

  vm_advance(vm, 1);

  switch (next)
  {
  case c_noop:
    LOG("NOOP\n");
    break;

  case c_yield:
    vm_yield(vm);
    return;

  case c_debug:
    arg_byte(vm, 0);

    if (read_argument(vm, 0))
    {
      system_uart_de_swap();
    }
    else
    {
      system_uart_swap();
    }
    break;

  case c_sysinfo:
    os_printf("Chip %ld\n", system_get_chip_id());
    os_printf("SDK %s\n", system_get_sdk_version());
    os_printf("Time %ld\n", system_get_time() / 1000);
    system_print_meminfo();
    os_printf("Free %ld bytes\n", system_get_free_heap_size());
    break;

  case c_restart:
    system_restart();
    break;

  case c_sleep:
    arg_byte(vm, 0);
    arg_integer(vm, 1);
    system_deep_sleep_set_option(2);
    system_deep_sleep((uint64_t)read_argument32(vm, 1));
    break;

  case c_wificonnect:
    arg_string(vm, 0);
    arg_string(vm, 1);
    wifiConnection.connectTo((const char *)vm->arguments[0], (const char *)vm->arguments[1]);
    break;

  case c_wifidisconnect:
    wifiConnection.disconnect();
    break;

  case c_wifistatus:
    arg_byte(vm, 0);
    vm->slot[read_argument(vm, 0)] = wifiConnection.getStatus();

  case c_print:
    arg_string(vm, 0);
    LOG("print '...'");
    os_printf("%s", (uint8_t *)vm->arguments[0]);
    break;

  case c_ioread:
    arg_byte(vm, 0); // slot
    arg_byte(vm, 1); // pin
    LOG("ioread [%d], %d\n", read_argument(vm, 0), read_argument(vm, 1));
    vm->slot[read_argument(vm, 0)] = (uint32_t)pinRead(read_argument(vm, 1));
    break;

  case c_iowrite:
  case c_iomode:
  case c_iotype:
    arg_byte(vm, 0); // pin
    arg_byte(vm, 1); // slot

    switch (next)
    {
    case c_iowrite:
      LOG("iowrite %d, [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      pinWrite(read_argument(vm, 0), (bool)vm->slot[read_argument(vm, 1)]);
      break;

    case c_iomode:
      LOG("iomode %d, [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      pinMode(read_argument(vm, 0), (PinMode)vm->slot[read_argument(vm, 1)]);
      break;

    case c_iotype:
      LOG("iotype %d, [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      pinType(read_argument(vm, 0), (uint8_t)vm->slot[read_argument(vm, 1)]);
      break;
    }
    break;

  case c_ioallout:
    pinType(0, 0);
    pinType(1, 3);
    pinType(2, 0);
    pinType(3, 3);
    pinMode(0, PinOutput);
    pinMode(1, PinOutput);
    pinMode(2, PinOutput);
    pinMode(3, PinOutput);
    break;

  case c_memget:
    arg_byte(vm, 0);    // slot
    arg_integer(vm, 1); // *ptr (address)

    LOG("memget [%d], %ld\n", read_argument(vm, 0), read_argument32(vm, 1));
    vm->slot[read_argument(vm, 0)] = READ_PERI_REG(read_argument32(vm, 1));
    break;

  case c_memset:
    arg_integer(vm, 0); // *ptr (address)
    arg_byte(vm, 1);    // slot

    LOG("memset %ld, [%d]\n", read_argument32(vm, 0), read_argument(vm, 1));
    WRITE_PERI_REG(read_argument32(vm, 0), vm->slot[read_argument(vm, 1)]);
    break;

  case c_push_b:
    arg_byte(vm, 0); // slot
    arg_byte(vm, 1); // byte
    LOG("push_b [%d], %d\n", read_argument(vm, 0), read_argument(vm, 1));
    vm->slot[read_argument(vm, 0)] = read_argument(vm, 1);
    break;

  case c_push_i:
    arg_byte(vm, 0);    // slot
    arg_integer(vm, 1); // int

    LOG("push_i [%d], %ld\n", read_argument(vm, 0), read_argument32(vm, 1));
    vm->slot[read_argument(vm, 0)] = read_argument32(vm, 1);
    break;

  case c_copy:
    arg_byte(vm, 0); // slot
    arg_byte(vm, 1); // slot

    LOG("copy [%d], %ld\n", read_argument(vm, 0), read_argument(vm, 1));
    vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)];
    break;

  case c_xor:
  case c_and:
  case c_or:
    arg_byte(vm, 0); // slot X
    arg_byte(vm, 1); // slot A
    arg_byte(vm, 2); // slot B

    switch (next)
    {
    case c_xor:
      LOG("xor [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1), read_argument(vm, 2));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] ^ vm->slot[read_argument(vm, 2)];
      break;

    case c_and:
      LOG("and [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1), read_argument(vm, 2));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] & vm->slot[read_argument(vm, 2)];
      break;

    case c_or:
      LOG("or [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1), read_argument(vm, 2));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] | vm->slot[read_argument(vm, 2)];
      break;
    }
    break;

  case c_not:
    arg_byte(vm, 0); // slot
    LOG("not [%d]\n", read_argument(vm, 0));
    vm->slot[read_argument(vm, 0)] = !vm->slot[read_argument(vm, 0)];
    break;

  case c_inc:
    arg_byte(vm, 0); // slot
    LOG("inc [%d]\n", read_argument(vm, 0));
    vm->slot[read_argument(vm, 0)]++;
    break;

  case c_dec:
    arg_byte(vm, 0); // slot
    LOG("dec [%d]\n", read_argument(vm, 0));
    vm->slot[read_argument(vm, 0)]--;
    break;

  case c_add:
    arg_byte(vm, 0); // slot X
    arg_byte(vm, 1); // slot A
    LOG("add [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
    vm->slot[read_argument(vm, 0)] += vm->slot[read_argument(vm, 1)];
    break;

  case c_sub:
    arg_byte(vm, 0); // slot X
    arg_byte(vm, 1); // slot A
    LOG("sub [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
    vm->slot[read_argument(vm, 0)] -= vm->slot[read_argument(vm, 1)];
    break;

  case c_gt:
  case c_lt:
  case c_gte:
  case c_lte:
  case c_equal:
    arg_integer(vm, 0); // slot X
    arg_integer(vm, 1); // slot B
    arg_integer(vm, 2); // slot C

    switch (next)
    {
    case c_gt:
      LOG("gt [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] > vm->slot[read_argument(vm, 2)];
      break;

    case c_lt:
      LOG("lt [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] < vm->slot[read_argument(vm, 2)];
      break;

    case c_gte:
      LOG("gte [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] >= vm->slot[read_argument(vm, 2)];
      break;

    case c_lte:
      LOG("lte [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] <= vm->slot[read_argument(vm, 2)];
      break;

    case c_equal:
      LOG("equal [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] == vm->slot[read_argument(vm, 2)];

    case c_notequal:
      LOG("notequal [%d], [%d], [%d]\n", read_argument(vm, 0), read_argument(vm, 1));
      vm->slot[read_argument(vm, 0)] = vm->slot[read_argument(vm, 1)] != vm->slot[read_argument(vm, 2)];
      break;
    }
    break;

  case c_delay:
    arg_integer(vm, 0); // milliseconds
    copy_int32(&cycleDelay, vm->arguments[0]);

    if (cycleDelay > MAX_DELAY)
    {
      cycleDelay = MAX_DELAY;
    }

    LOG("delay %ld\n", cycleDelay);
    break;

  case c_delay_v:
    arg_byte(vm, 0); // slot
    cycleDelay = vm->slot[read_argument(vm, 0)];

    if (cycleDelay > MAX_DELAY)
    {
      cycleDelay = MAX_DELAY;
    }

    LOG("delay %ld\n", cycleDelay);
    break;

  case c_jump:
    arg_integer(vm, 0); // program address from zero
    copy_int32(&vm->programCounter, vm->arguments[0]);
    LOG("jump %ld\n", vm->programCounter);
    break;

  case c_jumpif:
    arg_byte(vm, 0);    // slot
    arg_integer(vm, 1); // program address from zero

    if (vm->slot[read_argument(vm, 0)] != 0)
    {
      copy_int32(&vm->programCounter, vm->arguments[1]);
      LOG("jumpif [%d], %ld\n", read_argument(vm, 0), vm->programCounter);
    }
    break;

  case c_i2setup:
    LOG("i2setup\n");
    i2c_gpio_init();
    break;

  case c_i2start:
    LOG("i2start\n");
    i2c_start();
    break;

  case c_i2stop:
    LOG("i2stop\n");
    i2c_stop();
    break;

  case c_i2write:
    arg_byte(vm, 0); // slot

    LOG("i2write [%d]\n", read_argument(vm, 0));
    i2c_writeByte(vm->slot[read_argument(vm, 0)]);
    break;

  case c_i2writeack:
    int length;
    arg_integer(vm, 0); // length
    copy_int32(&length, vm->arguments[0]);

    LOG("i2writeack %d bytes\n", length);
    while (length--)
    {
      arg_byte(vm, 1);
      LOG("0x%.2x ", read_argument(vm, 1));
      i2c_writeByteAndAck(read_argument(vm, 1));
    }
    LOG("\n");
    break;

  case c_i2writeack_b:
    arg_byte(vm, 0); // byte
    LOG("i2writeack_b 0x%.2x\n", read_argument(vm, 0));
    i2c_writeByteAndAck(read_argument(vm, 0));
    break;

  case c_i2read:
    arg_byte(vm, 0); // slot

    vm->slot[read_argument(vm, 0)] = i2c_readByte();
    LOG("i2read [%d], 0x%.2x\n", read_argument(vm, 0), vm->slot[read_argument(vm, 0)]);
    break;

  case c_i2setack:
    arg_byte(vm, 0);
    LOG("i2setack %d\n", read_argument(vm, 0));
    i2c_setAck(read_argument(vm, 0));
    break;

  case c_i2getack:
    arg_byte(vm, 0);
    LOG("i2getack %d\n", read_argument(vm, 0));
    vm->slot[read_argument(vm, 0)] = i2c_getAck();
    break;

  case c_i2find:
    arg_byte(vm, 0);
    LOG("i2find %d\n", read_argument(vm, 0));
    vm->slot[read_argument(vm, 0)] = i2c_findDevice();
    break;

  case c_dump:
    vm_dump(vm);
    vm_dump_program(vm);
    break;

  default:
    os_printf("\n {!} Invalid opcode 0x%.2x at %d!\n", next, vm->programCounter);
    vm_dump(vm);
    vm_dump_program(vm);
    return;
  }

  vm_tick(vm, cycleDelay);
}

void vm_init(EspVM *vm)
{
  os_timer_setfn(&vm->cycleTimer, (os_timer_func_t *)&vm_cycle, vm);
  vm_reset(vm);
}

void vm_load(EspVM *vm, uint8_t *program, int length)
{
  if (vm->program != NULL)
  {
    os_free(vm->program);
  }

  vm->program = (uint8_t *)os_zalloc(length);
  vm->length = length;
  os_memcpy(vm->program, program, length);

  vm_reset(vm);
  vm_yield(vm);
}
