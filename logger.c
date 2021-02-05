#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

const static int circular_buffer_size = 100;
static char circular_buffer[100];
static int circular_buffer_pos = 0;
void circular_buffer_add_text(const char* text) {
  unsigned long buffer_right_side_size =
      circular_buffer_size - circular_buffer_pos;
  const unsigned long text_len = (unsigned long)strlen(text);
  if (buffer_right_side_size >= text_len) {
    memcpy(circular_buffer + circular_buffer_pos, text, text_len);
    circular_buffer_pos += text_len;
  } else {
    memcpy(circular_buffer + circular_buffer_pos, text, buffer_right_side_size);
    circular_buffer_pos = text_len - buffer_right_side_size;
    memcpy(circular_buffer, text + buffer_right_side_size, circular_buffer_pos);
  }
}
void circular_buffer_write1(const char* text) {
  unsigned long length = 0;
  for (unsigned long i = circular_buffer_pos; i < circular_buffer_size;
       ++i, ++length) {
    char c = circular_buffer[i];
    if (c == '\0') break;
    circular_buffer[length] = c;
  }
  for (int i = 0; i < circular_buffer_pos; ++i, ++length) {
    char c = circular_buffer[i];
    circular_buffer[length] = c;
  }
  circular_buffer[length] = '\0';
}

void o(char* format, ...) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm* ptm = localtime(&ts.tv_sec);
  int tenths_ms = ts.tv_nsec / 1000000L;

  // "[2021-02-05 17:31:35.101] " - 26 символов
  const int timestamp_length = 26;
  char buffer[timestamp_length];

  snprintf(buffer, timestamp_length, "[%04d-%02d-%02d %02d:%02d:%02d.%d] ",
           1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,
           ptm->tm_min, ptm->tm_sec, tenths_ms);
  circular_buffer_add_text(buffer);
  for (unsigned long i = 0; i < circular_buffer_size; ++i) {
    fprintf(stdout, "%c", circular_buffer[i]);
  }
  fprintf(stdout, "\n");

  va_list arg;
  va_start(arg, format);
  char* traverse = format;
  unsigned int i;
  char* s;
  while (*traverse) {
    if (*traverse != '%') {
      /// putchar(*traverse);
      traverse++;
      continue;
    } else
      traverse++;

    switch (*traverse) {
      case 'c':
        i = va_arg(arg, int);
        /// putchar(i);
        break;

      case 'd':
        i = va_arg(arg, int);
        /// fprintf(stdout, "%d", i);
        break;

      case 's':
        s = va_arg(arg, char*);
        /// puts(s);
        break;

      default:
        /// putchar(*traverse);
        traverse++;
        continue;
    }
    traverse++;
  }
  va_end(arg);
}
