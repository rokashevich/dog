#include "cirbuf.h"

#include <stdio.h>
#include <string.h>

void cirbuf_takeout(const char* src, size_t pos, char* dst, int siz) {
  const size_t offset = siz - pos;
  memcpy(dst, src + pos, offset);
  memcpy(dst + offset, src, pos);
  dst[siz - 1] = 0;
}

void cirbuf_fill(char* src, size_t* pos, int c) {
  memset(src, c, cirbuf_size);
  *pos = 0;
}

void cirbuf_push(const char* s, size_t n, char* cirbuf, size_t* pos) {
  // s - строку, которую надо запушить в буфер,
  // n - длина этой строки,
  // cifbuf - сам буфер,
  // pos - текущее смещение.
  if (n > cirbuf_size) {  // Впихиваемая s больше целого буфера.
    memcpy(cirbuf, s + n - cirbuf_size, cirbuf_size);
    *pos = 0;
  } else {
    const size_t bytes_right = cirbuf_size - 1 - *pos;
    if (bytes_right >= n) {  // Справа достаточно байт для n.
      // Впихиваемая s помещается в правую часть буфера.
      memcpy(cirbuf + *pos, s, n);
      *pos = n == bytes_right ? 0 : *pos + n;
    } else {  // Справа не достаточно байт для n, часть будет слева.
      memcpy(cirbuf + *pos, s, bytes_right);
      *pos = n - bytes_right;
      memcpy(cirbuf, s + n - *pos, *pos);
    }
  }
}
