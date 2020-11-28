#ifdef __cplusplus
extern "C" {
#endif

const uint8_t c_halt      = 0xff;
const uint8_t c_noop      = 0x01;
const uint8_t c_delay     = 0x02;   // uint32 delay
const uint8_t c_print     = 0x03;   // char* message
const uint8_t c_jumpto    = 0x04;   // uint32 address
const uint8_t c_memget    = 0x05;   // uint8 slot, int32 address
const uint8_t c_memset    = 0x06;   // uint8 slot, int32 address
const uint8_t c_push_b    = 0x07;   // uint8 slot, int8 value
const uint8_t c_push_i    = 0x08;   // uint8 slot, int32 value
const uint8_t c_if        = 0x09;   // uint8 slot, uint8 slot
const uint8_t c_ifnot     = 0x0a;   // uint8 slot, uint8 slot

const uint8_t c_xor       = 0x10;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_and       = 0x11;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_or        = 0x12;   // uint8 slot, uint8 a, uint8 b
const uint8_t c_not       = 0x13;   // uint8 slot, uint8 a
const uint8_t c_iowrite   = 0x31;   // uint8 pin, uint8 slot
const uint8_t c_ioread    = 0x32;   // uint8 slot, uint8 pin
// const uint8_t c_ioset     = 0x33;   // uint8 slot
// const uint8_t c_ioget     = 0x34;   // uint8 slot
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

uint8_t vm_pinRead(uint8_t* pin) {
  return GPIO_INPUT_GET(*pin) > 0 ? 1 : 0;
}

void vm_pinWrite(uint8_t* pin, uint8_t* value) {
  GPIO_OUTPUT_SET(*pin, *value > 0 ? 1 : 0);
}

void vm_pinType(uint8_t* pin, uint8_t* mode) {
  PIN_FUNC_SELECT(PERIPHS_IO_MUX + (*pin * 4), *mode);
}

void vm_pinMode(uint8_t* pin, uint8_t* mode) {
  pinMode(*pin, (PinMode)*mode);
}

#ifdef __cplusplus
}
#endif
