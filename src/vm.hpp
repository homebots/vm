#include "sdk.h"

#define MAX_DELAY 6871000
#define MAX_SLOTS 256
#define MAJOR 1
#define MINOR 0

typedef unsigned char *string;

typedef unsigned char byte;
typedef unsigned char *byteref;

typedef unsigned int uint;
typedef unsigned int *uintref;

// system instructions [0x01..0x1f]
const byte op_noop = 0x01;
const byte op_halt = 0x02;
const byte op_restart = 0x03;
const byte op_systeminfo = 0x04;
const byte op_debug = 0x05;
const byte op_dump = 0x06;
const byte op_yield = 0x07;
const byte op_delay = 0x08;
const byte op_print = 0x09;
const byte op_jumpto = 0x0a;
const byte op_jumpif = 0x0b;
const byte op_sleep = 0x0c;
const byte op_declare = 0x0d;

// binary operations [0x20..0x3f]
const byte op_gt = 0x20;
const byte op_gte = 0x21;
const byte op_lt = 0x22;
const byte op_lte = 0x23;
const byte op_equal = 0x24;
const byte op_notequal = 0x25;
const byte op_xor = 0x26;
const byte op_and = 0x27;
const byte op_or = 0x28;
const byte op_add = 0x29;
const byte op_sub = 0x2a;
const byte op_mul = 0x2b;
const byte op_div = 0x2c;
const byte op_mod = 0x2d;

// unary operations
const byte op_not = 0x2e;
const byte op_inc = 0x2f;
const byte op_dec = 0x30;

const byte op_assign = 0x31;

// memory/io instructions [0x40..0x5f]
const byte op_memget = 0x40;
const byte op_memset = 0x41;
// const byte op_memcopy = 0x42;

const byte op_iowrite = 0x43;
const byte op_ioread = 0x44;
const byte op_iomode = 0x45;
const byte op_iotype = 0x46;
const byte op_ioallout = 0x47;
const byte op_iointerrupt = 0x48;
const byte op_iointerruptEnable = 0x49;
const byte op_iointerruptDisable = 0x4a;

// wifi [0x60..0x6f]
const byte op_wifistatus = 0x60;
const byte op_wifiap = 0x61;
const byte op_wificonnect = 0x62;
const byte op_wifidisconnect = 0x63;
const byte op_wifilist = 0x64;

// protocols [0x70..0x8f]
// const byte op_i2setup = 0x70;
// const byte op_i2start = 0x71;
// const byte op_i2stop = 0x72;
// const byte op_i2write = 0x73;
// const byte op_i2read = 0x74;
// const byte op_i2setack = 0x75;
// const byte op_i2getack = 0x76;
// const byte op_i2find = 0x77;
// const byte op_i2writeack = 0x78;
// const byte op_i2writeack_b = 0x79;

const byte vt_null = 0;
const byte vt_identifier = 1;
const byte vt_byte = 2;
const byte vt_pin = 3;
const byte vt_address = 4;
const byte vt_integer = 5;
const byte vt_signedInteger = 6;
const byte vt_string = 7;

class Value
{
protected:
  bool hasValue = false;
  byte type = 0;
  void *value = NULL;

  void freeValue()
  {
    if (hasValue)
    {
      os_free(value);
      value = NULL;
    }
  }

public:
  void update(Value value)
  {
    update(value.type, value.value, value.hasValue);
  }

  void update(byte newType, void *newValue)
  {
    update(newType, newValue, false);
  }

  void update(byte newType, void *newValue, bool hasValue)
  {
    freeValue();
    type = newType;
    value = newValue;
    this->hasValue = hasValue;
  }

  byte getType()
  {
    return type;
  }

  uint toInteger()
  {
    byteref byte0 = (byteref)value;
    byteref byte1 = byte0 + 1;
    byteref byte2 = byte1 + 1;
    byteref byte3 = byte2 + 1;

    return (uint)(*byte3 & 0xff) << 24 |
           (uint)(*byte2 & 0xff) << 16 |
           (uint)(*byte1 & 0xff) << 8 |
           (uint)(*byte0 & 0xff);
  }

