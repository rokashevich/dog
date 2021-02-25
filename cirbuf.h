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
  // Очищаем буфер назначения.
  memset(dst_buf, 0, lines * width);

  // Отладочный вывод содержимого циклического буфера:
  //  fprintf(stderr, "- CIRBUF BEGIN (pos: %c) --------------------------\n",
  //          src_buf[src_pos]);
  //  for (int i = 0; i < src_siz; ++i) {
  //    fprintf(stderr, "%c", src_buf[i]);
  //  }
  //  fprintf(stderr, "- CIRBUF END --------------------------------------\n");

  char(*dst)[width] = dst_buf;

  unsigned long pos = src_pos > 0 ? src_pos - 1 : src_siz - 1;
  while (pos != src_pos) {
    pos = pos > 0 ? pos - 1 : src_siz - 1;
    if (src_buf[pos] == '\n') --lines;
    if (src_buf[pos] == 0 || lines == 0) {
      pos = pos < src_siz - 1 ? pos + 1 : 0;
      int l = 0;
      int w = 0;
      while (pos != src_pos) {
        if (src_buf[pos] != '\n') {
          if (w < width) dst[l][w++] = src_buf[pos];
        } else {
          dst[l][w] = 0;
          l++;
          w = 0;
        }
        pos = pos < src_siz - 1 ? pos + 1 : 0;
      }
      break;
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
