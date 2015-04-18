#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include "util.h"
#include "rw.h"

#define TMP_SUFFIX "/rw-XXXXXX.tmp"

static rw_t *rw;
const char *program_name = NULL;

static struct option long_options[] = {
  { "help",       no_argument,       NULL, 'h' },
  { "append",     no_argument,       NULL, 'a' },
  { "block-size", required_argument, NULL, 'b' },
  { "temp-dir",   required_argument, NULL, 't' },
  { "verbose",    no_argument,       NULL, 'v' },
  { NULL, 0, NULL, 0 }
};

static void print_help(void) {
  static const char *help =
    "Usage: %s [OPTION]... [FILE]...\n"
    "Pipe the contents of stdin to each FILE (or stdout if no FILE argument).\n\n"
    "  -a, --append             append to each FILE, do not overwrite\n"
    "  -b, --block-size=SIZE    use a block-size of SIZE bytes when copying\n"
    "                           files (default 4096)\n"
    "  -t, --temp-dir=DIR       use DIR to create the intermediate file to be\n"
    "                           copied to FILE (default \"/tmp\")\n"
    "  -v, --verbose            redirect to each FILE in addition to stdout\n"
    "  -h, --help               display this help and exit\n\n"
    "Example:\n"
    "  # this command won't work:\n"
    "  cat example.txt | uniq | sort > example.txt\n"
    "  # rw can make this happen:\n"
    "  cat example.txt | uniq | sort | %s example.txt\n";
  printf(help, program_name, program_name);
}

static void sighandler(int signum __attribute__((unused))) {
  if (rw != NULL && rw->is_open)
    unlink(rw->template);
  exit(EXIT_FAILURE);
}

static void setupt_signal_handlers(void) {
  struct sigaction sa = { .sa_handler = sighandler };
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    error(0, errno, "error setting up signal handlers");
  }
}

int main(int argc, char **argv) {
  program_name = argv[0];
  const char *tmp_dir = NULL;
  size_t rw_block_size = 4096;

  bool verbose_flag = false,
       append_flag = false;

  int c;
  while ((c = getopt_long(argc, argv, "avht:b:", long_options, NULL)) != -1) {
    switch (c) {
    case 'b':
      rw_block_size = strtol(optarg, NULL, 10);
      if (rw_block_size <= 0 || errno == ERANGE) {
        error(EXIT_FAILURE, 0, "invalid number for "
            "\"--byte-size\" option");
      }
      break;
    case 'a':
      append_flag = true;
      break;
    case 't':
      tmp_dir = optarg;
      break;
    case 'h':
      print_help();
      exit(EXIT_SUCCESS);
    case 'v':
      verbose_flag = true;
      break;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
      exit(EXIT_FAILURE);
    default:
      exit(EXIT_FAILURE);
    }
  }

  setupt_signal_handlers();
  if (optind < argc) {
    if ((rw = rw_create(tmp_dir, rw_block_size, append_flag)) == NULL)
      exit(EXIT_FAILURE);
    if (rw_stream(rw) == -1)
      exit(EXIT_FAILURE);
    if (verbose_flag && rw_write_stdout(rw) == -1)
      exit(EXIT_FAILURE);
    for (int i = optind; i < argc; i++) {
      if (rw_write(rw, argv[i]) == -1)
        exit(EXIT_FAILURE);
    }
    rw_free(rw);
  } else {
    fd_copy(STDIN_FILENO, STDOUT_FILENO, rw_block_size);
  }
}
