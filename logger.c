#include <errno.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
// void print_trace(void) {
//  void *array[10];
//  char **strings;
//  int size, i;

//  size = backtrace(array, 10);
//  strings = backtrace_symbols(array, size);
//  if (strings != NULL) {
//    printf("Obtained %d stack frames.\n", size);
//    for (i = 0; i < size; i++) printf("%s\n", strings[i]);
//  }

//  free(strings);
//}

static pthread_mutex_t lock;

void logger_init() {
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stdout, "pthread_mutex_init():%s\n", strerror(errno));
  }
}

void logger_reset() {}

void printer(const char* suffix, char* format, va_list arg) {
  pthread_mutex_lock(&lock);
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm* ptm = localtime(&ts.tv_sec);
  int tenths_ms = ts.tv_nsec / 1000000L;

  // "[2021-02-05 17:31:35.101] "
  fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%03d]", 1900 + ptm->tm_year,
          ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
          tenths_ms);
  fprintf(stdout, "%s", suffix);

  char* traverse = format;
  unsigned int i;
  char* s;
  while (*traverse) {
    if (*traverse != '%') {
      fprintf(stdout, "%c", *traverse);
      traverse++;
      continue;
    } else
      traverse++;

    switch (*traverse) {
      case 'c':
        i = va_arg(arg, int);
        fprintf(stdout, "%c", i);
        break;

      case 'd':
        i = va_arg(arg, int);
        fprintf(stdout, "%d", i);
        break;

      case 's':
        s = va_arg(arg, char*);
        fprintf(stdout, "%s", s);
        break;

      default:
        putchar(*traverse);
        traverse++;
        continue;
    }
    traverse++;
  }
  fprintf(stdout, "\n");
  fflush(stdout);
  pthread_mutex_unlock(&lock);
}

void o(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(" ", format, args);
  va_end(args);
}

void w(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer("!", format, args);
  va_end(args);
}

void e(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer("*", format, args);
  va_end(args);
}
