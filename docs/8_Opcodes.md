# All opcodes

| code              | byte |
| ----------------- | ---- |
| noop              | 0x01 |
| halt              | 0x02 |
| restart           | 0x03 |
| systeminfo        | 0x04 |
| debug             | 0x05 |
| dump              | 0x06 |
| yield             | 0x07 |
| delay             | 0x08 |
| print             | 0x09 |
| jumpto            | 0x0a |
| jumpif            | 0x0b |
| sleep             | 0x0c |
| gt                | 0x20 |
| gte               | 0x21 |
| lt                | 0x22 |
| lte               | 0x23 |
| equal             | 0x24 |
| notequal          | 0x25 |
| xor               | 0x26 |
| and               | 0x27 |
| or                | 0x28 |
| add               | 0x29 |
| sub               | 0x2a |
| mul               | 0x2b |
| div               | 0x2c |
| mod               | 0x2d |
| not               | 0x2e |
| inc               | 0x2f |
| dec               | 0x30 |
| assign            | 0x31 |
| declare           | 0x32 |
| memget            | 0x40 |
| memset            | 0x41 |
| iowrite           | 0x43 |
| ioread            | 0x44 |
| iomode            | 0x45 |
| iotype            | 0x46 |
| ioallout          | 0x47 |
| iointerrupt       | 0x48 |
| iointerruptToggle | 0x49 |
| wifistatus        | 0x60 |
| wifiap            | 0x61 |
| wificonnect       | 0x62 |
| wifidisconnect    | 0x63 |
| wifilist          | 0x64 |
| i2setup           | 0x70 |
| i2start           | 0x71 |
| i2stop            | 0x72 |
| i2write           | 0x73 |
| i2read            | 0x74 |
| i2setack          | 0x75 |
| i2getack          | 0x76 |
| i2find            | 0x77 |
| i2writeack        | 0x78 |

# All value types

| type          | byte |
| ------------- | ---- |
| null          | 0    |
| identifier    | 1    |
| byte          | 2    |
| pin           | 3    |
| address       | 4    |
| integer       | 5    |
| signedInteger | 6    |
| string        | 7    |
