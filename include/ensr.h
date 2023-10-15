#ifndef ENSR_H_
#define ENSR_H_

#include <stddef.h>
#include <stdio.h>

#define ENSR_PATH_MAX 4096
#define ENSR_COMM_MAX 32

#define ENSR_MOD_OFF(name)                                                     \
  {                                                                            \
    fprintf(stderr, "Module %s is disabled! Aborting...\n", name);             \
    exit(-1);                                                                  \
  }

/**
 * =============================
 * Configuration
 * Enable and disable features
 * dynamically here
 * =============================
 */

// Optional modules

#define ENSR_MOD_STDIO 1
#define ENSR_MOD_PATH 1
#define ENSR_MOD_PROC 1
#define ENSR_MOD_FMT 1

// Config variables
#define ENSR_CFG_FMT_OK "\x1B[32m"
#define ENSR_CFG_FMT_WARN "\x1B[33m"
#define ENSR_CFG_FMT_ERR "\x1B[31m"
#define ENSR_CFG_FMT_RESET "\x1B[0m"

#define ENSR_CFG_OK "ok\t"
#define ENSR_CFG_ERR "err\t"

// env variables
#define ENSR_ENV_FMT_OK "ENSR_OK"
#define ENSR_ENV_FMT_WARN "ENSR_WARN"
#define ENSR_ENV_FMT_ERR "ENSR_ERR"
#define ENSR_ENV_FMT_RESET "ENSR_FMT_RESET"

/**
 * =============================
 * End configuration
 * =============================
 */

enum ensr_mode {
  // eq check number either from stdin or input
  ENSR_MODE_EQN,
  // gt check number either from stdin or input
  ENSR_MODE_GTN,
  // lt check number either from stdin or input
  ENSR_MODE_LTN,

  // check if a pid is running
  ENSR_MODE_PID,

  // check if a command is running
  ENSR_MODE_COMM,

};

enum ensr_mode ensr_mode_from(const char *s);

struct ensr_config {
  _Bool verbose;

  enum ensr_mode mode;
  const char **input;
  size_t input_len;

  const char *rule_name;
  const char *proc;
  int pid;

  FILE *in;
  FILE *out;

#ifdef ENSR_MOD_FMT
  const char *fmt_ok;
  const char *fmt_warn;
  const char *fmt_err;
  const char *fmt_reset;
#endif

  const char *path;
};

struct ensr_ctx {
  _Bool header_drawn;
};

struct ensr_config ensr_config_env(void);

void ensr_fmt(FILE *f, const char *fmt);

int ensr_main(struct ensr_config *cfg);

_Bool ensr_strnisint(const char *str, size_t n);
void ensr_trimnl(char *s);

/**
 * Checks that the lines received in stdin
 * are exactly n lines
 * This is useful for piping results from commands
 * like grep into ensure
 */
int ensr_eqn(int n);
int ensr_gtn(int n);
int ensr_ltn(int n);

/**
 * Process related stuff
 * on linux we read proc(5)
 * on bsd we will use sysctl(2)
 * for tests, use a dummy implementation
 */

/**
 * Generic process information we might be interested in
 */
struct ensr_proc {
  int ok;

  int pid;
  char comm[ENSR_COMM_MAX];
  int uid;
};

int ensr_proc_pid_check(struct ensr_config *cfg, int pid);

int ensr_proc_name_check(struct ensr_config *cfg, const char *comm,
                         struct ensr_proc *procs, size_t len);

/**
 * Platform specific
 * Populate proc struct by pid
 * This will always return a single proc
 */
struct ensr_proc ensr_proc_pid(int pid);
void ensr_fproc_header(struct ensr_config *cfg, struct ensr_ctx *ctx);

/**
 * Platform specific
 * Read all pids and return them
 * the pid list is allocated using malloc(3)
 * and should be freed using free()
 */
struct ensr_proc *ensr_proc_pids(size_t *len);

int ensr_comm_running(const char *proc_name);
int ensr_pid_running(int pid);

/**
 * Check if the name is a valid program
 */
int ensr_in_path(const char *path);

/**
 * Checks if file exists
 */
int ensr_path_exists(const char *path);

int ensr_path_owner(const char *path, const char *usr);

/**
 * Platform init code. This is e.g. where pledge(2) could be used on OpenBSD
 */
int ensr_platform_init(void);

#endif
