# System instructions [0x01..0x1f]

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
