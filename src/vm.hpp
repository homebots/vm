#include "sdk.h"

#define MAX_DELAY 6871000
#define MAX_SLOTS 256

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

// operators [0x20..0x3f]
// binary operations
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
const byte op_declare = 0x32;

// memory/io instructions [0x40..0x5f]
const byte op_memget = 0x40;
const byte op_memset = 0x41;

const byte op_iowrite = 0x43;
const byte op_ioread = 0x44;
const byte op_iomode = 0x45;
const byte op_iotype = 0x46;
const byte op_ioallout = 0x47;
const byte op_iointerrupt = 0x48;
const byte op_iointerruptToggle = 0x49;

// wifi [0x60..0x6f]
const byte op_wifistatus = 0x60;
const byte op_wifiap = 0x61;
const byte op_wificonnect = 0x62;
const byte op_wifidisconnect = 0x63;
const byte op_wifilist = 0x64;

// protocols [0x70..0x8f]
const byte op_i2csetup = 0x70;
const byte op_i2cstart = 0x71;
const byte op_i2cstop = 0x72;
const byte op_i2cwrite = 0x73;
const byte op_i2cread = 0x74;
const byte op_i2csetack = 0x75;
const byte op_i2cgetack = 0x76;
const byte op_i2cfind = 0x77;
const byte op_i2cwriteack = 0x78;
const byte op_i2cwriteack_b = 0x79;

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

  byteref toString()
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
  bool debugger = true;
  send_callback onSend = 0;
} Program;

static Wifi wifi;

void program_tick(void *p);
void program_printf(Program *p, const char *format, ...) __attribute__((format(printf, 2, 3)));

