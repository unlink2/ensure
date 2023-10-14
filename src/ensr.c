#include "ensr.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#endif

const char *ensr_getenv(const char *env, const char * or) {
  const char *v = getenv(env);

  if (!v) {
    return or ;
  }
  return v;
}

int ensr_main(struct ensr_config *cfg) {
  if (ensr_platform_init() == -1) {
    fprintf(stderr, "Platform init failed!\n");
    return -1;
  }

  ensr_fmt(cfg->out, cfg->fmt_reset);
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

struct ensr_config ensr_config_env(void) {
  struct ensr_config cfg;
  memset(&cfg, 0, sizeof(cfg));

  cfg.fmt_reset = ensr_getenv(ENSR_ENV_FMT_RESET, ENSR_CFG_FMT_RESET);
  cfg.fmt_err = ensr_getenv(ENSR_ENV_FMT_ERR, ENSR_CFG_FMT_ERR);
  cfg.fmt_ok = ensr_getenv(ENSR_ENV_FMT_OK, ENSR_CFG_FMT_OK);
  cfg.fmt_warn = ensr_getenv(ENSR_ENV_FMT_WARN, ENSR_CFG_FMT_WARN);

  cfg.in = stdin;
  cfg.out = stdout;

  return cfg;
}
#ifdef ENSR_MOD_FMT

void ensr_fmt(FILE *f, const char *fmt) {
  if (!isatty(fileno(f))) {
    return;
  }

  fputs(fmt, f);
}

#else

void ensr_fmt(FILE *f, const char *fmt) { ENSR_MOD_OFF("fmt"); }
#endif

/**
 * Platform specific code
 */

#ifdef ENSR_MOD_PROC

void ensr_fproc(struct ensr_config *cfg, struct ensr_proc *proc) {
  if (proc->ok != 0) {
    ensr_fmt(cfg->out, cfg->fmt_err);
  } else {
    ensr_fmt(cfg->out, cfg->fmt_ok);
  }
  fprintf(cfg->out, "[v]\t%s\t%d\n", proc->comm, proc->pid);
}

int ensr_proc_pid_check(struct ensr_config *cfg, int pid) {
  struct ensr_proc proc = ensr_proc_pid(pid);
  ensr_fproc(cfg, &proc); 
  return proc.ok;
}

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
