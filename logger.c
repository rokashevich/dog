#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void o(char* format, ...) {
  time_t ltime;
  ltime = time(NULL);
  struct tm* tm;
  tm = localtime(&ltime);
  // Формат метки времени: "2020-10-14 09:28:59 "
  const char timestamp_format[] = "%04d-%02d-%02d %02d:%02d:%02d ";
  fprintf(stdout, timestamp_format, tm->tm_year + 1900, tm->tm_mon, tm->tm_mday,
          tm->tm_hour, tm->tm_min, tm->tm_sec);

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
