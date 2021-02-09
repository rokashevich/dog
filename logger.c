#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void printer(const char* suffix, char* format, ...) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm* ptm = localtime(&ts.tv_sec);
    int tenths_ms = ts.tv_nsec / 1000000L;

    // "[2021-02-05 17:31:35.101] "
    fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d.%d]", 1900 + ptm->tm_year,
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
            tenths_ms);
    fprintf(stdout, "%s", suffix);

    va_list arg;
    va_start(arg, format);
    char* traverse = format;
    unsigned int i;
    char* s;
    while (*traverse) {
      if (*traverse != '%') {
        putchar(*traverse);
        traverse++;
        continue;
      } else
        traverse++;

      switch (*traverse) {
        case 'c':
          i = va_arg(arg, int);
          putchar(i);
          break;

        case 'd':
          i = va_arg(arg, int);
          fprintf(stdout, "%d", i);
          break;

        case 's':
          s = va_arg(arg, char*);
          puts(s);
          break;

        default:
          putchar(*traverse);
          traverse++;
          continue;
      }
      traverse++;
    }
    va_end(arg);
}

void o(char* format, ...) {
    printer("   ", format);
}

void e(char* format, ...) {
    printer(" * ", format);
}

void w(char* format, ...) {
    printer(" ! ", format);
}
