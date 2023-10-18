#include "ensr.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

int ensr_do(struct ensr_config *cfg, struct ensr_ctx *ctx, const char *input) {
  int ok = -1;
  switch (cfg->mode) {
  case ENSR_MODE_COMM: {
    size_t len = 0;
    struct ensr_proc *procs = ensr_proc_pids(&len);
    if (procs == NULL) {
      return -1;
    }

    ensr_fproc_header(cfg, ctx);
    ok = ensr_proc_name_check(cfg, input, procs, len);

    free(procs);
    break;
  }
  case ENSR_MODE_PID: {
    if (ensr_strnisint(input, strlen(input))) {
      ensr_fproc_header(cfg, ctx);
      ok = ensr_proc_pid_check(cfg, atoi(input));
    } else {
      ensr_fmt(cfg->out, cfg->fmt_warn);
      fprintf(cfg->out, "%s is not a valid pid\n", input);
    }
    break;
  }
  case ENSR_MODE_ISZERO:
    if (ensr_strnisint(input, strlen(input))) {
      ok = ensr_iszero(cfg, atoi(input));
    } else {
      ensr_fmt(cfg->out, cfg->fmt_warn);
      fprintf(cfg->out, "%s is not an integer\n", input);
    }
    break;
  default:
    fprintf(stderr, "Not implemented\n");
    break;
  }

  ensr_fmt(cfg->out, cfg->fmt_reset);
  return ok;
}

int ensr_main(struct ensr_config *cfg) {
  if (ensr_platform_init() == -1) {
    fprintf(stderr, "Platform init failed!\n");
    return -1;
  }
  struct ensr_ctx ctx;
  memset(&ctx, 0, sizeof(ctx));

  int ok = 0;

  // process inputs
  for (size_t i = 0; i < cfg->input_len; i++) {
    if (ensr_do(cfg, &ctx, cfg->input[i]) != 0) {
      ok = -1;
    }
  }

  if (cfg->input_len == 0 || !isatty(fileno(cfg->in))) {
    // process stdin
    const int buflen = 1024;
    char buf[buflen];
    while (fgets(buf, buflen, cfg->in)) {
      ensr_trimnl(buf);
      if (ensr_do(cfg, &ctx, buf) != 0) {
        ok = -1;
      }
    }
  }

  return ok;
}

_Bool ensr_strnisint(const char *str, size_t n) {
  // test if the entire name is a digit, if so its a pid!
  for (size_t i = 0; i < n; i++) {
    if (!isdigit(str[i])) {
      return false;
    }
  }

  return true;
}

void ensr_trimnl(char *s) {
  // trim new line
  s[strcspn(s, "\n")] = '\0';
}

size_t ensr_glob_next(const char *pat, const char **str, size_t n,
                      size_t stride, size_t index) {

  return -1;
}

struct ensr_globpat ensr_glob_patnext(const char *pat, size_t pat_len,
                                      size_t i) {
  struct ensr_globpat gpat;
  // get next pattern char
  gpat.c = pat[i];
  gpat.read = 1;
  gpat.match_until = '\0';

  switch (gpat.c) {
  case '\\':
    gpat.c = pat[i + 1];
    gpat.read = 2;
    break;
  case '?':
    gpat.match_any = true;
    gpat.c = 0;
    break;
  case '*':
    gpat.c = 0;
    gpat.match_any = true;
    gpat.match_until = ensr_glob_patnext(pat, pat_len, i + 1).c;
    if (!gpat.match_until) {
      gpat.match_until = -1;
    }
    break;
  default:
    break;
  }

  return gpat;
}

_Bool ensr_glob_match(const char *pat, size_t pat_len, const char *str,
                      size_t str_len) {
  size_t pati = 0;
  struct ensr_globpat patc;
  memset(&patc, 0, sizeof(patc));

  char c = '\0';
  size_t i = 0;
  for (i = 0; i < str_len; i++) {
    c = str[i];
    if (!patc.match_until || patc.match_until == c) {
      patc = ensr_glob_patnext(pat, pat_len, pati);
      pati += patc.read;
    }
    if (!patc.match_any && c != patc.c) {
      return false;
    }

    if (!patc.match_until) {
      patc.match_any = false;
    }
  }
  return pati == pat_len && i == str_len;
}

enum ensr_mode ensr_mode_from(const char *s) {
  switch (s[0]) {
  case 'z':
    return ENSR_MODE_ISZERO;
  case 'p':
    return ENSR_MODE_PID;
  case 'c':
    return ENSR_MODE_COMM;
  default:
    fprintf(stderr, "Unknown mode: %c\n", s[0]);
    exit(-1);
    break;
  }
  return ENSR_MODE_ISZERO;
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

#ifdef ENSR_MOD_CMP

int ensr_iszero(struct ensr_config *cfg, int n) {
  bool z = n == 0;
  if (z) {
    ensr_fmt(cfg->out, cfg->fmt_ok);
    fprintf(cfg->out, "%s\t%d\n", ENSR_CFG_OK, n);
    return 0;
  } else {
    ensr_fmt(cfg->out, cfg->fmt_err);
    fprintf(cfg->out, "%s\t%d\n", ENSR_CFG_ERR, n);
    return -1;
  }
}

#else

int ensr_iszero(int n) { ENSR_MOD_OFF("cmp"); }

#endif

#ifdef ENSR_MOD_PROC

void ensr_fproc_header(struct ensr_config *cfg, struct ensr_ctx *ctx) {
  // only draw header once
  if (ctx->header_drawn) {
    return;
  }

  fputs("status\ttype\tpid\tcomm\n", cfg->out);
  ctx->header_drawn = true;
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

  int ok = 0;
  size_t count = 0;
  for (size_t i = 0; i < len; i++) {
    struct ensr_proc *proc = &procs[i];
    if (proc->ok) {
      ok = proc->ok;
      break;
    }

    if (ensr_glob_match(comm, strlen(comm), proc->comm, strlen(proc->comm))) {
      count++;
      ensr_fproc(cfg, proc);
    }
  }

  if (!count) {
    ensr_fmt(cfg->out, cfg->fmt_err);
    fputs(ENSR_CFG_ERR, cfg->out);
    fprintf(cfg->out, "proc\t-1\t%s\n", comm);

    ok = -1;
  }

  return ok;
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
    // fprintf(stderr, "%s\n", strerror(errno));
    goto FAIL;
  }
  fgets(proc.comm, ENSR_COMM_MAX, comm);
  ensr_trimnl(proc.comm);

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
        strcmp(dir->d_name, "..") != 0 &&
        ensr_strnisint(dir->d_name, strlen(dir->d_name))) {
      procs[*len] = ensr_proc_pid(atoi(dir->d_name));

      if (procs[*len].ok) {
        goto FAIL;
      }
      *len += 1;
    }
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
