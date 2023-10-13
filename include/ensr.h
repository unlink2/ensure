#ifndef ENSR_H_
#define ENSR_H_

/**
 * =============================
 * Configuration
 * Enable and disable features
 * dynamically here
 * =============================
 */

// Optional modules
#define ENSR_MOD_IN_PATH 1
#define ENSR_MOD_PATH_EXISTS 1
#define ENSR_MOD_PROC_RUNNING 1
#define ENSR_MOD_PID_RUNNING 1
#define ENSR_MOD_PATH_OWNER 1

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

struct ensr_config {
  _Bool verbose;

  const char *rule_name;
  const char *proc;
  int pid;

  const char *path;
};

int ensr_main(struct ensr_config *cfg);

/**
 * Process related stuff
 * on linux we read proc(5)
 * on bsd we will use sysctl(2)
 * for tests, use a dummy implementation
 */

struct ensr_proc {
  int pid;
  const char *comm;
  int uid;
};

struct ensr_proc ensr_proc_pid(int pid);
struct ensr_proc ensr_proc_name(const char *comm);

/**
 * Checks that the lines received in stdin
 * are exactly n lines
 * This is useful for piping results from commands
 * like grep into ensure
 */
int ensr_eqnlines(int nlines);
int ensr_gtnlines(int nlines);
int ensr_ltnlines(int nlines);

/**
 * Check if the name is a valid program
 */
int ensr_in_path(const char *path);

/**
 * Checks if file exists
 */
int ensr_path_exists(const char *path);

int ensr_path_owner(const char *path, const char *usr);

int ensr_proc_running(const char *proc_name);
int ensr_pid_running(int pid);

int ensr_platform_init(void);

#endif
