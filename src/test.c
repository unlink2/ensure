#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

void test_glob(void **state) {}

int main(int arc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_glob),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
