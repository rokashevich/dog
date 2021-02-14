#ifndef CIRBUF_H
#define CIRBUF_H

#include <stdio.h>
#include <string.h>

void cirbuf_get_nth_line(int n, int* begin, int* end) {}

void cirbuf_copy_lines(char* src_buf, int src_siz, int src_pos, void* dst_buf,
                       int lines, int width) {
  fprintf(stderr, "--------------------------------------------------\n");
  fprintf(stderr, "%c\n", src_buf[src_pos]);
  fprintf(stderr, "--------------------------------------------------\n");
  for (int i = 0; i < src_siz; ++i) {
    fprintf(stderr, "%c", src_buf[i]);
  }
  fprintf(stderr, "--------------------------------------------------\n");
  char(*dst)[width] = dst_buf;

  int begin = src_pos;
  int end;
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
  fprintf(stderr, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  for (int i = 0; i < lines; ++i) {
    fprintf(stderr, "%d: !%s?\n", i, dst[i]);
  }
}

#endif  // CIRBUF_H