void program_printf(Program *p, const char *format, ...)
{
  if (p->onSend == NULL || !p->debugger)
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

byteref readByte(Program *p)
{
  byteref reference = &p->bytes[p->counter];
  p->counter += 1;

  return reference;
}

Value readValue(Program *p)
{
  byte type = *(readByte(p));
  Value value;
  byteref ref;

  switch (type)
  {
  case vt_byte:
  case vt_pin:
  case vt_identifier:
    value.update(type, (void *)readByte(p));
    break;

  case vt_integer:
  case vt_address:
    ref = &p->bytes[p->counter];
    p->counter += 4;
    value.update(type, (void *)ref);
    break;

  case vt_string:
    // extra \0 at the end of string
    ref = &p->bytes[p->counter];
    p->counter += strlen((const char *)ref) + 1;
    value.update(type, (void *)ref);
    break;
  }

  return value;
}

void _updateSlotWithInteger(Program *p, byte slotId, uint value)
{
  auto type = p->slots[slotId].getType();
  auto valueRef = os_zalloc(sizeof(uint));
  *((uintref)valueRef) = value;

  p->slots[slotId].update(type, valueRef, true);
}

void vm_binaryOperation(Program *p, byte operation)
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

  _updateSlotWithInteger(p, target.toByte(), newValue);
  program_printf(p, "Binary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void vm_unaryOperation(Program *p, byte operation)
{
  auto target = readValue(p);
  auto newValue = target.toInteger() + ((operation == op_inc) ? 1 : -1);

  _updateSlotWithInteger(p, target.toByte(), newValue);
  program_printf(p, "Unary %d: $%d = %d\n", operation, target.toByte(), newValue);
}

void vm_notOperation(Program *p)
{
  auto target = readValue(p);
  auto value = !readValue(p).toBoolean();

  _updateSlotWithInteger(p, target.toByte(), (uint)value);
  program_printf(p, "Not %d: %d\n", target.toByte(), value);
}

void vm_assignOperation(Program *p)
{
  auto target = readValue(p);
  auto value = readValue(p);

  p->slots[target.toByte()].update(value);
}

void vm_sleep(Program *p)
{
  auto time = readValue(p).toInteger();

  program_printf(p, "sleep %ld\n", time);
  system_deep_sleep_set_option(2);
  system_deep_sleep((uint64_t)time);
}

void vm_halt(Program *p)
{
  program_printf(p, "halt\n");
  p->paused = true;
}

void vm_yield(Program *p)
{
  p->delayTime = 1;
}

void vm_delay(Program *p)
{
  p->delayTime = readValue(p).toInteger();

  if (p->delayTime > MAX_DELAY)
  {
    p->delayTime = MAX_DELAY;
  }

  program_printf(p, "delay %d\n", p->delayTime);
}

void vm_ioInterrupt(Program *p)
{
  auto pin = readValue(p).toByte();
  auto mode = readValue(p).toByte();
  auto position = readValue(p).toInteger();

  p->interruptHandlers[pin] = position;
  program_printf(p, "interrupt pin %d, mode %d, jump to %d\n", pin, mode, position);

  attachPinInterrupt(pin, (interruptCallbackHandler)&_onInterruptTrigger, (void *)p, (GPIO_INT_TYPE)mode);
}

void vm_ioInterruptToggle(Program *p)
{
  auto enabled = readValue(p).toBoolean();

  if (enabled)
  {
    armInterrupts();
    program_printf(p, "interrupts armed\n");
    return;
  }

  disarmInterrupts();
  program_printf(p, "interrupts disarmed\n");
}

void vm_jumpTo(Program *p)
{
  auto position = readValue(p);
  p->counter = position.toInteger();
  program_printf(p, "jump to %d\n", p->counter);
}

void vm_jumpIf(Program *p)
{
  auto condition = readValue(p);
  auto position = readValue(p);

  if (!condition.toBoolean())
    return;

  p->counter = position.toInteger();
  program_printf(p, "jump if: %d\n", p->counter);
}

void vm_toggleDebug(Program *p)
{
  auto value = readValue(p).toBoolean();

  if (value)
  {
    system_uart_de_swap();
    program_printf(p, "serial debug on\n");
    p->debugger = false;
    return;
  }

  p->debugger = true;
  program_printf(p, "serial debug off\n");
  system_uart_swap();
}

void vm_printStationStatus(Program *p)
{
  // TODO
}

void vm_systemInformation(Program *p)
{
  program_printf(p, "Chip %ld\n", system_get_chip_id());
  program_printf(p, "SDK %s\n", system_get_sdk_version());
  program_printf(p, "Time %ld\n", system_get_time() / 1000);
  program_printf(p, "Free %ld bytes\n", system_get_free_heap_size());
}

void vm_dump(Program *p)
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

void vm_print(Program *p)
{
  auto value = readValue(p);
  _printValue(p, value);
}

void vm_declareReference(Program *p)
{
  auto slotId = readValue(p).toByte();
  auto value = readValue(p);

  p->slots[slotId].update(value);

  program_printf(p, "declare %d, %d = ", slotId, p->slots[slotId].getType());
  _printValue(p, p->slots[slotId]);
  program_printf(p, "\n");
}

void vm_readFromMemory(Program *p)
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

void vm_writeToMemory(Program *p)
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

void vm_ioMode(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toByte();

  program_printf(p, "io mode %d %d\n", pin, value);
  pinMode(pin, (PinMode)value);
}

void vm_ioType(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toByte();

  program_printf(p, "io type %d %d\n", pin, value);
  pinType(pin, value);
}

void vm_ioWrite(Program *p)
{
  auto pin = readValue(p).toByte();
  auto value = readValue(p).toBoolean();

  program_printf(p, "io write %d %d\n", pin, value);
  pinWrite(pin, (bool)value);
}

void vm_ioRead(Program *p)
{
  auto target = readValue(p);
  auto pin = readValue(p).toByte();
  auto pinValue = (byte)pinRead(pin);

  _updateSlotWithInteger(p, target.toByte(), (uint)pinValue);
  program_printf(p, "io read %d, %d\n", pin, pinValue);
}

void vm_ioAllOut(Program *p)
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

void vm_startAccessPoint(Program *p)
{
  program_printf(p, "startAccessPoint\n");
  wifi.startAccessPoint();
}

void wifiConnect(Program *p)
{
  auto ssid = readValue(p).toString();
  auto password = readValue(p);
  bool hasPassword = password.getType() == vt_string;

  program_printf(p, "connect to %s / %s\n", ssid, password.toString());

  if (hasPassword)
  {
    wifi.connectTo((const char *)ssid, (const char *)password.toString());
    return;
  }

  wifi.connectTo((const char *)ssid, (const char *)NULL);
}

void wifiDisconnect(Program *p)
{
  wifi.disconnect();
}

void wifiList(Program *p)
{
  // TODO
}

void vm_i2csetup(Program *p)
{
  auto dataPin = readValue(p).toByte();
  auto clockPin = readValue(p).toByte();

  i2c_setup(dataPin, clockPin);
  program_printf(p, "i2c setup SDA=%d SCL=%d\n", dataPin, clockPin);
}

void vm_i2cstart(Program *p)
{
  program_printf(p, "i2c start\n");
  i2c_start();
}

void vm_i2cstop(Program *p)
{
  program_printf(p, "i2c stop\n");
  i2c_stop();
}

void vm_i2cwrite(Program *p)
{
  auto byte = readValue(p).toByte();
  program_printf(p, "i2c write %d\n", byte);
  i2c_writeByteAndAck(byte);
}

void vm_i2cread(Program *p)
{
  auto target = readValue(p);
  byte value = i2c_readByte();
  void *v = os_zalloc(sizeof(byte));
  *(byte *)v = value;
  p->slots[target.toByte()].update(vt_byte, &v, true);
  program_printf(p, "i2c read %d\n", value);
}

void program_next(Program *p)
{
  byte next = *(readByte(p));

  switch (next)
  {
  case op_noop:
    break;

  case op_halt:
    vm_halt(p);
    break;

  case op_restart:
    system_restart();
    break;

  case op_debug:
    vm_toggleDebug(p);
    break;

  case op_systeminfo:
    vm_systemInformation(p);
    break;

  case op_sleep:
    vm_sleep(p);
    break;

  case op_print:
    vm_print(p);
    break;

  case op_dump:
    vm_dump(p);
    break;

  case op_declare:
    vm_declareReference(p);
    break;

  case op_memget:
    vm_readFromMemory(p);
    break;

  case op_memset:
    vm_writeToMemory(p);
    break;

  case op_iowrite:
    vm_ioWrite(p);
    break;

  case op_ioread:
    vm_ioRead(p);
    break;

  case op_iomode:
    vm_ioMode(p);
    break;

  case op_iotype:
    vm_ioType(p);
    break;

  case op_ioallout:
    vm_ioAllOut(p);
    break;

  case op_iointerrupt:
    vm_ioInterrupt(p);
    break;

  case op_iointerruptToggle:
    vm_ioInterruptToggle(p);
    break;

  case op_delay:
    vm_delay(p);
    break;

  case op_yield:
    vm_yield(p);
    break;

  case op_jumpto:
    vm_jumpTo(p);
    break;

  case op_jumpif:
    vm_jumpIf(p);
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
    vm_binaryOperation(p, next);
    break;

  case op_inc:
  case op_dec:
    vm_unaryOperation(p, next);
    break;

  case op_not:
    vm_notOperation(p);
    break;

  case op_assign:
    vm_assignOperation(p);
    break;

  case op_wifistatus:
    vm_printStationStatus(p);
    break;

  case op_wifiap:
    vm_startAccessPoint(p);
    break;

  case op_wificonnect:
    wifiConnect(p);
    break;

  case op_wifidisconnect:
    wifiDisconnect(p);
    break;

  case op_wifilist:
    wifiList(p);
    break;

  default:
    program_printf(p, "\n[!] Invalid operation: %02x\n", next);
    vm_halt(p);
  }

  if (p->counter >= p->endOfTheProgram)
  {
    vm_halt(p);
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
    program->bytes = (byteref)os_realloc(program->bytes, length);
  }

  if (program->bytes == NULL)
  {
    program->bytes = (byteref)os_zalloc(length);
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