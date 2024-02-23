#include "osapi.h"

#define MAX_SLOTS 256
#define MAX_STACK_SIZE 64

#define vt_null 0
#define vt_identifier 1
#define vt_byte 2
#define vt_pin 3
#define vt_address 4
#define vt_integer 5
#define vt_signedInteger 6
#define vt_string 7

static char *printBuffer = (char *)malloc(1024);

typedef unsigned char byte;
typedef unsigned char *byteref;
typedef unsigned int uint;
typedef unsigned int *uintref;
typedef unsigned int uint32;
typedef unsigned long long uint64;

class Value
{
protected:
  bool hasValue = false;
  byte type = 0;
  void *value = nullptr;

  void freeValue()
  {
    if (hasValue)
    {
      os_free(value);
      value = nullptr;
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

  uint32 toInteger()
  {
    byteref byte0 = (byteref)value;
    byteref byte1 = byte0 + 1;
    byteref byte2 = byte1 + 1;
    byteref byte3 = byte2 + 1;

    uint32 number = (*byte3 & 0xff) << 24 |
                    (*byte2 & 0xff) << 16 |
                    (*byte1 & 0xff) << 8 |
                    (*byte0 & 0xff);

    return number;
  }

  byte toByte()
  {
    return *((byteref)value);
  }

  uint fromAddress()
  {
    return os_mem_read(toInteger());
  }

  bool fromPin()
  {
    return os_io_read(toByte());
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
      return os_strlen((const char *)toString()) != 0;

    case vt_pin:
      return fromPin() != 0;

    case vt_address:
      return fromAddress() != 0;
    }

    return false;
  }
};

typedef void (*send_callback)(char *, int);
typedef void (*halt_callback)();

class Program
{
public:
  Timer timer;
  byteref bytes = nullptr;
  uint endOfTheProgram = 0;
  uint counter = 0;
  uint delayTime = 0;
  Value slots[MAX_SLOTS];
  uint interruptHandlers[NUMBER_OF_PINS];
  bool paused = false;
  bool debug = false;
  send_callback onSend = 0;
  halt_callback onHalt = 0;

  int callStack[MAX_STACK_SIZE];
  int callStackPointer = MAX_STACK_SIZE - 1;

  void reset()
  {
    counter = 0;
    paused = false;

    os_memset(&interruptHandlers, 0, NUMBER_OF_PINS * sizeof(uint));
    os_memset(&callStack, 0, MAX_STACK_SIZE * sizeof(int));
  }

  int callStackPush(int value)
  {
    if (callStack[callStackPointer] == value)
    {
      return 0;
    }

    if (callStackPointer == 0)
    {
      paused = true;
      return -1;
    }

    callStackPointer--;
    callStack[callStackPointer] = counter;
    return 1;
  }

  int callStackPop()
  {
    if (callStackPointer + 1 >= MAX_STACK_SIZE)
    {
      paused = true;
      return -1;
    }

    counter = callStack[callStackPointer];
    callStackPointer++;
    return 1;
  }

  void printf(const char *format, va_list args)
  {
    os_memset(printBuffer, 0, 1024);
    char *p = (char *)format;
    int len = 0;

    for (; *p; p++)
    {
      if (*p != '%')
      {
        printBuffer[len++] = (*p);
        continue;
      }
      switch (*++p)
      {
      case 'd':
      {
        int d = va_arg(args, int);
        int c = ets_sprintf(printBuffer + len, "%d", d);
        len += c - 1;
        break;
      }
      case 'x':
      {
        int d = va_arg(args, int);
        int c = ets_sprintf(printBuffer + len, "%02x", d);
        len += c - 1;
        break;
      }
      case 'c':
      {
        char c = va_arg(args, int);
        printBuffer[len++] = c;
        break;
      }
      case 's':
      {
        char *s = va_arg(args, char *);
        int slen = strlen(s);
        os_memcpy(printBuffer + len, s, slen);
        len += slen;
        break;
      }
      default:
        printBuffer[len++] = (*p);
        break;
      }
    }

    onSend(printBuffer, len);
  }
};
