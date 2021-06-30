#include "c_types.h"
#include "ets_sys.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "pins.h"
// #include "i2c.h"

#define MAX_DELAY 6871000

typedef unsigned char *string;

typedef unsigned char byte;
typedef unsigned char *byteref;

typedef unsigned int uint;
typedef unsigned int *uintref;

#ifdef DEBUG
#define TRACE(...) os_printf(__VA_ARGS__);
#else
#define TRACE(...)
#endif

const byte op_halt = 0xfe;    // -
const byte op_restart = 0xfc; // -
const byte op_sysinfo = 0xfd; // -
const byte op_debug = 0xfb;   // byte enable
// const byte op_dump = 0xf9;           // -
const byte op_noop = 0x01; // -
// const byte op_yield = 0xfa;          // -
const byte op_delay = 0x02;  // uint32 delay
const byte op_print = 0x03;  // uint8[] message
const byte op_jumpto = 0x04; // uint32 address
const byte op_jumpif = 0x0f; // uint8 slot, uint32 address
const byte op_memget = 0x05; // uint8 slot, int32 address
const byte op_memset = 0x06; // int32 address, uint8 slot
// const byte op_gt = 0x09;             // uint8 slot, uint8 b, uint8 b
// const byte op_gte = 0x0a;            // uint8 slot, uint8 b, uint8 b
// const byte op_lt = 0x0b;             // uint8 slot, uint8 b, uint8 b
// const byte op_lte = 0x0c;            // uint8 slot, uint8 b, uint8 b
// const byte op_equal = 0x0d;          // uint8 slot, uint8 b, uint8 b
// const byte op_notequal = 0x0e;       // uint8 slot, uint8 b, uint8 b
// const byte op_xor = 0x10;            // uint8 slot, uint8 a, uint8 b
// const byte op_and = 0x11;            // uint8 slot, uint8 a, uint8 b
// const byte op_or = 0x12;             // uint8 slot, uint8 a, uint8 b
// const byte op_not = 0x13;            // uint8 slot
// const byte op_inc = 0x14;            // uint8 slot
// const byte op_dec = 0x15;            // uint8 slot
// const byte op_add = 0x16;            // uint8 slot, uint8 slot
// const byte op_sub = 0x17;            // uint8 slot, uint8 slot
// const byte op_mul = 0x18;            // uint8 slot, uint8 slot
// const byte op_div = 0x19;            // uint8 slot, uint8 slot
// const byte op_mod = 0x1a;            // uint8 slot, uint8 slot
// const byte op_copy = 0x1b;           // uint8 dest, uint8 src
const byte op_iowrite = 0x31;  // uint8 pin, uint8 slot
const byte op_ioread = 0x32;   // uint8 slot, uint8 pin
const byte op_iomode = 0x35;   // uint8 pin, uint8 slot
const byte op_iotype = 0x36;   // uint8 pin, uint8 slot
const byte op_ioallout = 0x37; // -
// const byte op_wificonnect = 0x3a;    // uint8 pin, uint8 slot
// const byte op_wifidisconnect = 0x3b; // uint8 pin, uint8 slot
// const byte op_wifistatus = 0x3c;     // uint8 pin, uint8 slot
// const byte op_wifilist = 0x3e;       // uint8 pin, uint8 slot
const byte op_sleep = 0x3f; // uint8 mode, uint32 time
// const byte op_i2setup = 0x40;        // -
// const byte op_i2start = 0x41;        // -
// const byte op_i2stop = 0x42;         // -
// const byte op_i2write = 0x43;        // uint8 byte
// const byte op_i2read = 0x44;         // uint8 slot
// const byte op_i2setack = 0x45;       // uint8 byte
// const byte op_i2getack = 0x46;       // uint8 byte
// const byte op_i2find = 0x48;         // uint8 slot
// const byte op_i2writeack = 0x49;     // uint32 length, uint* bytes
// const byte op_i2writeack_b = 0x4a;   // uint8 byte

