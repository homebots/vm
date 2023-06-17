#define WITH_DEBUG
#define SERIAL_SPEED 115200

#include "homebots.h"
#include "vm.hpp"

// WIFI_SSID and WIFI_SSID should be defined as environment variables
#ifndef WIFI_SSID
#define WIFI_SSID "HomeBots"
#define WIFI_PASSWORD ""
#endif

#ifdef WITH_DEBUG
#define TRACE(...) os_printf(__VA_ARGS__);
#else
#define TRACE(...)
#endif

static Program program;
static os_timer_t wifiTimer;
static struct espconn *conn;

void checkAgain()
{
  os_timer_arm(&wifiTimer, 5000, 0);
}

void checkConnection(void *arg)
{
  espconn *conn = (espconn *)arg;
  if (!wifi.isConnected())
  {
    TRACE("wifi off\n");
    if (ESPCONN_CLOSE != conn->state)
    {
      espconn_disconnect(conn);
    }

    checkAgain();
    return;
  }

  if (conn->state != ESPCONN_LISTEN)
  {
    TRACE("not started: %d\n", conn->state);
    espconn_accept(conn);
    checkAgain();
    return;
  }

  switch (conn->state)
  {
  case ESPCONN_NONE:
    TRACE("Not started\n");
    break;

  case ESPCONN_WAIT:
    TRACE("Waiting\n");
    break;

  case ESPCONN_LISTEN:
    TRACE("Listening on %d\n", conn->proto.tcp->local_port);
    break;

  case ESPCONN_CONNECT:
    TRACE("Connecting\n");
    break;

  case ESPCONN_CLOSE:
    TRACE("Closed\n");
    break;

  case ESPCONN_WRITE:
  case ESPCONN_READ:
    TRACE("Buffered\n");
    break;
  }

  if (conn->state != ESPCONN_LISTEN)
  {
    checkAgain();
  }
}

void onReceive(void *arg, char *pdata, unsigned short length)
{
  TRACE("Received %d bytes\n", length);
  program_load(&program, (unsigned char *)pdata, (uint)length);
  struct espconn *conn = (espconn *)arg;
  const char *response = "HTTP/1.1 200 OK\r\n\r\nOK\0\r\n";
  espconn_send(conn, (uint8 *)response, strlen(response));
  program_start(&program);
}

void onSend(char *data, int length)
{
  espconn_send(conn, (uint8 *)data, length);
}

void onDisconnect(void *arg)
{
  TRACE("Client disconnected\n");
  checkAgain();
}

void onReconnect(void *arg, int error)
{
  TRACE("Reconnected after error %d\n", error);
  checkAgain();
}

void onConnect(void *arg)
{
  struct espconn *conn = (espconn *)arg;
  TRACE("Client connected\n");

  espconn_set_opt(conn, ESPCONN_START | ESPCONN_KEEPALIVE);
}

void setup()
{
  wifi.disconnect();
  wifi.stopAccessPoint();
  wifi.connectTo(WIFI_SSID, WIFI_PASSWORD);
  wifi.startAccessPoint();

  conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  conn->type = ESPCONN_TCP;
  conn->state = ESPCONN_NONE;
  conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  conn->proto.tcp->local_port = 3000;

  espconn_create(conn);
  espconn_regist_time(conn, 5, 1);
  espconn_regist_connectcb(conn, &onConnect);
  espconn_regist_recvcb(conn, &onReceive);
  espconn_regist_disconcb(conn, &onDisconnect);
  espconn_regist_sentcb(conn, (espconn_sent_callback)&checkAgain);
  espconn_accept(conn);

  program.onSend = &onSend;

  os_timer_setfn(&wifiTimer, &checkConnection, conn);
  checkAgain();
}
