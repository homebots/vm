```
@BEGIN
  not       #0, #0,
  iowrite   pin 0, #0,
  iowrite   pin 2, #0,
  delay     500
  jump to BEGIN
```

```
// declare byte
declare(0x00  0x02)

// ioWrite byte to pin 0
ioWrite(0x00  0x01 0x00)

// ioWrite byte to pin 2
ioWrite(0x02 0x01 0x00)

// invert byte
not(0x01 0x00 0x01 0x00)
```
