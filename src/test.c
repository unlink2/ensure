#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ensr.h"

void test_glob(void **state) {
  {
    assert_int_equal('t', ensr_glob_patnext("test", 4, 0).c);
    assert_int_equal('e', ensr_glob_patnext("test", 4, 1).c);
    assert_int_equal('s', ensr_glob_patnext("test", 4, 2).c);
    assert_int_equal('t', ensr_glob_patnext("test", 4, 3).c);

    assert_true(ensr_glob_patnext("?est", 4, 0).match_any);
    assert_int_equal('e', ensr_glob_patnext("*est", 4, 0).match_until);
  }
  {
    assert_int_equal('*', ensr_glob_patnext("\\*est", 5, 0).c);
    assert_int_equal('*', ensr_glob_patnext("\\*est", 5, 0).c);
    assert_int_equal('e', ensr_glob_patnext("\\*est", 5, 2).c);
    assert_int_equal('s', ensr_glob_patnext("\\*est", 5, 3).c);
    assert_int_equal('t', ensr_glob_patnext("\\*est", 5, 4).c);
  }

  assert_true(ensr_glob_match("test", "test"));
  assert_true(ensr_glob_match("te\\*st", "te*st"));
  assert_true(ensr_glob_match("te\\?st", "te?st"));
  assert_true(ensr_glob_match("te\\Xst", "teXst"));
  assert_true(ensr_glob_match("?est", "test"));
  assert_true(ensr_glob_match("*est", "test"));
  assert_true(ensr_glob_match("*st", "test"));
  assert_true(ensr_glob_match("*s*", "test"));
  assert_true(ensr_glob_match("*test", "abtest"));
  assert_true(ensr_glob_match("*te*", "abtest"));
  assert_true(ensr_glob_match("te*", "test"));

  assert_false(ensr_glob_match("?est", "tast"));
  assert_false(ensr_glob_match("*s*", "teat"));
  assert_false(ensr_glob_match("*es", "test"));
}

int main(int arc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_glob),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
