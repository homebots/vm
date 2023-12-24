#define WITH_DEBUG
#define SERIAL_SPEED 115200

#include "homebots.h"
#include "vm.hpp"

// WIFI_SSID and WIFI_SSID should be defined as environment variables
#ifndef WIFI_SSID
#define WIFI_SSID "HomeBots"
#define WIFI_PASSWORD "HomeBots"
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

    auto status = wifi_station_get_connect_status();
    TRACE("wifi state: %d\n", status);
    if (status != STATION_CONNECTING && status != STATION_WRONG_PASSWORD)
    {
      wifi.connectTo(WIFI_SSID, WIFI_PASSWORD);
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

void onReceive(void *arg, char *data, unsigned short length)
{
  TRACE("Received %d bytes\n", length);
  int i = 0;

  while (i < length)
  {
    os_printf("%02x", data[i++]);
  }

  i = 0;
  struct espconn *conn = (espconn *)arg;
  const char *OK = "HTTP/1.1 200 OK\r\n\r\nOK\r\n";
  const char *notOK = "HTTP/1.1 400 Bad payload\r\n\r\n";
  const char separator[4] = {0x0d, 0x0a, 0x0d, 0x0a};

  if (strncmp(data, "GET", 3) == 0)
  {
    espconn_send(conn, (uint8 *)OK, strlen(OK));
    vm_systemInformation(&program);
    vm_dump(&program);
    espconn_disconnect(conn);
    return;
  }

  if (strncmp(data, "POST", 4) == 0)
  {
    while (i < length)
    {
      if (data[i] == 0x0d && strncmp(data + i, separator, 4) == 0)
      {
        i += 4;
        os_printf("Program at %d\n", i);
        program_load(&program, (unsigned char *)data + i, length - i);
        os_printf("Program loaded\n");
        program_start(&program);
        espconn_send(conn, (uint8 *)OK, strlen(OK));
        return;
      }

      i++;
    }
  }

  espconn_send(conn, (uint8 *)notOK, strlen(notOK));
  espconn_disconnect(conn);
}

void onSend(char *data, int length)
{

  espconn_send(conn, (uint8 *)data, length);
}

void onDisconnect(void *arg)
{
  TRACE("Client disconnected\n");
  struct espconn *conn = (espconn *)arg;
  espconn_accept(conn);
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
  wifi.startAccessPoint();
  os_printf("Connecting to %s : %s\n", WIFI_SSID, WIFI_PASSWORD);
  wifi.connectTo(WIFI_SSID, WIFI_PASSWORD);

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
