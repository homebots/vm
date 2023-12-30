# Memory/IO instructions [0x40..0x42]

## Summary

| op code           | encoding                  | equivalent pseudocode           | description                                                                                              |
| ----------------- | ------------------------- | ------------------------------- | -------------------------------------------------------------------------------------------------------- |
| memget            | `0x40 Identifier Address` | memoryGet(target, address)      | read a value from memory into a slot                                                                     |
| memset            | `0x41 Address Value`      | memorySet(address, value)       | write any value to a memory address                                                                      |

### Memory/IO Instructions Documentation

This document provides detailed information on the Memory/IO instructions supported by the virtual machine (VM). Each instruction is outlined with its specific opcode, encoding format, equivalent pseudocode, and a brief description. Examples for all accepted instruction inputs are also provided to facilitate understanding and usage.

#### 1. Memory Get (memget)
- **Opcode**: `0x40`
- **Encoding**: `0x40 Identifier Address`
- **Equivalent Pseudocode**: `memoryGet(target, address)`
- **Description**: Reads a value from memory into a specified target slot.
- **Example**:
  ```
  memoryGet(x, 0x03E8) // Read value from memory address 0x03E8 into variable x
  ```

#### 2. Memory Set (memset)
- **Opcode**: `0x41`
- **Encoding**: `0x41 Address Value`
- **Equivalent Pseudocode**: `memorySet(address, value)`
- **Description**: Writes any value to a specified memory address.
- **Example**:
  ```
  memorySet(0x03E8, 123) // Write the value 123 to memory address 0x03E8
  ```