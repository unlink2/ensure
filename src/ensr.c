#include "ensr.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#include <dirent.h>
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

  int ok = -1;

  switch (cfg->mode) {
  case ENSR_MODE_COMM: {

    size_t len = 0;
    struct ensr_proc *procs = ensr_proc_pids(&len);
    if (procs == NULL) {
      return -1;
    }

    ok = ensr_proc_name_check(cfg, cfg->input, procs, len);

    free(procs);
    break;
  }
  case ENSR_MODE_EQN:
  case ENSR_MODE_GTN:
  case ENSR_MODE_LTN:
  case ENSR_MODE_PID:
  case ENSR_MODE_EXISTS:
    fprintf(stderr, "Not implemented\n");
    break;
  }

  ensr_fmt(cfg->out, cfg->fmt_reset);
  return ok;
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

#ifdef ENSR_MOD_PROC

void ensr_fproc_header(struct ensr_config *cfg) {
  fputs("status\ttype\tpid\tcomm\n", cfg->out);
}

void ensr_fproc(struct ensr_config *cfg, struct ensr_proc *proc) {
  if (proc->ok) {
    ensr_fmt(cfg->out, cfg->fmt_err);
    fputs(ENSR_CFG_ERR, cfg->out);
  } else {
    ensr_fmt(cfg->out, cfg->fmt_ok);
    fputs(ENSR_CFG_OK, cfg->out);
  }
  fprintf(cfg->out, "proc\t%d\t%s\n", proc->pid, proc->comm);
}

int ensr_proc_name_check(struct ensr_config *cfg, const char *comm,
                         struct ensr_proc *procs, size_t len) {
  if (!comm) {
    return -1;
  }

  ensr_fproc_header(cfg);
  int ok = -1;
  for (size_t i = 0; i < len; i++) {
    struct ensr_proc *proc = &procs[i];
    if (proc->ok) {
      ok = proc->ok;
      break;
    }

    if (strcmp(comm, proc->comm) == 0) {
      ok = proc->ok;
      ensr_fproc(cfg, proc);
      break;
    }
  }

  if (ok) {
    ensr_fmt(cfg->out, cfg->fmt_err);
    fputs(ENSR_CFG_ERR, cfg->out);
    fprintf(cfg->out, "proc\t-1\t%s\n", comm);
  }

  return ok;
}

int ensr_proc_pid_check(struct ensr_config *cfg, int pid) {
  struct ensr_proc proc = ensr_proc_pid(pid);
  ensr_fproc_header(cfg);
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

  // trim new line
  proc.comm[strcspn(proc.comm, "\n")] = '\0';

  fclose(comm);

  return proc;
FAIL:
  proc.ok = -1;
  return proc;
}

struct ensr_proc *ensr_proc_pids(size_t *len) {
  *len = 0;
  struct ensr_proc *procs = NULL;

  DIR *d = opendir("/proc");
  if (d == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    return NULL;
  }

  size_t all_files_cnt = 0;
  struct dirent *dir = NULL;

  // first simple count all entries
  // and malloc
  while ((dir = readdir(d)) != NULL) {
    all_files_cnt++;
  }

  procs = malloc(sizeof(struct ensr_proc) * all_files_cnt);

  rewinddir(d);

  // now process all pid entries
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 &&
        strcmp(dir->d_name, "..") != 0) {

      // test if the entire name is a digit, if so its a pid!
      for (size_t i = 0; i < strlen(dir->d_name); i++) {
        if (!isdigit(dir->d_name[i])) {
          goto NOT_PID;
        }
      }

      procs[*len] = ensr_proc_pid(atoi(dir->d_name));

      if (procs[*len].ok) {
        goto FAIL;
      }
      *len += 1;
    }
    // place continue here to not have label at end of compount statement
  NOT_PID:
    continue;
  }

  closedir(d);

  return procs;
FAIL:
  free(procs);
  return NULL;
}

#endif

#else
struct ensr_proc ensr_proc_pid(int pid) { ENSR_MOD_OFF("proc"); }
#endif

// TODO: implement for each platform
int ensr_platform_init(void) { return 0; }
