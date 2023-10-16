#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ensr.h"

void test_glob(void **state) {
  {
    size_t i = 0;
    assert_int_equal('t', ensr_glob_patnext("test", 4, &i));
    assert_int_equal('e', ensr_glob_patnext("test", 4, &i));
    assert_int_equal('s', ensr_glob_patnext("test", 4, &i));
    assert_int_equal('t', ensr_glob_patnext("test", 4, &i));
  }
  {
    size_t i = 0;
    assert_int_equal('*', ensr_glob_patnext("\\*est", 5, &i));
    assert_int_equal('e', ensr_glob_patnext("\\*est", 5, &i));
    assert_int_equal('s', ensr_glob_patnext("\\*est", 5, &i));
    assert_int_equal('t', ensr_glob_patnext("\\*est", 5, &i));
  }

  assert_true(ensr_glob_match("test", 4, "test", 4));
  assert_true(ensr_glob_match("?est", 4, "test", 4));
}

int main(int arc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_glob),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
