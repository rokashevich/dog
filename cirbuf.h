#ifndef CIRBUF_H
#define CIRBUF_H
#include <stdio.h>
enum { cirbuf_size = 1024 };
void cirbuf_takeout(const char* src, size_t pos, char* dst);
void cirbuf_fill(char* src, size_t* pos, int c);
void cirbuf_push(const char* s, size_t n, char* cirbuf, size_t* pos);

#endif  // CIRBUF_H
