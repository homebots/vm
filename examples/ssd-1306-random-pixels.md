0   row
1   ack
2   maxWidth
3   cursor
4   pixel
5   device
6   jump
7   maxHeight

  c_push_b,
  slot0,
  0xb0,
  c_push_b,
  slot7,
  0xb8,
  c_push_b,
  slot2,
  132,
  c_i2setup,
  c_i2find,
  slot5,
  c_i2start,
  c_i2write,
  slot5,
  c_i2getack,
  slot1,
  c_i2writeack, // init display
  0x1c,
  0x0,
  0x0,
  0x0,
  0x0,
  0xae,
  0xd5,
  0x80,
  0xa8,
  0x3f,
  0xd3,
  0x0,
  0x40,
  0x8d,
  0x14,
  0x20,
  0x2,
  0xa1,
  0xc8,
  0x0,
  0x10,
  0xda,
  0x12,
  0x81,
  0xcf,
  0xd9,
  0xf1,
  0xdb,
  0x20,
  0xa4,
  0xa6,
  0xaf,
  c_i2stop,
  c_i2start,
  c_i2write,
  slot5,
  c_i2getack,
  slot1,
  c_i2writeack_b,   // write row
  0,
  c_i2write,
  slot0,
  c_i2getack,
  slot1,
  c_i2stop,
  c_i2start,
  c_i2write,
  5,
  c_i2getack,
  1,
  c_i2write,        // write pixel
  4,
  c_delay,
  100,
  0,
  0,
  0,
  c_i2getack,
  1,
  c_inc,            // add pixel
  4,
  c_inc,            // add cursor
  3,
  c_lte,
  6,
  3,
  2,
  c_jumpif,         // write another pixel
  6,
  69,
  0,
  0,
  0,
  c_i2stop,
  c_gte,
  6,
  0,
  7,
  c_jumpif,
  6,
  0,
  0,
  0,
  0,
  c_inc,
  0,
  c_jump,
  51,
  c_halt,