#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ensr.h"
#include "argtable2.h"

struct arg_lit *verb = NULL;
struct arg_lit *help = NULL;
struct arg_lit *version = NULL;

struct arg_str *mode = NULL;

struct arg_str *inputs = NULL;

// arg end stores errors
struct arg_end *end = NULL;

#define ensr_argtable                                                          \
  { help, version, verb, mode, inputs, end, }

void ensr_args_free(void) {
  void *argtable[] = ensr_argtable;
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}

void ensr_args_parse(int argc, char **argv) {
  help = arg_litn(NULL, "help", 0, 1, "display this help and exit");
  version = arg_litn(NULL, "version", 0, 1, "display version info and exit");
  verb = arg_litn("v", "verbose", 0, 1, "verbose output");
  mode =
      arg_str0("m", "mode", "MODE",
               "select mode [E(qual)|g(reater)|l(ess)|e(xists)|p(id)|c(omm)]");
  inputs = arg_strn(NULL, NULL, "INPUT", 0, 1, "Input depends on mode");
  end = arg_end(20);

  void *argtable[] = ensr_argtable;

  // output params
  char progname[] = "ensr";
  char short_desc[] = "";

  // version info
  int version_major = 0;
  int version_minor = 0;
  int version_patch = 1;

  int nerrors = arg_parse(argc, argv, argtable);
  int exitcode = 0;

  if (help->count > 0) {
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    printf("%s\n\n", short_desc);
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    goto exit;
  }

  if (version->count) {
    printf("%s version %d.%d.%d\n", progname, version_major, version_minor,
           version_patch);
    goto exit;
  }

  if (nerrors > 0) {
    arg_print_errors(stdout, end, progname);
    printf("Try '%s --help' for more information.\n", progname);
    exitcode = 1;
    goto exit;
  }

  return;
exit:
  ensr_args_free();
  exit(exitcode); // NOLINT
}

int main(int argc, char **argv) {
  ensr_args_parse(argc, argv);

  // map args to cfg here
  struct ensr_config cfg;
  memset(&cfg, 0, sizeof(cfg));

  cfg.verbose = verb->count > 0;
  if (mode->count) {
    cfg.mode = ensr_mode_from(mode->sval[0]);
  }

  if (inputs->count) {
    cfg.input = inputs->sval[0];
  }

  int res = ensr_main(&cfg);

  ensr_args_free();
  return res;
}
