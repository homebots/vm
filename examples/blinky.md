```c
static uint8_t program[] = {
  c_ioread, slot0, pin2,
  c_not, slot1, slot0,
  c_iowrite, pin0, slot1,
  c_iowrite, pin2, slot1,
  c_delay, 0x80, 0x01, 0x00, 0x00,
  c_jumpto, 0x00, 0x00, 0x00, 0x00,
  c_halt
};

```