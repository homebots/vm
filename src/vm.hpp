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

#ifdef WITH_DEBUG
#define TRACE(...) os_printf(__VA_ARGS__);
#else
#define TRACE(...)
#endif

// system instructions
const byte op_noop = 0x01;
const byte op_halt = 0x02;
const byte op_restart = 0x03;
const byte op_systeminfo = 0x04;
const byte op_debug = 0x05;
const byte op_dump = 0x06;

const byte op_delay = 0x08;
const byte op_print = 0x09;
const byte op_jumpto = 0x0a;
const byte op_jumpif = 0x0b;
const byte op_sleep = 0x0c;
const byte op_declare = 0x0d;

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

// memory/io instructions
const byte op_memget = 0x40;
const byte op_memset = 0x41;
// const byte op_memcopy = 0x42;
const byte op_iowrite = 0x43;
const byte op_ioread = 0x44;
const byte op_iomode = 0x45;
const byte op_iotype = 0x46;
const byte op_ioallout = 0x47;

// const byte op_wificonnect = 0x3a;
// const byte op_wifidisconnect = 0x3b;
// const byte op_wifistatus = 0x3c;
// const byte op_wifilist = 0x3e;
// const byte op_i2setup = 0x40;
// const byte op_i2start = 0x41;
// const byte op_i2stop = 0x42;
// const byte op_i2write = 0x43;
// const byte op_i2read = 0x44;
// const byte op_i2setack = 0x45;
// const byte op_i2getack = 0x46;
// const byte op_i2find = 0x48;
// const byte op_i2writeack = 0x49;
// const byte op_i2writeack_b = 0x4a;

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
      TRACE("free %d\n", value);
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

class Program
{
public:
  os_timer_t timer;
  byteref bytes = NULL;
  uint endOfTheProgram = 0;
  uint counter = 0;
  uint delayTime = 0;
  Value slots[MAX_SLOTS];
  bool paused = false;
  bool debugEnabled = true;

  void load(byteref _bytes, uint length)
  {
    if (bytes != NULL && endOfTheProgram < length)
    {
      bytes = (string)os_realloc(bytes, length);
    }

    if (bytes == NULL)
    {
      bytes = (string)os_zalloc(length);
    }

    os_memcpy(bytes, _bytes, length);
    endOfTheProgram = length;
    counter = 0;
    paused = false;

    TRACE("Loaded %d bytes\n", length);
  }

  void tick()
  {
    byte next = *(readByte());

    switch (next)
    {
    case op_noop:
      break;

    case op_halt:
      halt();
      break;

    case op_restart:
      system_restart();
      break;

    case op_debug:
      toggleDebug();
      break;

    case op_systeminfo:
      systemInformation();
      break;

    case op_sleep:
      sleep();
      break;

    case op_print:
      print();
      break;

    case op_dump:
      dump();
      break;

    case op_declare:
      declareReference();
      break;

    case op_memget:
      readFromMemory();
      break;

    case op_memset:
      writeToMemory();
      break;

    case op_iowrite:
      ioWrite();
      break;

    case op_ioread:
      ioRead();
      break;

    case op_iomode:
      ioMode();
      break;

    case op_iotype:
      ioType();
      break;

    case op_ioallout:
      ioAllOut();
      break;

    case op_delay:
      delay();
      break;

    case op_jumpto:
      jumpTo();
      break;

    case op_jumpif:
      jumpIf();
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
      binaryOperation(next);
      break;

    case op_inc:
    case op_dec:
      unaryOperation(next);
      break;

    case op_not:
      notOperation();
      break;

    case op_assign:
      assignOperation();
      break;

    default:
      TRACE("Invalid operation: %02x\n", next);
      halt();
    }

    if (counter >= endOfTheProgram)
    {
      halt();
    }
  }

  void binaryOperation(byte operation)
  {
    auto target = readValue();
    auto a = readValue();
    auto b = readValue();

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

    updateSlotWithInteger(target.toByte(), newValue);
    TRACE("Binary %d: $%d = %d\n", operation, target.toByte(), newValue);
  }

  void unaryOperation(byte operation)
  {
    auto target = readValue();
    auto newValue = target.toInteger() + ((operation == op_inc) ? 1 : -1);

    updateSlotWithInteger(target.toByte(), newValue);
    TRACE("Unary %d: $%d = %d\n", operation, target.toByte(), newValue);
  }

  void notOperation()
  {
    auto target = readValue();
    auto value = !readValue().toBoolean();

    updateSlotWithInteger(target.toByte(), (uint)value);
    TRACE("Not %d: %d\n", target.toByte(), value);
  }

  void assignOperation()
  {
    auto target = readValue();
    auto value = readValue();

    slots[target.toByte()].update(value);
  }

  void sleep()
  {
    auto time = readValue().toInteger();

    TRACE("sleep %ld\n", time);
    system_deep_sleep_set_option(2);
    system_deep_sleep((uint64_t)time);
  }

  void halt()
  {
    TRACE("halt\n");
    paused = true;
  }

