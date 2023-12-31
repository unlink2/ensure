#ifndef ENSR_H_
#define ENSR_H_

#include <stdbool.h>
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

#define ENSR_MOD_FMT 1
#define ENSR_MOD_PATH 1
#define ENSR_MOD_PROC 1
#define ENSR_MOD_CMP 1

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
  // check if input is an integer and is zero
  ENSR_MODE_ISZERO,

  // check if a pid is running
  ENSR_MODE_PID,

  // check if a command is running
  ENSR_MODE_COMM,

  // check if name is in $PATH
  ENSR_MODE_IN_SYSPATH,
};

enum ensr_mode ensr_mode_from(const char *s);

struct ensr_config {
  // general settings
  _Bool verbose;
  enum ensr_mode mode;

  // input array
  const char **input;
  size_t input_len;

  // io
  FILE *in;
  FILE *out;
#ifdef ENSR_MOD_PROC
  const char *proc;
  int pid;
#endif
#ifdef ENSR_MOD_FMT
  const char *fmt_ok;
  const char *fmt_warn;
  const char *fmt_err;
  const char *fmt_reset;
#endif
#ifdef ENSR_MOD_PATH
  const char *syspath;
#endif

  const char *path;
};

struct ensr_globpat {
  size_t read; // how many bytes of the pattern were consumed
  char c;
  _Bool match_any;
  char match_until;
};

struct ensr_config ensr_config_env(void);

void ensr_fmt(FILE *f, const char *fmt);

int ensr_main(struct ensr_config *cfg);

// returns the next item in an array of strings
// the array is incremented by stride bytes for each iteration (if stride is 0
// it will use sizeof(char*) bytes) the array is of size n and the search for
// the next valid item will begin at index Returns the next index. Returns -1 on
// error.
size_t ensr_glob_next(const char *pat, const char **str, size_t n,
                      size_t stride, size_t index);

struct ensr_globpat ensr_glob_patnext(const char *pat, size_t pat_len,
                                      size_t i);
// Glob patterns currently supported:
//  *: Matches any sequence of characters
//  ?: Matches a single character
//  \x: Matches the literal character x
// TODO: put glob documnation in man
_Bool ensr_glob_match(const char *pat, const char *str);

_Bool ensr_strnisint(const char *str, size_t n);
void ensr_trimnl(char *s);

/**
 * cmp module
 */

/**
 * Checks if number recieved over stdin is zero
 */
int ensr_iszero(struct ensr_config *cfg, int n);

/**
 * Proc module
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

/**
 * Not platform specific
 * General proc code
 */
int ensr_proc_pid_check(struct ensr_config *cfg, int pid);

int ensr_proc_name_check(struct ensr_config *cfg, const char *comm,
                         struct ensr_proc *procs, size_t len);

/**
 * Platform specific
 * Populate proc struct by pid
 * This will always return a single proc
 */
struct ensr_proc ensr_proc_pid(int pid);
void ensr_fproc_header(struct ensr_config *cfg);

/**
 * Platform specific
 * Read all pids and return them
 * the pid list is allocated using malloc(3)
 * and should be freed using free()
 */
struct ensr_proc *ensr_proc_pids(size_t *len);

/**
 * Path module
 */

/**
 * Takes a path and checks if name is in it
 * outputs results to cfg->out
 */
int ensr_syspath_check(const struct ensr_config *cfg, const char *path,
                       const char *name);

/**
 * Platform specific
 * Check if the name is a valid program
 * Takes in $PATH and checks if name is a valid member of $PATH
 */
int ensr_in_syspath(const struct ensr_config *cfg, const char *path,
                    const char *name);

int ensr_path_owner(const char *path, const char *usr);

/**
 * Platform init code. This is e.g. where pledge(2) could be used on OpenBSD
 */
int ensr_platform_init(void);

#endif
