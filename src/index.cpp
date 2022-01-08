// #define DEBUG
#define SERIAL_SPEED 115200

#include "homebots.h"
#include "vm.hpp"
#include "espconn.h"

static ws_info webSocket;
static Program program;
static Wifi wifi;
static os_timer_t webSocketCheck;
static char stdout[1024];
static uint stdout_len = 0;

bool ICACHE_FLASH_ATTR ws_isConnected(ws_info *webSocket)
{
  return webSocket->connectionState != CS_CONNECTED && webSocket->connectionState != CS_CONNECTING;
}

LOCAL void ICACHE_FLASH_ATTR putc(char c)
{
  if (!ws_isConnected(&webSocket))
  {
    return;
  }

  if (c != '\n' && stdout_len < sizeof(stdout))
  {
    stdout[stdout_len] = c;
    stdout_len++;
    return;
  }

  stdout[stdout_len] = 0;
  stdout_len = 0;
  ws_send(&webSocket, WS_OPCODE_TEXT, stdout, stdout_len);
}

void ICACHE_FLASH_ATTR onReceive(struct ws_info *wsInfo, int length, char *message, int opCode)
{
  if (length && opCode == WS_OPCODE_BINARY || opCode == WS_OPCODE_TEXT)
  {
    program.load((uint8_t *)message, length);
    os_timer_arm(&program.timer, 10, 0);
  }
}

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

void reconnectWebSocket()
{
  if (!wifi.isConnected())
  {
    return;
  }

  if (ws_isConnected(&webSocket))
  {
    ws_close(&webSocket);
    ws_connect(&webSocket, "ws://hub.homebots.io/hub/homebots");
    return;
  }
}

void ICACHE_FLASH_ATTR setup()
{
  webSocket.onReceive = onReceive;
  os_timer_setfn(&webSocketCheck, (os_timer_func_t *)reconnectWebSocket, NULL);
  os_timer_arm(&webSocketCheck, 3000, 1);

  os_timer_setfn(&program.timer, (os_timer_func_t *)tick, NULL);

  wifi.startAccessPoint("HomeBot");
}
