#ifndef CIRBUF_H
#define CIRBUF_H

#include <stdio.h>
#include <string.h>

void cirbuf_clear(char* src_buf, int src_siz, unsigned long* src_pos) {
  memset(src_buf, 0, sizeof(src_buf) / sizeof(*src_buf));
  *src_pos = 0;
}

void cirbuf_copy_lines(char* src_buf, int src_siz, unsigned long src_pos,
                       void* dst_buf, int lines, int width) {
  // Отладочный вывод содержимого циклического буфера:
  //  fprintf(stderr, "- CIRBUF BEGIN (pos: %c) --------------------------\n",
  //          src_buf[src_pos]);
  //  for (int i = 0; i < src_siz; ++i) {
  //    fprintf(stderr, "%c", src_buf[i]);
  //  }
  //  fprintf(stderr, "- CIRBUF END --------------------------------------\n");

  char(*dst)[width] = dst_buf;

  unsigned long begin = src_pos;
  unsigned long end;
  for (int line = lines - 1; line >= 0; --line) {
    end = begin > 0 ? begin - 1 : src_siz - 1;
    begin = end > 0 ? end - 1 : src_siz - 1;
    while (src_buf[begin] != '\n') {
      begin = begin > 0 ? begin - 1 : src_siz - 1;
    }
    begin = begin < src_siz ? begin + 1 : 0;
    if (begin < end)
      strncpy(dst[line], src_buf + begin, end - begin);
    else {
      strncpy(dst[line], src_buf + begin, src_siz - begin);
      strncpy(dst[line] + src_siz - begin, src_buf, end);
    }
  }

  // Отладочный вывод получившегося:
  //  fprintf(stderr, "- LINES BEGIN ------------------------------------\n");
  //  for (int i = 0; i < lines; ++i) {
  //    fprintf(stderr, "%d: !%s?\n", i, dst[i]);
  //  }
  //  fprintf(stderr, "- LINES END --------------------------------------\n");
}

#endif  // CIRBUF_H
