#ifdef __cplusplus
extern "C"
{
#endif

// #define DEBUG
#define SERIAL_SPEED 115200

#include "vm.h"

  static EspVM vm;
  static ws_info webSocket;
  static os_timer_t webSocketCheck;

  static uint8_t program[] = {
      c_wificonnect,
      'H',
      'o',
      'm',
      'e',
      'B',
      'o',
      't',
      's',
      0,
      'H',
      'o',
      'm',
      'e',
      'B',
      'o',
      't',
      's',
      0,
      c_debug,
      1,
      c_halt};

  void onReceive(struct ws_info *wsInfo, int length, char *message, int opCode)
  {
    switch (opCode)
    {
    case WS_OPCODE_BINARY:
    case WS_OPCODE_TEXT:
      LOG("LOAD %d bytes\n", length);
      vm_load(&vm, (uint8_t *)message);
      break;
    }
  }

  void connectWebSocket()
  {
    if (!vm_isWifiConnected())
      return;

    if (webSocket.connectionState != CS_CONNECTED && webSocket.connectionState != CS_CONNECTING)
    {
      LOG("Reconnecting\n");
      ws_close(&webSocket);
      ws_connect(&webSocket, "ws://hub.homebots.io/hub/display");
      return;
    }
  }

  void ICACHE_FLASH_ATTR user_init()
  {
    uart_div_modify(0, UART_CLK_FREQ / SERIAL_SPEED);
    gpio_init();
    system_update_cpu_freq(SYS_CPU_160MHZ);

    webSocket.onReceive = onReceive;
    os_timer_setfn(&webSocketCheck, (os_timer_func_t *)connectWebSocket, NULL);
    os_timer_arm(&webSocketCheck, 3000, 1);

    vm_init(&vm);
    vm_load(&vm, program);
  }

#ifdef __cplusplus
}
#endif