  byte toByte()
  {
    return *((byteref)value);
  }

  uint fromAddress()
  {
    return READ_PERI_REG(toInteger());
  }

  bool fromPin()
  {
    return pinRead(toByte());
  }

  string toString()
  {
    return (byteref)value;
  }

  bool toBoolean()
  {
    switch (type)
    {
    case vt_byte:
    case vt_identifier: // TODO unwrap value of identifier
      return toByte() != 0;

    case vt_integer:
      return toInteger() != 0;

    case vt_string:
      return strlen((const char *)toString()) != 0;

    case vt_pin:
      return fromPin() != 0;

    case vt_address:
      return fromAddress() != 0;
    }

    return false;
  }
};

typedef void (*send_callback)(char *, int);

typedef struct
{
  os_timer_t timer;
  byteref bytes = NULL;
  uint endOfTheProgram = 0;
  uint counter = 0;
  uint delayTime = 0;
  Value slots[MAX_SLOTS];
  uint interruptHandlers[NUMBER_OF_PINS];
  bool paused = false;
  bool debugger = false;
  send_callback onSend = 0;
} Program;

static Wifi wifi;

void program_tick(void *p);
void program_printf(Program *p, const char *format, ...) __attribute__((format(printf, 2, 3)));

void program_printf(Program *p, const char *format, ...)
{
  if (p->onSend == 0 || !p->debugger)
  {
    return;
  }

  va_list arg;
  va_start(arg, format);
  char buffer[1024];
  int length = os_sprintf(buffer, format, arg);
  va_end(arg);
  p->onSend(buffer, length);
}

void _onInterruptTrigger(void *arg, uint8_t pin)
{
  Program *p = (Program *)arg;
  p->counter = p->interruptHandlers[pin];
  p->paused = false;

  program_tick(p);
}

void _printValue(Program *p, Value value)
{
  switch (value.getType())
  {
  case vt_identifier:
    _printValue(p, p->slots[value.toByte()]);
    break;
  case vt_byte:
    program_printf(p, "%02x", value.toByte());
    break;

  case vt_pin:
    program_printf(p, "%d", value.fromPin());
    break;

  case vt_integer:
    program_printf(p, "%d", value.toInteger());
    break;

  case vt_address:
    program_printf(p, "%p", value.fromAddress());
    break;

  case vt_string:
    program_printf(p, "%s", value.toString());
    break;
  }
}

void move(Program *p, uint32_t amount)
{
  p->counter += amount;
}

byteref readByte(Program *p)
{
  byteref reference = &p->bytes[p->counter];
  move(p, 1);

  return reference;
}

uintref readNumber(Program *p)
{
  byteref reference = &p->bytes[p->counter];
  move(p, 4);

  return (uintref)reference;
}

byteref readString(Program *p)
{
  byteref string = &p->bytes[p->counter];

  // extra \0 at the end of string
  move(p, strlen((const char *)string) + 1);

  return string;
}

Value readValue(Program *p)
{
  byte type = *(readByte(p));
  Value value;

  switch (type)
  {
  case vt_byte:
  case vt_pin:
  case vt_identifier:
    value.update(type, (void *)readByte(p));
    break;

  case vt_integer:
  case vt_address:
    value.update(type, (void *)readNumber(p));
    break;

  case vt_string:
    value.update(type, (void *)readString(p));
    break;
  }

  return value;
}

void updateSlotWithInteger(Program *p, byte slotId, uint value)
{
  auto type = p->slots[slotId].getType();
  auto valueRef = os_zalloc(sizeof(uint));
  *((uintref)valueRef) = value;

  p->slots[slotId].update(type, valueRef, true);
}

