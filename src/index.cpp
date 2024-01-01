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
static const char *httpOK = "HTTP/1.1 200 OK\r\n\r\nOK\r\n";
static const char *httpNotOK = "HTTP/1.1 400 Bad payload\r\n\r\n";
static const char separator[4] = {0x0d, 0x0a, 0x0d, 0x0a};

void checkAgain()
{
  os_timer_disarm(&wifiTimer);
  os_timer_arm(&wifiTimer, 10000, 0);
}

void checkConnection(void *arg)
{
  if (!wifi.isConnected())
  {
    TRACE("wifi off\n");
    if (ESPCONN_CLOSE != conn->state)
    {
      espconn_disconnect(conn);
    }

    uint8 status = wifi_station_get_connect_status();
    if (status == STATION_WRONG_PASSWORD)
    {
      TRACE("wrong password\n");
      return;
    }

    if (status != STATION_CONNECTING)
    {
      TRACE("connect to '%s'/'%s'\n", WIFI_SSID, WIFI_PASSWORD);
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

void printBuffer(char *data, unsigned short length)
{
  int i = 0;

  while (i < length)
  {
    os_printf("%02x", data[i++]);
  }

  os_printf("<<END\n");
}

void onReceive(void *arg, char *data, unsigned short length)
{
  TRACE("Received %d bytes\n", length);
  int i = 0;

  if (strncmp(data, "GET", 3) == 0)
  {
    TRACE("Status");
    espconn_send(conn, (uint8 *)httpOK, strlen(httpOK));
    // espconn_disconnect(conn);
    // vm_systemInformation(&program);
    // vm_dump(&program);
    return;
  }

  if (strncmp(data, "POST", 4) != 0)
  {
    espconn_send(conn, (uint8 *)httpNotOK, strlen(httpNotOK));
    espconn_disconnect(conn);
  }

  // skip headers
  while (i < length)
  {
    if (data[i] == 0x0d && strncmp(data + i, separator, 4) == 0)
    {
      i += 4;
      break;
    }

    i++;
  }

  if (i < length)
  {
    os_printf("Program found at %d\n", i);
    printBuffer(data + i, length - i);
    program_load(&program, (unsigned char *)data + i, length - i);
    program_start(&program);
  }

  espconn_send(conn, (uint8 *)httpOK, strlen(httpOK));
  espconn_disconnect(conn);
}

void onSend(char *data, int length)
{
  os_printf("send %d bytes\n", length);
  os_printf("conn state: %d\n", conn->state);
  printBuffer(data, length);
  if (conn->state == ESPCONN_CONNECT)
  {
    espconn_send(conn, (uint8 *)data, length);
  }
}

void onDisconnect(void *arg)
{
  TRACE("Client disconnected\n");
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