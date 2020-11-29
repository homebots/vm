#ifdef __cplusplus
extern "C" {
#endif

#define maxSlots 16
#define maxArguments 6

const uint8_t c_halt      = 0xff;   // -
const uint8_t c_noop      = 0x01;   // -
const uint8_t c_delay     = 0x02;   // uint32 delay
const uint8_t c_print     = 0x03;   // uint8* message
const uint8_t c_jump      = 0x04;   // uint32 address
const uint8_t c_memget    = 0x05;   // uint8 slot, int32 address
const uint8_t c_memset    = 0x06;   // uint8 slot, int32 address
const uint8_t c_push_b    = 0x07;   // uint8 slot, int8 value
const uint8_t c_push_i    = 0x08;   // uint8 slot, int32 value
const uint8_t c_gt        = 0x09;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_gte       = 0x0a;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_lt        = 0x0b;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_lte       = 0x0c;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_equal     = 0x0d;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_notequal  = 0x0e;   // uint8 slot, uint8 b, uint8 b
const uint8_t c_jumpif    = 0x0f;   // uint8 slot, uint32 address
const uint8_t c_xor       = 0x10;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_and       = 0x11;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_or        = 0x12;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_not       = 0x13;   // uint8 slot, uint8 a
const uint8_t c_inc       = 0x14;   // uint8 slot
const uint8_t c_dec       = 0x15;   // uint8 slot
const uint8_t c_add       = 0x16;   // uint8 slot, uint8 slot
const uint8_t c_rm        = 0x17;   // uint8 slot, uint8 slot

const uint8_t c_iowrite   = 0x31;   // uint8 pin, uint8 slot
const uint8_t c_ioread    = 0x32;   // uint8 slot, uint8 pin
const uint8_t c_iomode    = 0x35;   // uint8 pin, uint8 slot
const uint8_t c_iotype    = 0x36;   // uint8 pin, uint8 slot

const uint8_t slot0       = 0;
const uint8_t slot1       = 1;
const uint8_t slot2       = 2;
const uint8_t slot3       = 3;
const uint8_t slot4       = 4;
const uint8_t slot5       = 5;

const uint8_t pin0        = 0;
const uint8_t pinTx       = 1;
const uint8_t pin2        = 2;
const uint8_t pinRx       = 3;

#ifdef __cplusplus
}
#endif