void binaryOperation(Program *p, byte operation)
{
  auto target = readValue(p);
  auto a = readValue(p);
  auto b = readValue(p);

  auto valueOfA = a.toInteger();
  auto valueOfB = b.toInteger();
  uint newValue;

  switch (operation)
  {

  case op_gt:
    newValue = valueOfA > valueOfB;
    break;
  case op_gte:
    newValue = valueOfA >= valueOfB;
    break;
  case op_lt:
    newValue = valueOfA < valueOfB;
    break;
  case op_lte:
    newValue = valueOfA <= valueOfB;
    break;
  case op_equal:
    newValue = valueOfA == valueOfB;
    break;
  case op_notequal:
    newValue = valueOfA != valueOfB;
    break;
  case op_xor:
    newValue = valueOfA ^ valueOfB;
    break;
  case op_and:
    newValue = valueOfA & valueOfB;
    break;
  case op_or:
    newValue = valueOfA | valueOfB;
    break;
  case op_add:
    newValue = valueOfA + valueOfB;
    break;
  case op_sub:
    newValue = valueOfA - valueOfB;
    break;
  case op_mul:
    newValue = valueOfA * valueOfB;
    break;
  case op_div:
    newValue = valueOfA / valueOfB;
    break;
  case op_mod:
    newValue = valueOfA % valueOfB;
    break;
  }

  updateSlotWithInteger(p, target.toByte(), newValue);
  program_printf(p, "Binary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void unaryOperation(Program *p, byte operation)
{
  auto target = readValue(p);
  auto newValue = target.toInteger() + ((operation == op_inc) ? 1 : -1);

  updateSlotWithInteger(p, target.toByte(), newValue);
  program_printf(p, "Unary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void notOperation(Program *p)
{
  auto target = readValue(p);
  auto value = !readValue(p).toBoolean();

  updateSlotWithInteger(p, target.toByte(), (uint)value);
  program_printf(p, "Not %d: %d\n", target.toByte(), value);
}

void assignOperation(Program *p)
{
  auto target = readValue(p);
  auto value = readValue(p);

  p->slots[target.toByte()].update(value);
}

void sleep(Program *p)
{
  auto time = readValue(p).toInteger();

  program_printf(p, "sleep %ld\n", time);
  system_deep_sleep_set_option(2);
  system_deep_sleep((uint64_t)time);
}

void halt(Program *p)
{
  program_printf(p, "halt\n");
  p->paused = true;
}

void yield(Program *p)
{
  p->delayTime = 1;
}

void delay(Program *p)
{
  p->delayTime = readValue(p).toInteger();

  if (p->delayTime > MAX_DELAY)
  {
    p->delayTime = MAX_DELAY;
  }

  program_printf(p, "delay %d\n", p->delayTime);
}

void ioInterrupt(Program *p)
{
  auto pin = readValue(p).toByte();
  auto mode = readValue(p).toByte();
  auto position = readValue(p).toInteger();

  p->interruptHandlers[pin] = position;
  program_printf(p, "interrupt pin %d, mode %d, jump to %d\n", pin, mode, position);

  attachPinInterrupt(pin, (interruptCallbackHandler)&_onInterruptTrigger, (void *)p, (GPIO_INT_TYPE)mode);
}

void ioInterruptEnable(Program *p)
{
  armInterrupts();
  program_printf(p, "interrupts armed\n");
}

void ioInterruptDisable(Program *p)
{
  disarmInterrupts();
  program_printf(p, "interrupts disarmed\n");
}

void jumpTo(Program *p)
{
  auto position = readValue(p);
  p->counter = position.toInteger();
  program_printf(p, "jump to %d\n", p->counter);
}

void jumpIf(Program *p)
{
  auto condition = readValue(p);
  auto position = readValue(p);

  if (!condition.toBoolean())
    return;

  p->counter = position.toInteger();
  program_printf(p, "jump if: %d\n", p->counter);
}

void toggleDebug(Program *p)
{
  auto value = *(readByte(p));

  if (value)
  {
    system_uart_de_swap();
    program_printf(p, "debug is on\n");
    p->debugger = false;
    return;
  }

  p->debugger = true;
  program_printf(p, "debug is off\n");
  system_uart_swap();
}

void printStationStatus(Program *p)
{
  // TODO
}

void systemInformation(Program *p)
{
  program_printf(p, "Chip %ld\n", system_get_chip_id());
  program_printf(p, "SDK %s\n", system_get_sdk_version());
  program_printf(p, "Time %ld\n", system_get_time() / 1000);
  system_print_meminfo();
  program_printf(p, "Free %ld bytes\n", system_get_free_heap_size());
}

void dump(Program *p)
{
  uint i = 0;
  program_printf(p, "\nProgram\n");
  while (i < p->endOfTheProgram)
  {
    program_printf(p, "%02x ", p->bytes[i++]);
  }

  program_printf(p, "\nSlots\n");
  for (i = 0; i < MAX_SLOTS; i++)
  {
    program_printf(p, "%d: '", i);
    _printValue(p, p->slots[i]);
    program_printf(p, "'\n");
  }

  program_printf(p, "\nInterrupts\n");
  for (i = 0; i < NUMBER_OF_PINS; i++)
  {
    program_printf(p, "%d: %d\n", i, p->interruptHandlers[i]);
  }
}

void print(Program *p)
{
  auto value = readValue(p);
  _printValue(p, value);
}

void declareReference(Program *p)
{
  auto slotId = readValue(p).toByte();
  auto value = readValue(p);

  p->slots[slotId].update(value);

  program_printf(p, "declare %d, %d = ", slotId, p->slots[slotId].getType());
  _printValue(p, p->slots[slotId]);
  program_printf(p, "\n");
}

void readFromMemory(Program *p)
{
  auto slotId = readValue(p).toByte();
  auto address = (void *)readValue(p).toInteger();

  program_printf(p, "memget [%d], %ld\n", slotId, address);

  // if (1)
  // {
  //   if (address < 0x3ff00000 || address >= 0x60010000)
  //   {
  //     return;
  //   }

  //   uint32_t val_aligned = *(uint32_t *)(addr & (~3));
  //   uint32_t shift = (addr & 3) * 8;
  //   return (val_aligned >> shift) & 0xff;
  // }

  p->slots[slotId].update(vt_address, address);
}

void writeToMemory(Program *p)
{
  auto address = readValue(p).toInteger();
  auto value = readValue(p);

  program_printf(p, "memset %ld\n", address);

  switch (value.getType())
  {
  case vt_byte:
    WRITE_PERI_REG(address, value.toByte());
    break;

  case vt_pin:
    WRITE_PERI_REG(address, value.fromPin());
    break;

  case vt_integer:
    WRITE_PERI_REG(address, value.toInteger());
    break;

  case vt_address:
    WRITE_PERI_REG(address, READ_PERI_REG(value.toInteger()));
    break;
  }
}

void ioMode(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toByte();

  program_printf(p, "io mode %d %d\n", pin, value);
  pinMode(pin, (PinMode)value);
}

void ioType(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toByte();

  program_printf(p, "io type %d %d\n", pin, value);
  pinType(pin, value);
}

void ioWrite(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toBoolean();

  program_printf(p, "io write %d %d\n", pin, value);
  pinWrite(pin, (bool)value);
}

void ioRead(Program *p)
{
  auto target = readValue(p);
  auto pin = readValue(p).toByte();
  auto pinValue = (byte)pinRead(pin);

  updateSlotWithInteger(p, target.toByte(), (uint)pinValue);
  program_printf(p, "io read %d, %d\n", pin, pinValue);
}

void ioAllOut(Program *p)
{
  program_printf(p, "io all out\n");
  pinType(0, 0);
  pinType(1, 3);
  pinType(2, 0);
  pinType(3, 3);
  pinMode(0, PinOutput);
  pinMode(1, PinOutput);
  pinMode(2, PinOutput);
  pinMode(3, PinOutput);
}

void startAccessPoint(Program *p)
{
  program_printf(p, "startAccessPoint\n");
  wifi.startAccessPoint();
}

void program_next(Program *p)
{
  byte next = *(readByte(p));

  switch (next)
  {
  case op_noop:
    break;

  case op_halt:
    halt(p);
    break;

  case op_restart:
    system_restart();
    break;

  case op_debug:
    toggleDebug(p);
    break;

  case op_systeminfo:
    systemInformation(p);
    break;

  case op_sleep:
    sleep(p);
    break;

  case op_print:
    print(p);
    break;

  case op_dump:
    dump(p);
    break;

  case op_declare:
    declareReference(p);
    break;

  case op_memget:
    readFromMemory(p);
    break;

  case op_memset:
    writeToMemory(p);
    break;

  case op_iowrite:
    ioWrite(p);
    break;

  case op_ioread:
    ioRead(p);
    break;

  case op_iomode:
    ioMode(p);
    break;

  case op_iotype:
    ioType(p);
    break;

  case op_ioallout:
    ioAllOut(p);
    break;

  case op_iointerrupt:
    ioInterrupt(p);
    break;

  case op_iointerruptEnable:
    ioInterruptEnable(p);
    break;

  case op_iointerruptDisable:
    ioInterruptDisable(p);
    break;

  case op_delay:
    delay(p);
    break;

  case op_yield:
    yield(p);
    break;

  case op_jumpto:
    jumpTo(p);
    break;

  case op_jumpif:
    jumpIf(p);
    break;

  case op_gt:
  case op_gte:
  case op_lt:
  case op_lte:
  case op_equal:
  case op_notequal:
  case op_xor:
  case op_and:
  case op_or:
  case op_add:
  case op_sub:
  case op_mul:
  case op_div:
  case op_mod:
    binaryOperation(p, next);
    break;

  case op_inc:
  case op_dec:
    unaryOperation(p, next);
    break;

  case op_not:
    notOperation(p);
    break;

  case op_assign:
    assignOperation(p);
    break;

  case op_wifiap:
    startAccessPoint(p);
    break;

  case op_wifistatus:
    printStationStatus(p);
    break;

  default:
    program_printf(p, "\n[!] Invalid operation: %02x\n", next);
    halt(p);
  }

  if (p->counter >= p->endOfTheProgram)
  {
    halt(p);
  }
}

void program_tick(void *p)
{
  Program *program = (Program *)p;

  if (program->paused)
  {
    os_timer_disarm(&program->timer);
    program_printf(program, "\n[!] program is paused\n");
    return;
  }

  while (!program->delayTime && !program->paused)
  {
    program_next(program);
  }

  if (program->delayTime)
  {
    os_timer_disarm(&program->timer);
    os_timer_arm(&program->timer, (uint32_t)program->delayTime, 0);
    program->delayTime = 0;
  }
}

void program_load(Program *program, byteref _bytes, uint length)
{
  if (program->bytes != NULL && program->endOfTheProgram < length)
  {
    program->bytes = (string)os_realloc(program->bytes, length);
  }

  if (program->bytes == NULL)
  {
    program->bytes = (string)os_zalloc(length);
  }

  os_memcpy(program->bytes, _bytes, length);
  program->endOfTheProgram = length;
  program->counter = 0;
  program->paused = false;

  program_printf(program, "[i] Loaded %d bytes\n", length);
}

void program_start(Program *program)
{
  uint i = 0;
  for (i = 0; i < NUMBER_OF_PINS; i++)
  {
    program->interruptHandlers[i] = 0;
  }

  os_timer_setfn(&program->timer, (os_timer_func_t *)program_tick, program);
  os_timer_arm(&program->timer, 1, 0);
}