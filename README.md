# Homebots VM

Virtual Machine to execute programs on an esp8266 without the need to flash a new firmware.

## Build and flash

### From prebuilt binaries

See [releases](https://github.com/homebots/vm/releases).

#### From source

```bash
$ make && make flash
```

See [`Makefile`](Makefile) for more options.

## Concepts

- every instruction has a unique opcode and is 1-byte long
- every instruction can have zero or more operands
- operands are of type `byte`, `integer`, `unsigned integer`, or `string`.
  `byte` is also used to represent IO pins or a memory slot. Valid pins are `0`, `1`, `2` or `3`.
  `integer` is also used to represent memory addresses.

- types are encoded as a `Value` type, with these data types:

| type           | encoding: type + data      | length         |
| -------------- | -------------------------- | -------------- |
| null           | `0x00`                     | 1              |
| identifier     | `0x01 byte`                | 2              |
| byte           | `0x02 byte`                | 2              |
| pin            | `0x03 0x00-0x03`           | 2              |
| address        | `0x04 byte byte byte byte` | 5              |
| integer        | `0x05 byte byte byte byte` | 5              |
| signed integer | `0x06 byte byte byte byte` | 5              |
| string         | `0x07 bytes ... 0x00`      | str length + 2 |

- multi-byte numbers, like integers and addresses, are LE encoded. That means the bytes are flipped.
  For example, `1000` decimal is encoded as `e8 03 00 00`.
- String is encoded as a sequence of bytes. The last byte is always a null byte (`0x00`).

## Instructions

Note: `Value(type)` represents a sequence of bytes representing the data defined as `type`, followed by the data bytes of that type.

For example, an unsigned integer with value 1 corresponds to bytes `0x05 0x01 0x00 0x00 0x00` and is declared as `Value(int) inputName`

| op code    | encoding                                | equivalent pseudocode      | description                                                                                     |
| ---------- | --------------------------------------- | -------------------------- | ----------------------------------------------------------------------------------------------- |
| noop       | `0x01`                                  | noop()                     | do nothing                                                                                      |
| halt       | `0x02`                                  | halt()                     | stop program                                                                                    |
| restart    | `0x03`                                  | restart()                  | restart program                                                                                 |
| systeminfo | `0x04`                                  | systemInfo()               | print system details to serial output (chip ID, SDK version, local time and memory information) |
| debug      | `0x05 Value(bool)`                      | debug(enabled)             | swap UART 0 with UART 1 if `false`, swap back to UART 0 if `true`, for serial output            |
| dump       | `0x06`                                  | dump()                     | print all bytes of the program running                                                          |
| delay      | `0x08 Value(int)`                       | delay(interval)            | delay the execution of the next instruction. time in milliseconds                               |
| print      | `0x09 Value(*)`                         | print(value)               | prints any value to the serial output                                                           |
| jumpto     | `0x0a Value(int)`                       | jumpTo(address)            | jump to any address of the current program                                                      |
| jumpif     | `0x0b Value(*) Value(int)`              | jumpIf(condition, address) | jump to any address of the current program if condition is truthy                               |
| sleep      | `0x0c Value(int)`                       | sleep(time)                | put the esp8266 into deep sleep mode for a given time in milliseconds                           |
| declare    | `0x0d byte byte`                        | declare(slotId, type)      | declare the type of a memory slot                                                               |
| assign     | `0x31 Value(identifier) Value(*)`       | assign(target, value)      | assign a new value to a memory slot                                                             |
| memget     | `0x40 Value(identifier) Value(address)` | memoryGet(target, address) | read a value from memory into a slot                                                            |
| memset     | `0x41 Value(address) Value(*)`          | memorySet(address, value)  | write any value to a memory address                                                             |
| iowrite    | `0x43 Value(byte) Value(*)`             | ioWrite(pin, value)        | write any value to pin. Any non-zero value writes `1`, otherwise `0`                            |
| ioread     | `0x44 Value(identifier) Value(byte)`    | ioRead(target, pin)        | read any pin into a target slot.                                                                |
| iomode     | `0x45 Value(byte) Value(byte)`          | ioWrite(pin, mode)         | set pin mode. Refer to modes table below                                                        |
| iotype     | `0x46 Value(byte) Value(byte)`          | ioWrite(pin, type)         | set pin type. Refer to types table below                                                        |
| ioallout   | `0x47`                                  | ioAllOut()                 | set all pins to output mode.                                                                    |

## Arithmetic instructions

**Binary operations**

| op code  | encoding                                   | equivalent pseudocode |
| -------- | ------------------------------------------ | --------------------- |
| gt       | `0x20 Value(identifier) Value(*) Value(*)` | a = b > 2             |
| gte      | `0x21 Value(identifier) Value(*) Value(*)` | a = b >= 2            |
| lt       | `0x22 Value(identifier) Value(*) Value(*)` | a = b < 2             |
| lte      | `0x23 Value(identifier) Value(*) Value(*)` | a = b <= 2            |
| equal    | `0x24 Value(identifier) Value(*) Value(*)` | a = b == 2            |
| notequal | `0x25 Value(identifier) Value(*) Value(*)` | a = b != 2            |
| xor      | `0x26 Value(identifier) Value(*) Value(*)` | a = b ^ 2             |
| and      | `0x27 Value(identifier) Value(*) Value(*)` | a = b & 2             |
| or       | `0x28 Value(identifier) Value(*) Value(*)` | a = b \| 2            |
| add      | `0x29 Value(identifier) Value(*) Value(*)` | a = b + 2             |
| sub      | `0x2a Value(identifier) Value(*) Value(*)` | a = b - 2             |
| mul      | `0x2b Value(identifier) Value(*) Value(*)` | a = b \* 2            |
| div      | `0x2c Value(identifier) Value(*) Value(*)` | a = b / 2             |
| mod      | `0x2d Value(identifier) Value(*) Value(*)` | a = b % 2             |

**Unary operations**

| op code | encoding                          | equivalent pseudocode |
| ------- | --------------------------------- | --------------------- |
| not     | `0x2e Value(identifier) Value(*)` | a = !b                |
| inc     | `0x2f Value(identifier)`          | a++                   |
| dec     | `0x30 Value(identifier)`          | a--                   |

## Pin modes and types

| pin mode | description                          |
| -------- | ------------------------------------ |
| 0        | input                                |
| 1        | output                               |
| 2        | open drain (floating pin)            |
| 3        | input with internal pull-up resistor |

| pin | type       |
| --- | ---------- |
| 0   | 0 = gpio 0 |
| 1   | 0 = tx 0   |
|     | 3 = gpio 1 |
| 2   | 0 = gpio 2 |
|     | 2 = tx 1   |
|     | 4 = tx 0   |
| 3   | 0 = rx 0   |
|     | 3 = gpio 3 |

## Program memory

A program is just a sequence of bytes, representing both instructions and data.

To store data locally as variables, a program can create up to 255 memory slots.

Each slot is represented as a `Value` object.

Example:

```
// declare a memory slot with type `integer`

0x0d, 0x00, 0x01,

// store the unsigned integer 1000 in the memory slot
// assign = 0x31
0x31, 0x00, 0x05, 0xe8, 0x03, 0x00, 0x00,

// jump to the beginning of the program
// jump = 0x0a
// Value(int) 1 = 0x05 0x00 0x00 0x00 0x01

0x0a, 0x05, 0x00, 0x00, 0x00, 0x00,
```
