# I/O instructions [0x43..0x5f]

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

