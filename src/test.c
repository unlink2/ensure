#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ensr.h"

void test_glob(void **state) {
  assert_true(ensr_glob_match("test", "test"));
  assert_true(ensr_glob_match("?est", "test"));
}

int main(int arc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_glob),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
