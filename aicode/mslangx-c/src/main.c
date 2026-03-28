#include <stdio.h>
#include <string.h>

static void mslangc_print_help(FILE *stream) {
  fprintf(stream,
    "usage: mslangc [--help] [-e code] [script]\n"
    "\n"
    "Bootstrap CLI for the mslangc runtime.\n");
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    mslangc_print_help(stdout);
    return 0;
  }

  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    mslangc_print_help(stdout);
    return 0;
  }

  if (strcmp(argv[1], "-e") == 0) {
    if (argc < 3) {
      fprintf(stderr, "error: missing argument for -e\n");
      return 1;
    }

    fprintf(stderr, "error: inline execution is not implemented yet\n");
    return 1;
  }

  if (argv[1][0] == '-') {
    fprintf(stderr, "error: unknown option: %s\n", argv[1]);
    return 1;
  }

  fprintf(stderr, "error: script execution is not implemented yet: %s\n",
    argv[1]);
  return 1;
}