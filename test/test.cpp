#define WITH_DEBUG
#define SERIAL_SPEED 115200

#include "espmock.hpp"
#include "vm.hpp"
#include <stdio.h>

static Program program;

void onSend(char *bytes, int length)
{
  fwrite(bytes, 1, length, stdout);
  printf("\n");
}

int main()
{
  // unsigned char bytes[] = {op_debug, vt_byte, 0x01, op_print, vt_byte, 0x01, op_systeminfo, op_delay, 0xe8, 0x03, 0, 0, op_jumpto, vt_integer, 0x00, 0x00, 0x00, 0x00};
  unsigned char bytes[] = {op_debug, vt_byte, 0x01, op_systeminfo, op_noop};

  program.onSend = &onSend;
  vm_load(&program, bytes, sizeof(bytes));
  vm_start(&program);
  vm_tick(&program);
}