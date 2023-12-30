# I/O instructions [0x43..0x5f]

## Summary

| op code           | encoding                  | equivalent pseudocode           | description                                                                                              |
| ----------------- | ------------------------- | ------------------------------- | -------------------------------------------------------------------------------------------------------- |
| io_write           | `0x43 Byte Value`         | ioWrite(pin, value)             | write any value to pin. Any non-zero value writes `1`, otherwise `0`                                     |
| io_read            | `0x44 Identifier Byte`    | ioRead(target, pin)             | read any pin into a target slot.                                                                         |
| io_mode            | `0x45 Byte Byte`          | ioMode(pin, mode)               | set pin mode. Refer to modes table below                                                                 |
| io_type            | `0x46 Byte Byte`          | ioType(pin, type)               | set pin type. Refer to types table below                                                                 |
| io_allout          | `0x47`                    | ioAllOut()                      | set all pins to output mode.                                                                             |
| io_interrupt       | `0x48 Byte Byte Integer`  | ioInterrupt(pin, mode, address) | set interrupt in `pin`, with mode `mode`, and when triggered, jump to an address in the current program. |
| io_interruptToggle | `0x49 Byte`               | ioInterruptToggle(value)        | set interrupts on or off                                                                                 |

# Interrupt modes

Used by `io_interrupt`.

| code | type          |
| ---- | ------------- |
| 0    | disable       |
| 1    | positive edge |
| 2    | negative edge |
| 3    | any edge      |
| 4    | low level     |
| 5    | high level    |

# Pin modes

Used by `iomode`:

| pin mode | description                          |
| -------- | ------------------------------------ |
| 0        | input                                |
| 1        | output                               |
| 2        | open drain (floating pin)            |
| 3        | input with internal pull-up resistor |

# Pin types

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

## Description

### I/O Instructions Documentation

This document provides detailed information on each Input/Output (I/O) instruction supported by the virtual machine (VM). Each instruction is outlined with its specific opcode, encoding format, equivalent pseudocode, and a brief description. Examples for all accepted instruction inputs are also provided to facilitate understanding and usage.

#### 1. IO Write
- **Opcode**: `0x43`
- **Encoding**: `0x43 Byte Value`
- **Equivalent Pseudocode**: `ioWrite(pin, value)`
- **Description**: Writes any value to a specified pin. Any non-zero value writes `1`, otherwise `0`.
- **Example**:
  ```
  ioWrite(5, 1) // Write `1` to pin 5
  ```

#### 2. IO Read
- **Opcode**: `0x44`
- **Encoding**: `0x44 Identifier Byte`
- **Equivalent Pseudocode**: `ioRead(target, pin)`
- **Description**: Reads the value of any pin into a target slot.
- **Example**:
  ```
  ioRead(x, 3) // Read pin 3 into variable x
  ```

#### 3. IO Mode
- **Opcode**: `0x45`
- **Encoding**: `0x45 Byte Byte`
- **Equivalent Pseudocode**: `ioMode(pin, mode)`
- **Description**: Sets the mode of a pin. Refer to the Pin Modes table for mode details.
- **Example**:
  ```
  ioMode(4, 1) // Set pin 4 to output mode
  ```

#### 4. IO Type
- **Opcode**: `0x46`
- **Encoding**: `0x46 Byte Byte`
- **Equivalent Pseudocode**: `ioType(pin, type)`
- **Description**: Sets the type of a pin. Refer to the Pin Types table for type details.
- **Example**:
  ```
  ioType(2, 0) // Set pin 2 to GPIO 2
  ```

#### 5. IO All Out
- **Opcode**: `0x47`
- **Encoding**: `0x47`
- **Equivalent Pseudocode**: `ioAllOut()`
- **Description**: Sets all pins to output mode.
- **Example**:
  ```
  ioAllOut()
  ```

#### 6. IO Interrupt
- **Opcode**: `0x48`
- **Encoding**: `0x48 Byte Byte Integer`
- **Equivalent Pseudocode**: `ioInterrupt(pin, mode, address)`
- **Description**: Sets an interrupt on a pin, with mode `mode`, and when triggered, jumps to an address in the current program. Refer to the Interrupt Modes table for mode details.
- **Example**:
  ```
  ioInterrupt(6, 1, 0x01F4) // Set a positive edge interrupt on pin 6, jump to address 0x01F4 when triggered
  ```

#### 7. IO Interrupt Toggle
- **Opcode**: `0x49`
- **Encoding**: `0x49 Byte`
- **Equivalent Pseudocode**: `ioInterruptToggle(value)`
- **Description**: Toggles interrupts on or off.
- **Example**:
  ```
  ioInterruptToggle(true) // Enable interrupts
  ```

### Interrupt Modes (For `io_interrupt`)
- **0**: Disable
- **1**: Positive Edge
- **2**: Negative Edge
- **3**: Any Edge
- **4**: Low Level
- **5**: High Level

### Pin Modes (For `io_mode`)
- **0**: Input
- **1**: Output
- **2**: Open Drain (Floating Pin)
- **3**: Input with Internal Pull-up Resistor

### Pin Types (For `io_type`)
- **0 (GPIO 0)**
  - Pin 0: GPIO 0
- **1 (TX 0)**
  - Pin 1: TX 0
  - Pin 2: GPIO 1
- **2 (TX 1)**
  - Pin 2: GPIO 2
  - Pin 2: TX 1
  - Pin 2: TX 0
- **3 (GPIO 3)**
  - Pin 3: RX 0
  - Pin 3: GPIO 3