const byte vt_identifier = 1;
const byte vt_byte = 2;
const byte vt_pin = 3;
const byte vt_address = 4;
const byte vt_integer = 5;
const byte vt_signedInteger = 6;
const byte vt_string = 7;

class Value
{
public:
  byte type;
  void *value;

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
      return toByte() != 0;

    case vt_integer:
      return toInteger() != 0;

    case vt_string:
      return toString() != 0;

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
  string bytes = NULL;
  uint endOfTheProgram = 0;
  uint counter = 0;
  uint delayTime = 0;
  Value slots[256];
  bool paused = false;
  bool debugEnabled = true;

  Program() {}

  void load(byteref _bytes, uint length)
  {
    bytes = _bytes;
    endOfTheProgram = length;
    TRACE("Loaded %d bytes\n", length);
    // if (bytes != 0)
    // {
    //   os_free(bytes);
    // }

    // bytes = (uint8_t *)os_zalloc(length);
    // bytes = length;
    // os_memcpy(bytes, program, length);
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

    case op_sysinfo:
      systemInformation();
      break;

    case op_sleep:
      sleep();
      break;

    case op_print:
      print();
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

    default:
      TRACE("Invalid operation: %02x\n", next);
      halt();
    }

    if (counter >= endOfTheProgram)
    {
      halt();
    }
  }

  void sleep()
  {
    auto value = readValue();
    auto time = value.toInteger();

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
    TRACE("jump if [%d], %d\n", condition.type, counter);
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

  void print()
  {
    auto value = readValue();

    switch (value.type)
    {
    case vt_byte:
      TRACE("%02x", value.toByte());
      break;

    case vt_pin:
      TRACE("%d", value.fromPin());
      break;

    case vt_integer:
      TRACE("%d", value.toInteger());
      break;

    case vt_address:
      TRACE("%p", value.fromAddress());
      break;

    case vt_string:
      TRACE("%s", value.toString());
      break;
    }
  }

  void readFromMemory()
  {
    auto slot = readValue().toByte();
    auto address = (void *)readValue().toInteger();

    Value newValue;

    // TRACE("memget [%d], %ld\n", slot, address);
    slots[slot] = newValue;
    newValue.type = vt_address;
    newValue.value = address;
  }

  void writeToMemory()
  {
    auto address = readValue().toInteger();
    auto value = readValue();

    // TRACE("memset [%d], %ld\n", slot, address);

    switch (value.type)
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

      // case vt_address:
      // case vt_string:
      // break;
    }
  }

  void ioMode()
  {
    auto pin = readValue().toByte();
    auto value = readValue().toByte();

    TRACE("io mode %d %d\n", pin, value);
    pinMode(pin, (PinMode)value);
  }

  void ioType()
  {
    auto pin = readValue().toByte();
    auto value = readValue().toByte();

    TRACE("io type %d %d\n", pin, value);
    pinType(pin, value);
  }

  void ioWrite()
  {
    auto pin = readValue().toByte();
    auto value = readValue().toByte();

    TRACE("io write %d %d\n", pin, value);
    pinWrite(pin, (bool)value);
  }

  void ioRead()
  {
    auto pin = readValue().toByte();
    auto target = readValue().toByte();
    Value value;

    value.type = vt_byte;
    value.value = &target;

    TRACE("io read %d %d\n", pin, target);
    slots[target] = value;
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
    byteref typeRef = readByte();
    byte type = *typeRef;
    Value value;

    value.type = type;

    switch (type)
    {
    case vt_byte:
    case vt_pin:
    case vt_identifier:
      value.value = (void *)readByte();
      break;

    case vt_integer:
    case vt_address:
      value.value = (void *)readNumber();
      break;

    case vt_string:
      value.value = (void *)readString();
      break;
    }

    if (type == vt_identifier)
    {
      value = slots[value.toByte()];
    }

    return value;
  }
};
