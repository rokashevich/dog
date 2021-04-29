#include <pthread.h>
#include <stdio.h>

int main() {
  for (int i = 1; i <= 100; ++i) {
    fprintf(stderr, "abcdefghijklmnopqrstuwxyz%d\n", i);
    // fprintf(stdout, "TEXT TEXT TEXT TEXT TEXT TEXT TEXT STDOUT %d\n",i);
    // fprintf(stderr, "TEXT TEXT TEXT TEXT TEXT TEXT TEXT STDERR %d\n",i);
  }
  return 1 / 0;
}
