# Concepts

- every instruction has a unique code, which is 1-byte long
- every instruction can have zero or more operands
- each operand can be of type `Byte`, `Number`, `Integer`, `String` or `Null`.
- the `Byte` type is also used to represent IO pins or a memory slot to store variable data. Valid pins are `0`, `1`, `2` or `3`.
- the `Integer` is also used to represent memory addresses.
- values are encoded as a byte with their type, followed by their content.
  An operator that accepts any type declares its operands as `Value`, to represent any value.

# Data types

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

Each data type is represented a sequence of bytes. They always begin with a single byte (the `type` byte), followed by the bytes of that type.

For example, an unsigned integer with value 1 corresponds to bytes `0x05 0x01 0x00 0x00 0x00` and is declared as `Integer`.

Additionally, to make a reference to memory slots, the type `Identifier` is used as an operand.

# Functions and jump table

Functions are defined in a jump table.

Later in the program, this table is used by function calls or jump instructions to move from one function to another.

Every function has a name, defined after a `def` keyword and before a `:` in one line, followed by a body with at least one instruction, terminated by the `end` keyword.
