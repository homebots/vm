#define MAX_SLOTS 256
#define MAX_STACK_SIZE 64
#define MAX_STACK_CURSOR MAX_STACK_SIZE - 1
#define MAX_PRINT_BUFFER 1024
#define MAX_PRINT_CURSOR MAX_PRINT_BUFFER - 1

#define vt_null 0
#define vt_identifier 1
#define vt_byte 2
#define vt_pin 3
#define vt_address 4
#define vt_integer 5
#define vt_signedInteger 6
#define vt_string 7

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
  int callStackCursor = 0;

  char printBuffer[MAX_PRINT_BUFFER];
  int printBufferCursor = 0;

  void reset()
  {
    counter = 0;
    paused = false;

    os_memset(&interruptHandlers, 0, NUMBER_OF_PINS * sizeof(uint));
    os_memset(&callStack, 0, MAX_STACK_SIZE * sizeof(int));
    os_memset(&printBuffer, 0, MAX_PRINT_BUFFER);
    printBufferCursor = 0;
  }

  int callStackPush(int value)
  {
    if (callStackCursor >= MAX_STACK_CURSOR)
    {
      os_printf("MAX stack");
      paused = true;
      return -1;
    }

    if (callStack[callStackCursor] == value)
    {
      return 0;
    }

    callStack[callStackCursor++] = counter;
    return 1;
  }

  int callStackPop()
  {
    if (callStackCursor == 0)
    {
      os_printf("MIN stack");
      paused = true;
      return -1;
    }

    counter = callStack[callStackCursor--];
    return 1;
  }

  void flush()
  {
    if (printBufferCursor == 0)
    {
      return;
    }

    onSend(printBuffer, printBufferCursor);
    printBufferCursor = 0;
    os_memset(&printBuffer, 0, MAX_PRINT_BUFFER);
  }

  void putchar(char c)
  {
    if (printBufferCursor >= MAX_PRINT_CURSOR)
    {
      flush();
    }

    printBuffer[printBufferCursor++] = c;

    if (c == '\n')
    {
      flush();
    }
  }

  void putchars(const char *c, int len)
  {
    int i = 0;

    for (; i < len; i++)
    {
      putchar(c[i]);
    }
  }

  void printf(const char *format, va_list args)
  {
    char *p = (char *)format;
    char number[8];

    for (; *p; p++)
    {
      if (*p != '%')
      {
        putchar(*p);
        continue;
      }

      switch (*++p)
      {
      case 'd':
      {
        int d = va_arg(args, int);
        int c = os_sprintf(number, "%d", d);
        putchars(number, c);
        break;
      }
      case 'x':
      {
        int d = va_arg(args, int);
        int c = os_sprintf(number, "%02x", d);
        putchars(number, c);
        break;
      }
      case 'c':
      {
        char c = va_arg(args, int);
        putchar(c);
        break;
      }
      case 's':
      {
        char *s = va_arg(args, char *);
        putchars(s, strlen(s));
        break;
      }
      default:
        putchar(*p);
        break;
      }
    }
  }
};
