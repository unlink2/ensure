#include "ensr.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ensr_main(struct ensr_config *cfg) {
  if (ensr_platform_init() == -1) {
    fprintf(stderr, "Platform init failed!\n");
    return -1;
  }

  ensr_proc_pid(1);

  return 0;
}

enum ensr_mode ensr_mode_from(const char *s) {
  switch (s[0]) {
  case 'E':
    return ENSR_MODE_EQN;
  case 'g':
    return ENSR_MODE_GTN;
  case 'l':
    return ENSR_MODE_LTN;
  case 'p':
    return ENSR_MODE_PID;
  case 'c':
    return ENSR_MODE_COMM;
  default:
    fprintf(stderr, "Unknown mode: %c\n", s[0]);
    exit(-1);
    break;
  }
  return ENSR_MODE_EQN;
}

/**
 * Platform specific code
 */

#ifdef ENSR_MOD_PROC

#ifdef __linux__
struct ensr_proc ensr_proc_pid(int pid) {
  struct ensr_proc proc;
  memset(&proc, 0, sizeof(proc));
  proc.pid = pid;

  char buf[ENSR_PATH_MAX];
  sprintf(buf, "/proc/%d/comm", pid);

  FILE *comm = fopen(buf, "re");
  if (!comm) {
    fprintf(stderr, "%s\n", strerror(errno));
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

#else
struct ensr_proc ensr_proc_pid(int pid) { ENSR_MOD_OFF("proc"); }
#endif

// TODO: implement for each platform
int ensr_platform_init(void) { return 0; }
