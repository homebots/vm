#define WITH_DEBUG
#define SERIAL_SPEED 115200

#include "espmock.hpp"
#include "vm.hpp"
#include <stdio.h>

static Program program;

void onSend(char *bytes, int length)
{
  fwrite(bytes, 1, length, stdout);
}

int main(int argc, char **argv)
{
  program.onSend = &onSend;

  char *fileName = argv[1];

  if (argc < 2 || !strlen(fileName))
  {
    printf("No file to run!\n\nUsage:\n  vm path/to/file.bin\n");
    return -1;
  }

  printf("Running %s\n", fileName);

  FILE *file;
  unsigned char *buffer;
  long length;

  file = fopen(fileName, "r");
  if (file == NULL)
  {
    perror("Error in opening file");
    return -1;
  }

  fseek(file, 0L, SEEK_END);
  length = ftell(file);
  rewind(file);
  buffer = (unsigned char *)malloc(length);

  if (buffer == NULL)
  {
    perror("Error allocating memory");
    fclose(file);
    return -2;
  }

  fread(buffer, sizeof(char), length, file);
  fclose(file);
  vm_load(&program, buffer, length);
  free(buffer);

  vm_tick(&program);
}