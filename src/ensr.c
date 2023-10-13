#include "ensr.h"
#include <stdio.h>

int ensr_main(struct ensr_config *cfg) {
  if (ensr_platform_init() == -1) {
    fprintf(stderr, "Platform init failed!\n");
    return -1;
  }

  return 0;
}

/**
 * Platform specific code
 */

int ensr_platform_init(void) { return 0; }
