# Memory/IO instructions [0x40..0x42]

| op code           | encoding                  | equivalent pseudocode           | description                                                                                              |
| ----------------- | ------------------------- | ------------------------------- | -------------------------------------------------------------------------------------------------------- |
| memget            | `0x40 Identifier Address` | memoryGet(target, address)      | read a value from memory into a slot                                                                     |
| memset            | `0x41 Address Value`      | memorySet(address, value)       | write any value to a memory address                                                                      |
