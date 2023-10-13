#include "ensr.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int ensr_main(struct ensr_config *cfg) {
  if (ensr_platform_init() == -1) {
    fprintf(stderr, "Platform init failed!\n");
    return -1;
  }

  return 0;
}

enum ensr_mode ensr_mode_from(const char *s) { return ENSR_MODE_EQLINES; }

/**
 * Platform specific code
 */

#ifdef __linux__

#ifdef ENSR_MOD_PROC

struct ensr_proc ensr_proc_pid(int pid) {
  struct ensr_proc proc;
  memset(&proc, 0, sizeof(proc));
  proc.pid = pid;

  char buf[ENSR_PATH_MAX];
  sprintf(buf, "/prc/%d/comm", pid);

  FILE *comm = fopen(buf, "re");
  if (!comm) {
    fprintf(stderr, "%s", strerror(errno));
    goto FAIL;
  }
  fgets(proc.comm, ENSR_COMM_MAX, comm);
  fclose(comm);

  return proc;
FAIL:
  proc.ok = -1;
  return proc;
}

#endif

#endif

// TODO: implement for each platform
int ensr_platform_init(void) { return 0; }
