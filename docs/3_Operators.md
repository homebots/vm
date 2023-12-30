# Variable/Value and arithmetic instructions [0x20..0x3f]

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