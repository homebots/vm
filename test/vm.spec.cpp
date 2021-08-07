#include "assert.h"
#include "vm.hpp"

int main()
{
  describe(
      "Main controller", []
      {
        it("should set up the wifi", []
           {
             Program program;
             byte bytes[] = {5, 0, 71, 67, 0, 2, 1, 67, 1, 2, 1, 8, 5, 5, 0, 0, 0, 67, 0, 2, 0, 67, 2, 2, 1, 8, 5, 5, 0, 0, 0, 67, 1, 2, 0, 67, 3, 2, 1, 8, 5, 5, 0, 0, 0, 67, 2, 2, 0, 67, 0, 2, 1, 8, 5, 5, 0, 0, 0, 67, 3, 2, 0, 67, 1, 2, 1, 10, 4, 3, 0, 0, 0};
             program.load(bytes, sizeof(bytes));
           });
      });

  testSummary();
}