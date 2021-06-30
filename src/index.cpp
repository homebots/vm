#ifdef __cplusplus
extern "C"
{
#endif

// #define DEBUG
#define SERIAL_SPEED 115200

#include "vm.hpp"

  static ws_info webSocket;
  static Program program;
  static uint8_t bytes[] = {
      op_debug, vt_byte, 0,
      op_iotype, vt_byte, 0, vt_byte, 0,
      op_iotype, vt_byte, 2, vt_byte, 0,
      op_iomode, vt_byte, 0, vt_byte, 1,
      op_iomode, vt_byte, 2, vt_byte, 1,
      op_iowrite, vt_byte, 2, vt_byte, 1,
      op_iowrite, vt_byte, 0, vt_byte, 1,
      op_delay, vt_integer, 0x64, 0x00, 0, 0,
      op_iowrite, vt_byte, 2, vt_byte, 0,
      op_iowrite, vt_byte, 0, vt_byte, 0,
      op_delay, vt_integer, 0xe8, 0x03, 0, 0,
      op_jumpto, vt_integer, 0x17, 0, 0, 0};

  void ICACHE_FLASH_ATTR tick()
  {
    if (program.paused)
    {
      return;
    }

    while (!program.delayTime && !program.paused)
    {
      program.tick();
    }

    os_timer_arm(&program.timer, program.delayTime, 0);
    program.delayTime = 0;
  }

  void ICACHE_FLASH_ATTR user_init()
  {
    uart_div_modify(0, UART_CLK_FREQ / SERIAL_SPEED);
    gpio_init();
    system_update_cpu_freq(SYS_CPU_160MHZ);

    uint length = (uint)sizeof(bytes);
    uint i = 0;
    os_printf("\nProgram\n");

    while (i < length)
    {
      os_printf("%02x ", bytes[i++]);
    }

    program.load(bytes, length);

    os_timer_setfn(&program.timer, (os_timer_func_t *)tick, NULL);
    os_timer_arm(&program.timer, 1000, 0);
  }

#ifdef __cplusplus
}
#endif
