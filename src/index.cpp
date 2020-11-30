#ifdef __cplusplus
extern "C" {
#endif

#include "sdk/sdk.h"
#include "vm.h"

#define SERIAL_SPEED      115200
#define DEBUG

EspVM vm;
ws_info webSocket;
os_timer_t webSocketCheck;

static uint8_t program[] = {
  c_wificonnect,
  'H','o','m','e','B','o','t','s',0,
  'H','o','m','e','B','o','t','s',0,
  c_halt,
};

void onReceive(struct ws_info* wsInfo, int length, char *message, int opCode) {
  switch (opCode) {
    case WS_OPCODE_BINARY:
    case WS_OPCODE_TEXT:
      LOG("LOAD %d bytes\n", length);
      vm_load(&vm, (uint8_t*)message);
      vm_yield(&vm);
      break;
  }
}

void connectWebSocket() {
  LOG("Check connection\n");
  if (!vm_isWifiConnected()) {
    return;
  }

  if (webSocket.connectionState != CS_CONNECTED && webSocket.connectionState != CS_CONNECTING) {
    ws_connect(&webSocket, "ws://hub.homebots.io/hub/display");
    return;
  }
}

void ICACHE_FLASH_ATTR setup() {
  #ifndef DEBUG
  system_uart_swap();
  #endif

  webSocket.onReceive = onReceive;
  os_timer_setfn(&webSocketCheck, (os_timer_func_t *)connectWebSocket, NULL);
  os_timer_arm(&webSocketCheck, 1000, 1);

  vm_init(&vm);
  vm_load(&vm, program);
}

void ICACHE_FLASH_ATTR user_init() {
  uart_div_modify(0, UART_CLK_FREQ / SERIAL_SPEED);
  gpio_init();
  system_update_cpu_freq(SYS_CPU_160MHZ);
  setup();
}

#ifdef __cplusplus
}
#endif
