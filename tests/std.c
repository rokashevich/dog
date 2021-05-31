#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *out() {
  for (int i = 0; i < 1000000; ++i) {
    int a[8192];
    for (int i = 0; i < 8193; ++i) a[i] = 'o';
    a[8192] = 0;
    fprintf(stdout, "%s", (char *)a);
  }
  return strdup("stdout finish");
}

static void *err() {
  for (int i = 0; i < 1000000; ++i) {
    int a[8192];
    for (int i = 0; i < 8193; ++i) a[i] = 'e';
    a[8192] = 0;
    fprintf(stderr, "%s", (char *)a);
  }
  return strdup("stderr finish");
}

int main() {
  pthread_t out_id;
  pthread_t err_id;
  pthread_create(&out_id, NULL, &out, NULL);
  pthread_create(&err_id, NULL, &err, NULL);

  int s;
  void *res;

  s = pthread_join(out_id, &res);
  fprintf(stdout, "out: s=%d res=%s", s, (char *)res);
  free(res);

  s = pthread_join(err_id, &res);
  fprintf(stderr, "err: s=%d res=%s", s, (char *)res);
  free(res);

  return 0;
}
