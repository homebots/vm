#ifndef _ASSERT_CPP_
#define _ASSERT_CPP_

#include "sdk/homebots.h"
// #include <string.h>
// #include <iostream>
#include <stdlib.h>

#define lprintf(...)        \
  do                        \
  {                         \
    leftPadding();          \
    os_printf(__VA_ARGS__); \
  } while (0);

using namespace std;

const char *RESULT_OK = "✔";
const char *RESULT_FAIL = "×";
const char *LEFTPAD = "  ";

typedef void (*Spec)();

int failed = 0;
int successful = 0;
int skipped = 0;
int paddingLevel = 0;
int hasErrors = false;

Spec beforeEachFn = NULL;
Spec afterEachFn = NULL;

void leftPadding()
{
  for (int i = 1; i < paddingLevel; i++)
  {
    os_printf("%s", LEFTPAD);
  }
}

void printResult(const char *description)
{
  const char *prefix = RESULT_OK;
  if (hasErrors)
  {
    prefix = RESULT_FAIL;
  }

  int length = strlen(description) + 5;
  char *message = (char *)os_zalloc(length);

  os_printf(message);
  os_printf("%d", length);
  os_printf("%s %s", prefix, description);
  lprintf("%s\n", message);
  free(message);
}

template <typename T>
bool compare(T actual, T expected)
{
  return actual == expected;
}

bool compare(unsigned char *actual, const char *expected)
{
  return 0 == strcmp((const char *)actual, expected);
}

void runSpec(Spec spec)
{
  try
  {
    spec();
  }
  catch (...)
  {
    hasErrors = true;
  }
}

void skip(const char *description)
{
  paddingLevel++;
  lprintf("- %s\n", description);
  paddingLevel--;
}

/////////

void describe(const char *description, Spec spec)
{
  paddingLevel++;
  lprintf("- %s\n", description);
  spec();
  paddingLevel--;
}

void it(const char *description, Spec spec)
{
  hasErrors = false;
  paddingLevel++;

  if (beforeEachFn != NULL)
  {
    runSpec(beforeEachFn);
  }

  runSpec(spec);

  if (afterEachFn != NULL)
  {
    runSpec(afterEachFn);
  }

  printResult(description);

  if (hasErrors)
  {
    failed++;
  }
  else
  {
    successful++;
  }

  paddingLevel--;
}

void beforeEach(Spec spec) { beforeEachFn = spec; }
void afterEach(Spec spec) { afterEachFn = spec; }
void xdescribe(const char *description) { skip(description); }
void xdescribe(const char *description, Spec spec)
{
  skipped++;
  skip(description);
}
void xit(const char *description)
{
  skipped++;
  skip(description);
}
void xit(const char *description, Spec spec)
{
  skipped++;
  skip(description);
}

void testSummary()
{
  os_printf("\n\nSuccessful: %d\n", successful);
  os_printf("Failed: %d\n", failed);
  os_printf("Skipped: %d\n", skipped);
  os_printf("\nTotal: %d\n", skipped + failed + successful);
  os_printf("\nDone.\n\n");

  if (failed > 0)
  {
    exit(1);
  }
}

template <typename T>
class Expectation
{
public:
  T actual;

  Expectation(T _actual) : actual(_actual) {}

  void toBe(T expected)
  {
    _toBe(!compare(actual, expected), expected);
  }

  void toBe(const char *expected)
  {
    _toBe(!compare(actual, expected), (unsigned char *)expected);
  }

private:
  void _toBe(bool value, T expected)
  {
    hasErrors = value;

    if (hasErrors)
    {
      os_printf("\n");
      leftPadding();
      os_printf("  Expected ");
      os_printf(expected);
      os_printf(" but got ");
      os_printf(actual);
      os_printf("\n");

      throw "error";
    }
  }
};

template <typename T>
Expectation<T> expect(T actual)
{
  Expectation<T> e(actual);
  return e;
}

#endif