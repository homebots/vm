# System instructions [0x01..0x1f]

This document provides detailed information on each system instruction supported by the virtual machine (VM). Each instruction is outlined with its specific opcode, encoding format, equivalent pseudocode, and a brief description. Examples for all accepted instruction inputs are also provided to facilitate understanding and usage.

## Summary

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
| def        | `0x0f 0x00 0x01`     | def abc:                   | define the start of a function                                                                  |


## System Instructions Documentation

#### 1. No-operation (noop)
- **Opcode**: `0x01`
- **Encoding**: `0x01`
- **Equivalent Pseudocode**: `noop()`
- **Description**: Performs no operation. It is typically used for timing adjustments or as a placeholder.
- **Example**:
  ```
  noop()
  ```

#### 2. Halt
- **Opcode**: `0x02`
- **Encoding**: `0x02`
- **Equivalent Pseudocode**: `halt()`
- **Description**: Stops the program execution immediately.
- **Example**:
  ```
  halt()
  ```

#### 3. Restart
- **Opcode**: `0x03`
- **Encoding**: `0x03`
- **Equivalent Pseudocode**: `restart()`
- **Description**: Restarts the program from the beginning.
- **Example**:
  ```
  restart()
  ```

#### 4. System Information
- **Opcode**: `0x04`
- **Encoding**: `0x04`
- **Equivalent Pseudocode**: `systemInfo()`
- **Description**: Prints system details to serial output, including chip ID, SDK version, local time, and memory information.
- **Example**:
  ```
  systemInfo()
  ```

#### 5. Debug
- **Opcode**: `0x05`
- **Encoding**: `0x05 Byte`
- **Equivalent Pseudocode**: `debug(enabled)`
- **Description**: Swaps UART 0 with UART 1 if `false`, and swaps back to UART 0 if `true`, for serial output.
- **Example**:
  ```
  debug(true)
  debug(false)
  debug(on)
  debug(off)
  debug(0)
  debug(0x01)
  ```

#### 6. Dump
- **Opcode**: `0x06`
- **Encoding**: `0x06`
- **Equivalent Pseudocode**: `dump()`
- **Description**: Prints all bytes of the program currently running.
- **Example**:
  ```
  dump()
  ```

#### 7. Yield
- **Opcode**: `0x07`
- **Encoding**: `0x07`
- **Equivalent Pseudocode**: `yield()`
- **Description**: Delays the execution of the next instruction by one cycle. This allows the VM to step out of the execution loop and continue after 1ms
- **Example**:
  ```
  yield()
  ```

#### 8. Delay
- **Opcode**: `0x08`
- **Encoding**: `0x08 Integer`
- **Equivalent Pseudocode**: `delay(interval)`
- **Description**: Similar to `yield`, it delays the execution of the next instruction for a specified time in milliseconds.
- **Example**:
  ```
  delay(500) // 500 milliseconds
  ```

#### 9. Print
- **Opcode**: `0x09`
- **Encoding**: `0x09 Value`
- **Equivalent Pseudocode**: `print(value)`
- **Description**: Prints any value to the serial output.
- **Example**:
  ```
  print("Hello World")
  print(1234)
  ```

#### 10. Jump To
- **Opcode**: `0x0a`
- **Encoding**: `0x0a Integer`
- **Equivalent Pseudocode**: `jumpTo(address)`
- **Description**: Jumps to any memory address, starting at the beginning of the current program.
- **Example**:
  ```
  jump to 0x01f40000 // Jump to address 0x01f40000
  ```

#### 11. Jump If
- **Opcode**: `0x0b`
- **Encoding**: `0x0b Value Integer`
- **Equivalent Pseudocode**: `jumpIf(condition, address)`
- **Description**: Jumps to any address of the current program if the condition is truthy.
- **Example**:
  ```
  jumpIf(x > 10, 0x03E80000)
  ```

#### 12. Sleep
- **Opcode**: `0x0c`
- **Encoding**: `0x0c Integer`
- **Equivalent Pseudocode**: `sleep(time)`
- **Description**: Puts the ESP8266 into deep sleep mode for a given time in milliseconds.
- **Example**:
  ```
  sleep(10000) // Sleep for 10 seconds
  ```

#### 13. Define Function
- **Opcode**: `0x0f`
- **Encoding**: `0x0f 0x00 0x01`
- **Equivalent Pseudocode**: `def abc:`
- **Description**: Defines the start of a function.
- **Example**:
  ```
  def abc:
  // Function body
  ```
