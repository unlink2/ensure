#ifndef ENSR_H_
#define ENSR_H_

#include <stddef.h>

#define ENSR_PATH_MAX 4096
#define ENSR_COMM_MAX 32

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

// Config variables
#define ENSR_CFG_FMT_OK ""
#define ENSR_CFG_FMT_WARN ""
#define ENSR_CFG_FMT_ERR ""
#define ENSR_CFG_FMT_OFF ""

// env variables
#define ENSR_ENV_FMT_OK "ENSR_OK"
#define ENSR_ENV_FMT_WARN "ENSR_WARN"
#define ENSR_ENV_FMT_ERR "ENSR_ERR"
#define ENSR_ENV_FMT_OFF "ENSR_FMT_OFF"

/**
 * =============================
 * End configuration
 * =============================
 */

enum ensr_mode {
  ENSR_MODE_EQLINES,
  ENSR_MODE_GTLINES,
  ENSR_MODE_LTLINES,
  ENSR_MODE_PID,
  ENSR_MODE_COMM,
  ENSR_MODE_EXISTS,
};

enum ensr_mode ensr_mode_from(const char *s);

struct ensr_config {
  _Bool verbose;

  enum ensr_mode mode;
  const char **inputs;
  size_t inputs_len;

  const char *rule_name;
  const char *proc;
  int pid;

  const char *path;
};

int ensr_main(struct ensr_config *cfg);

#ifdef ENSR_MOD_STDIO

/**
 * Checks that the lines received in stdin
 * are exactly n lines
 * This is useful for piping results from commands
 * like grep into ensure
 */
int ensr_eqnlines(int nlines);
int ensr_gtnlines(int nlines);
int ensr_ltnlines(int nlines);

#endif

#ifdef ENSR_MOD_PROC
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

/**
 * Populate proc struct by pid
 * This will always return a single proc
 */
struct ensr_proc ensr_proc_pid(int pid);

/**
 * Get all procs by comm name
 * This returns a sturct that needs to be freed
 */
struct ensr_proc *ensr_proc_name(const char *comm, size_t *len);

int ensr_comm_running(const char *proc_name);
int ensr_pid_running(int pid);

#endif

#ifdef ENSR_MOD_PATH

/**
 * Check if the name is a valid program
 */
int ensr_in_path(const char *path);

/**
 * Checks if file exists
 */
int ensr_path_exists(const char *path);

int ensr_path_owner(const char *path, const char *usr);

#endif

/**
 * Platform init code. This is e.g. where pledge(2) could be used on OpenBSD
 */
int ensr_platform_init(void);

#endif
