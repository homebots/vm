
#define WITH_DEBUG
#define SERIAL_SPEED 115200
#define __CHIP_ESP8266__

#include "homebots.h"
#include "esp8266.hpp"
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
static const char *httpOK = "HTTP/1.1 200 OK\r\n\r\n";
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
    if (ESPCONN_CLOSE != conn->state)
    {
      espconn_disconnect(conn);
    }

    uint8 status = wifi_station_get_connect_status();
    if (status != STATION_CONNECTING)
    {
      wifi.connectTo(WIFI_SSID, WIFI_PASSWORD);
    }

    checkAgain();
    return;
  }

  // todo will kill active connections
  if (conn->state != ESPCONN_LISTEN)
  {
    espconn_accept(conn);
  }

  checkAgain();
}

void onReceive(void *arg, char *data, unsigned short length)
{
  int i = 0;

  if (strncmp(data, "GET", 3) == 0)
  {
    espconn_send(conn, (uint8 *)httpOK, strlen(httpOK));
    program.flush();
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
    TRACE("Running %d bytes\n", length - i);
    espconn_send(conn, (uint8 *)httpOK, strlen(httpOK));
    program_load(&program, (unsigned char *)data + i, length - i);
    return;
  }

  espconn_send(conn, (uint8 *)httpNotOK, strlen(httpNotOK));
  espconn_disconnect(conn);
}

void onSend(char *data, int length)
{
  if (conn->state == ESPCONN_WRITE)
  {
    espconn_send(conn, (uint8 *)data, length);
  }
}

void onHalt()
{
  if (conn->state != ESPCONN_LISTEN)
  {
    espconn_disconnect(conn);
    checkAgain();
  }
}

void onDisconnect(void *arg)
{
  TRACE("Disconnected\n");
  espconn_accept(conn);
  checkAgain();
}

void onReconnect(void *arg, int error)
{
  checkAgain();
}

void onConnect(void *arg)
{
  struct espconn *conn = (espconn *)arg;
  espconn_set_opt(conn, ESPCONN_START | ESPCONN_KEEPALIVE);
}

void setup()
{
  wifi.disconnect();
  wifi.stopAccessPoint();
  wifi.startAccessPoint();
  wifi.connectTo(WIFI_SSID, WIFI_PASSWORD);

  conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  conn->type = ESPCONN_TCP;
  conn->state = ESPCONN_NONE;
  conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  conn->proto.tcp->local_port = 3000;

  espconn_create(conn);
  espconn_regist_time(conn, 60, 1);
  // espconn_tcp_set_max_con_allow(conn, 4);
  espconn_regist_connectcb(conn, &onConnect);
  espconn_regist_recvcb(conn, &onReceive);
  espconn_regist_disconcb(conn, &onDisconnect);
  espconn_regist_sentcb(conn, (espconn_sent_callback)&checkAgain);
  espconn_accept(conn);

  program.onSend = &onSend;
  program.onHalt = &onHalt;

  os_timer_setfn(&wifiTimer, &checkConnection, conn);
  checkAgain();
}
