#include "c_types.h"
// #include "ets_sys.h"
// #include "mem.h"
// #include "osapi.h"
// #include "user_interface.h"
#include "pins.h"
// #include "i2c.h"

const uint8_t op_halt = 0xfe;           // -
const uint8_t op_restart = 0xfc;        // -
const uint8_t op_sysinfo = 0xfd;        // -
const uint8_t op_debug = 0xfb;          // uint8_t enable
const uint8_t op_dump = 0xf9;           // -
const uint8_t op_noop = 0x01;           // -
const uint8_t op_yield = 0xfa;          // -
const uint8_t op_delay = 0x02;          // uint32 delay
const uint8_t op_print = 0x03;          // uint8[] message
const uint8_t op_jumpto = 0x04;         // uint32 address
const uint8_t op_jumpif = 0x0f;         // uint8 slot, uint32 address
const uint8_t op_memget = 0x05;         // uint8 slot, int32 address
const uint8_t op_memset = 0x06;         // int32 address, uint8 slot
const uint8_t op_gt = 0x09;             // uint8 slot, uint8 b, uint8 b
const uint8_t op_gte = 0x0a;            // uint8 slot, uint8 b, uint8 b
const uint8_t op_lt = 0x0b;             // uint8 slot, uint8 b, uint8 b
const uint8_t op_lte = 0x0c;            // uint8 slot, uint8 b, uint8 b
const uint8_t op_equal = 0x0d;          // uint8 slot, uint8 b, uint8 b
const uint8_t op_notequal = 0x0e;       // uint8 slot, uint8 b, uint8 b
const uint8_t op_xor = 0x10;            // uint8 slot, uint8 a, uint8 b
const uint8_t op_and = 0x11;            // uint8 slot, uint8 a, uint8 b
const uint8_t op_or = 0x12;             // uint8 slot, uint8 a, uint8 b
const uint8_t op_not = 0x13;            // uint8 slot
const uint8_t op_inc = 0x14;            // uint8 slot
const uint8_t op_dec = 0x15;            // uint8 slot
const uint8_t op_add = 0x16;            // uint8 slot, uint8 slot
const uint8_t op_sub = 0x17;            // uint8 slot, uint8 slot
const uint8_t op_mul = 0x18;            // uint8 slot, uint8 slot
const uint8_t op_div = 0x19;            // uint8 slot, uint8 slot
const uint8_t op_mod = 0x1a;            // uint8 slot, uint8 slot
const uint8_t op_copy = 0x1b;           // uint8 dest, uint8 src
const uint8_t op_iowrite = 0x31;        // uint8 pin, uint8 slot
const uint8_t op_ioread = 0x32;         // uint8 slot, uint8 pin
const uint8_t op_iomode = 0x35;         // uint8 pin, uint8 slot
const uint8_t op_iotype = 0x36;         // uint8 pin, uint8 slot
const uint8_t op_ioallout = 0x37;       // -
const uint8_t op_wificonnect = 0x3a;    // uint8 pin, uint8 slot
const uint8_t op_wifidisconnect = 0x3b; // uint8 pin, uint8 slot
const uint8_t op_wifistatus = 0x3c;     // uint8 pin, uint8 slot
const uint8_t op_wifilist = 0x3e;       // uint8 pin, uint8 slot
const uint8_t op_sleep = 0x3f;          // uint8 mode, uint32 time
const uint8_t op_i2setup = 0x40;        // -
const uint8_t op_i2start = 0x41;        // -
const uint8_t op_i2stop = 0x42;         // -
const uint8_t op_i2write = 0x43;        // uint8 byte
const uint8_t op_i2read = 0x44;         // uint8 slot
const uint8_t op_i2setack = 0x45;       // uint8 byte
const uint8_t op_i2getack = 0x46;       // uint8 byte
const uint8_t op_i2find = 0x48;         // uint8 slot
const uint8_t op_i2writeack = 0x49;     // uint32 length, uint* bytes
const uint8_t op_i2writeack_b = 0x4a;   // uint8 byte

const uint8_t vt_identifier = 1;
const uint8_t vt_byte = 2;
const uint8_t vt_pin = 3;
const uint8_t vt_address = 4;
const uint8_t vt_integer = 5;
const uint8_t vt_signedInteger = 6;
const uint8_t vt_string = 7;

typedef struct
{
  uint8_t type;
  void *value;
} Value;

class Program
{
public:
  uint8_t *bytes;
  int endOfTheProgram = 0;
  int counter = 0;
  bool paused = false;

  void tick()
  {
    uint8_t *next = readByte();

    switch (*next)
    {
    case op_noop:
      break;

    case op_halt:
      halt();
      break;

    case op_print:
      print();
      break;

    case op_iowrite:
      ioWrite();
      break;

    case op_delay:
      delay();
      break;

    case op_jumpto:
      jumpTo();
      break;
    }

    if (counter >= endOfTheProgram)
    {
      // TODO halt
    }
  }

  void move(int amount)
  {
    counter += amount;
  }

  uint8_t *readByte()
  {
    uint8_t *byte = (uint8_t *)(bytes + counter);
    move(1);

    return byte;
  }

  int *readNumber()
  {
    int *number = (int *)(bytes + counter);
    move(4);

    return number;
  }

  uint8_t *readString()
  {
    uint8_t *position = bytes + counter;
    move(strlen((const char *)position));

    return position;
  }

  Value *readValue()
  {
    uint8_t *type = readByte();
    Value *value = new Value;
    value->type = *type;

    switch (*type)
    {
    case vt_byte:
    case vt_pin:
      value->value = (void *)readByte();

    case vt_integer:
    case vt_address:
      value->value = (void *)readNumber();

    case vt_string:
      value->value = (void *)readString();
    }

    return value;
  }

  void halt()
  {
    // TODO halt
  }

  void print()
  {
    auto value = readValue();
    // TODO print
  }

  void ioWrite()
  {
    auto pin = readByte();
    auto value = readValue();
    bool *pinValue = (bool *)value->value;

    pinWrite(*pin, *pinValue);
    // TODO pin write
  }

  void delay()
  {
    auto delay = readValue();
    // TODO delay
  }

  void jumpTo()
  {
    auto position = readNumber();
    counter = *position;
  }

  Program(uint8_t *_bytes, uint32_t length)
  {
    bytes = _bytes;
    endOfTheProgram = length;
  }
};
