# Homebots VM

Virtual Machine to execute programs on an esp8266 without the need to flash a new firmware.

## Usage

```bash
$ make && make flash
```

See the makefile for more options. This is a work in progress.

## Concepts

- each instruction has a unique opcode and is 1-byte long
- each instruction can have zero or more operands
- operands are either byte, integer, unsigned integer, or string
- operands are encoded as follows:

| type           | encoding: type + data       | length         |
| -------------- | --------------------------- | -------------- |
| null           | `0x00`                      | 1              |
| identifier     | `0x01 byte`                 | 2              |
| byte           | `0x02 byte`                 | 2              |
| pin            | `0x03 0x00-0x0f`            | 2              |
| address        | `0x04 byte byte byte byte ` | 5              |
| integer        | `0x05 byte byte byte byte`  | 5              |
| signed integer | `0x06 byte byte byte byte`  | 5              |
| string         | `0x07 bytes ... 0x00`       | str length + 2 |

Integer and address are LE encoded. For example, `1000` decimal is encoded as `e8 03 00 00`.

String is encoded as a sequence of bytes. The last byte is always a null byte.
