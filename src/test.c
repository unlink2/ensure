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

  assert_true(ensr_glob_match("test", 4, "test", 4));
  assert_true(ensr_glob_match("?est", 4, "test", 4));
  assert_true(ensr_glob_match("*est", 4, "test", 4));
  assert_true(ensr_glob_match("*st", 3, "test", 4));
  assert_true(ensr_glob_match("*s*", 3, "test", 4));

  assert_false(ensr_glob_match("?est", 4, "tast", 4));
  assert_false(ensr_glob_match("*s*", 4, "teat", 4));
  assert_false(ensr_glob_match("*es", 3, "test", 4));
}

int main(int arc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_glob),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