  void delay()
  {
    delayTime = readValue().toInteger();

    if (delayTime > MAX_DELAY)
    {
      delayTime = MAX_DELAY;
    }

    TRACE("delay %d\n", delayTime);
  }

  void jumpTo()
  {
    auto position = readValue();
    counter = position.toInteger();
    TRACE("jump to %d\n", counter);
  }

  void jumpIf()
  {
    auto condition = readValue();
    auto position = readValue();

    if (!condition.toBoolean())
      return;

    counter = position.toInteger();
    TRACE("jump if: %d\n", counter);
  }

  void toggleDebug()
  {
    auto value = *(readByte());

    if (value)
    {
      system_uart_de_swap();
      TRACE("debug is on\n");
      return;
    }

    TRACE("debug is off\n");
    system_uart_swap();
  }

  void systemInformation()
  {
    TRACE("Chip %ld\n", system_get_chip_id());
    TRACE("SDK %s\n", system_get_sdk_version());
    TRACE("Time %ld\n", system_get_time() / 1000);
    system_print_meminfo();
    TRACE("Free %ld bytes\n", system_get_free_heap_size());
  }

  void dump()
  {
    uint i = 0;
    os_printf("\nProgram\n");
    while (i < endOfTheProgram)
    {
      os_printf("%02x ", bytes[i++]);
    }

    i = 0;
    for (; i < MAX_SLOTS; i++)
    {
      printValue(slots[i]);
      TRACE("'\n'");
    }
  }

  void print()
  {
    auto value = readValue();
    printValue(value);
  }

  void declareReference()
  {
    auto slotId = readValue().toByte();
    auto value = readValue().toByte();

    slots[slotId].update(value);

    TRACE("declare %d, %d\n", slotId, slots[slotId].getType());
  }

  void readFromMemory()
  {
    auto slotId = readValue().toByte();
    auto address = (void *)readValue().toInteger();

    TRACE("memget [%d], %ld\n", slotId, address);

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

    slots[slotId].update(vt_address, address);
  }

  void writeToMemory()
  {
    auto address = readValue().toInteger();
    auto value = readValue();

    TRACE("memset %ld\n", address);

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

  void ioMode()
  {
    auto pin = *(readByte());
    auto value = readValue().toByte();

    TRACE("io mode %d %d\n", pin, value);
    pinMode(pin, (PinMode)value);
  }

  void ioType()
  {
    auto pin = *(readByte());
    auto value = readValue().toByte();

    TRACE("io type %d %d\n", pin, value);
    pinType(pin, value);
  }

  void ioWrite()
  {
    auto pin = *(readByte());
    auto value = readValue().toBoolean();

    TRACE("io write %d %d\n", pin, value);
    pinWrite(pin, (bool)value);
  }

  void ioRead()
  {
    auto target = readValue();
    auto pin = *(readByte());
    auto pinValue = (byte)pinRead(pin);

    updateSlotWithInteger(target.toByte(), (uint)pinValue);
    TRACE("io read %d, %d\n", pin, pinValue);
  }

  void ioAllOut()
  {
    TRACE("io all out\n");
    pinType(0, 0);
    pinType(1, 3);
    pinType(2, 0);
    pinType(3, 3);
    pinMode(0, PinOutput);
    pinMode(1, PinOutput);
    pinMode(2, PinOutput);
    pinMode(3, PinOutput);
  }

protected:
  void printValue(Value value)
  {
    switch (value.getType())
    {
    case vt_identifier:
      printValue(slots[value.toByte()]);
      break;
    case vt_byte:
      os_printf("%02x", value.toByte());
      break;

    case vt_pin:
      os_printf("%d", value.fromPin());
      break;

    case vt_integer:
      os_printf("%d", value.toInteger());
      break;

    case vt_address:
      os_printf("%p", value.fromAddress());
      break;

    case vt_string:
      os_printf("%s", value.toString());
      break;
    }
  }

  void move(uint32_t amount)
  {
    counter += amount;
  }

  byteref readByte()
  {
    byteref reference = &bytes[counter];
    move(1);

    return reference;
  }

  uintref readNumber()
  {
    byteref reference = &bytes[counter];
    move(4);

    return (uintref)reference;
  }

  byteref readString()
  {
    byteref string = &bytes[counter];

    // extra \0 at the end of string
    move(strlen((const char *)string) + 1);

    return string;
  }

  Value readValue()
  {
    byte type = *(readByte());
    Value value;

    switch (type)
    {
    case vt_byte:
    case vt_pin:
    case vt_identifier:
      value.update(type, (void *)readByte());
      break;

    case vt_integer:
    case vt_address:
      value.update(type, (void *)readNumber());
      break;

    case vt_string:
      value.update(type, (void *)readString());
      break;
    }

    return value;
  }

  void updateSlotWithInteger(byte slotId, uint value)
  {
    auto type = slots[slotId].getType();
    auto valueRef = os_zalloc(sizeof(uint));
    *((uintref)valueRef) = value;

    slots[slotId].update(type, valueRef, true);
  }
};
