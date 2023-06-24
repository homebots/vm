# Homebots VM

Virtual Machine to execute programs on an esp8266 without the need to flash a new firmware.

## Build and flash

### From prebuilt binaries

See [releases](https://github.com/homebots/vm/releases).

The firmwares from releases are separated into two parts.
Each file has the flash address in its name, like `0x10000.bin`.
Use that to correctly flash the firmware parts.


Command example:

```sh
esptool.py write_flash --compress --flash_freq 80m -fm qio -fs 1MB 0x10000 0x10000.bin
```

After flashing, the ESP8266 will start in both Access Point and Wifi mode.
The AP mode creates an open WIFI called `Homebots_AP`, for local testing.
The VM opens an HTTP server at port 3000. Connect to this network and use `http://192.168.4.1:3000` to interact with the machine.

### From source

This is the preferred way to use the VM, as you probably want to change the WIFI SSID and password for your devices.

```bash
export WIFI_SSID="My_Network"
export WIFI_PASSWORD="SecretCode123"

make && make flash
```

See the [`Makefile`](Makefile) for more details on how each command works.

## Concepts

- every instruction has a unique code, which is 1-byte long
- every instruction can have zero or more operands
- each operand can be of type `Byte`, `Number`, `Integer`, `String` or `Null`.
- the `Byte` type is also used to represent IO pins or a memory slot to store variable data. Valid pins are `0`, `1`, `2` or `3`.
- the `Integer` is also used to represent memory addresses.
- values are encoded as a byte with their type, followed by their content.
  An operator that accepts any type declares its operands as `Value`, to represent any value.

These are the valid _data_ types:

| type             | encoding: type and data    | length         | Pseudocode |
| ---------------- | -------------------------- | -------------- | ---------- |
| null             | `0x00`                     | 1              | `Null`     |
| identifier       | `0x01 byte`                | 2              | `Byte`     |
| byte             | `0x02 byte`                | 2              | `Byte`     |
| pin              | `0x03 0x00-0x03`           | 2              | `Byte`     |
| address          | `0x04 byte byte byte byte` | 5              | `Integer`  |
| unsigned integer | `0x05 byte byte byte byte` | 5              | `Integer`  |
| signed integer   | `0x06 byte byte byte byte` | 5              | `Number`   |
| string           | `0x07 bytes ... 0x00`      | str length + 2 | `String`   |

- multi-byte numbers, like integers and addresses, are LE encoded. That means the bytes are in reverse order.
  For example, `1000` decimal is encoded as `e8 03 00 00`. In hex, `e=14`, so `14 * 16 + 8` on first byte, plus `256 * 3` from second byte, which equals to `1000`.
- String is encoded as a sequence of bytes. The last byte is always a null byte (`0x00`).

## Instructions

Each data type is represented a sequence of bytes. They always begin with `type`, followed by the bytes of that type.

For example, an unsigned integer with value 1 corresponds to bytes `0x05 0x01 0x00 0x00 0x00` and is declared as `Integer inputName`.

Additionally, to make a reference to memory slots, the type `Identifier` is used as an operand.

## System instructions [0x01..0x1f]

| op code    | encoding             | equivalent pseudocode      | description                                                                                     |
| ---------- | -------------------- | -------------------------- | ----------------------------------------------------------------------------------------------- |
| noop       | `0x01`               | noop()                     | do nothing                                                                                      |
| halt       | `0x02`               | halt()                     | stop program                                                                                    |
| restart    | `0x03`               | restart()                  | restart program                                                                                 |
| systeminfo | `0x04`               | systemInfo()               | print system details to serial output (chip ID, SDK version, local time and memory information) |
| debug      | `0x05 Byte`          | debug(enabled)             | swap UART 0 with UART 1 if `false`, swap back to UART 0 if `true`, for serial output            |
| dump       | `0x06`               | dump()                     | print all bytes of the program running                                                          |
| yield      | `0x07`               | delay(interval)            | delay the execution of the next instruction. time in milliseconds                               |
| delay      | `0x08 Integer`       | delay(interval)            | delay the execution of the next instruction. time in milliseconds                               |
| print      | `0x09 Value`         | print(value)               | prints any value to the serial output                                                           |
| jumpto     | `0x0a Integer`       | jumpTo(address)            | jump to any address of the current program                                                      |
| jumpif     | `0x0b Value Integer` | jumpIf(condition, address) | jump to any address of the current program if condition is truthy                               |
| sleep      | `0x0c Integer`       | sleep(time)                | put the esp8266 into deep sleep mode for a given time in milliseconds                           |

## Variable/Value and arithmetic instructions [0x20..0x3f]

**Binary operations**

All binary operations have the same format: store in `Identifier` the result of `Value OPERATOR Value`

| op code  | encoding                      | equivalent pseudocode |
| -------- | ----------------------------- | --------------------- |
| gt       | `0x20 Identifier Value Value` | a = b > 2             |
| gte      | `0x21 Identifier Value Value` | a = b >= 2            |
| lt       | `0x22 Identifier Value Value` | a = b < 2             |
| lte      | `0x23 Identifier Value Value` | a = b <= 2            |
| equal    | `0x24 Identifier Value Value` | a = b == 2            |
| notequal | `0x25 Identifier Value Value` | a = b != 2            |
| xor      | `0x26 Identifier Value Value` | a = b ^ 2             |
| and      | `0x27 Identifier Value Value` | a = b & 2             |
| or       | `0x28 Identifier Value Value` | a = b \| 2            |
| add      | `0x29 Identifier Value Value` | a = b + 2             |
| sub      | `0x2a Identifier Value Value` | a = b - 2             |
| mul      | `0x2b Identifier Value Value` | a = b \* 2            |
| div      | `0x2c Identifier Value Value` | a = b / 2             |
| mod      | `0x2d Identifier Value Value` | a = b % 2             |

**Unary operations**

The binary operation NOT have this format: store in `Identifier` the result of `! Value`
Increase and Decrease work on mutating an `Identifier` in place

| op code | encoding                | equivalent pseudocode |
| ------- | ----------------------- | --------------------- |
| not     | `0x2e Identifier Value` | a = !b                |
| inc     | `0x2f Identifier`       | a++                   |
| dec     | `0x30 Identifier`       | a--                   |

**Declare/assign value operations**

| op code | encoding                | equivalent pseudocode | description                         |
| ------- | ----------------------- | --------------------- | ----------------------------------- |
| assign  | `0x31 Identifier Value` | assign(target, value) | assign a new value to a memory slot |
| declare | `0x32 Byte Byte`        | declare(slotId, type) | declare the type of a memory slot   |

## Memory/IO instructions [0x40..0x5f]

| op code           | encoding                  | equivalent pseudocode           | description                                                                                              |
| ----------------- | ------------------------- | ------------------------------- | -------------------------------------------------------------------------------------------------------- |
| memget            | `0x40 Identifier Address` | memoryGet(target, address)      | read a value from memory into a slot                                                                     |
| memset            | `0x41 Address Value`      | memorySet(address, value)       | write any value to a memory address                                                                      |
| iowrite           | `0x43 Byte Value`         | ioWrite(pin, value)             | write any value to pin. Any non-zero value writes `1`, otherwise `0`                                     |
| ioread            | `0x44 Identifier Byte`    | ioRead(target, pin)             | read any pin into a target slot.                                                                         |
| iomode            | `0x45 Byte Byte`          | ioMode(pin, mode)               | set pin mode. Refer to modes table below                                                                 |
| iotype            | `0x46 Byte Byte`          | ioType(pin, type)               | set pin type. Refer to types table below                                                                 |
| ioallout          | `0x47`                    | ioAllOut()                      | set all pins to output mode.                                                                             |
| iointerrupt       | `0x48 Byte Byte Integer`  | ioInterrupt(pin, mode, address) | set interrupt in `pin`, with mode `mode`, and when triggered, jump to an address in the current program. |
| iointerruptToggle | `0x49 Byte`               | ioInterruptToggle(value)        | set interrupts on or off                                                                                 |



## Pin modes

Used by `iomode`:

| pin mode | description                          |
| -------- | ------------------------------------ |
| 0        | input                                |
| 1        | output                               |
| 2        | open drain (floating pin)            |
| 3        | input with internal pull-up resistor |

## Pin types

Used by `iotype`. There are specific functions that can be associated with certain IO pins.

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

## Interrupt modes

Used by `ioInterrupt`.

| code | type          |
| ---- | ------------- |
| 0    | disable       |
| 1    | positive edge |
| 2    | negative edge |
| 3    | any edge      |
| 4    | low level     |
| 5    | high level    |

## Program memory

A program is just a sequence of bytes, representing both instructions and data.

To store and manipulate variables, a program can create up to 255 memory slots. That means **a program can have up to 255 variables in total**.

Each variable is represented as a `Value`.

Example:

```esp
// declare a variable of type `integer` at slot `0` with initial value `1`:
// declare = 0x0d
// 0x02 is vtype "byte"
// 0x05 is vtype "int"
0d 02 00 05 01 00 00 00

// store the unsigned integer 1000 in the memory slot
// assign = 0x31
31 02 00 05 e8 03 00 00,

// jump to the beginning of the program
// jump = 0x0a
// Integer 1 = 0x05 0x00 0x00 0x00 0x01

0a 05 00 00 00 00,
```
