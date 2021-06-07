#include <dirent.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
static void* func() {
  if (fork() == 0) {
    char* args[] = {"/usr/bin/xterm", "-e", "mc", NULL};
    execvp(args[0], args);
    return NULL;
  }
  if (fork() == 0) {
    char* args[] = {"/usr/bin/xterm", "-e", "mc", NULL};
    execvp(args[0], args);
    return NULL;
  }
  while (1) {
    sleep(1);
  }
  return NULL;
}

unsigned long long get_rss_by_pid(unsigned long long rss, const pid_t pid) {
  DIR* d;
  struct dirent* dir;
  char tasks[PATH_MAX];
  sprintf(tasks, "/proc/%i/task", pid);
  d = opendir(tasks);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (!(strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))) continue;
      const pid_t task_pid = atoi(dir->d_name);
      char children[PATH_MAX];
      sprintf(children, "/proc/%i/task/%i/children", pid, task_pid);
      printf("children=%s\n", children);
      FILE* f = fopen(children, "r");
      if (f) {
        char c;
        char chunk[16] = "";
        memset(chunk, 0, sizeof(chunk));
        int chunks = 0;
        while ((c = fgetc(f)) && !feof(f)) {
          if (c == ' ') {
            chunks += 1;
            printf("chunk=%s\n", chunk);
            const pid_t child_pid = atoi(chunk);
            rss += get_rss_by_pid(rss, child_pid);
            memset(chunk, 0, sizeof(chunk));
          } else {
            chunk[strlen(chunk)] = c;
          }
        }
        if (chunks == 0) {
          char statm[PATH_MAX];
          sprintf(statm, "/proc/%i/statm", pid);
          FILE* f = fopen(statm, "r");
          if (!f) return 0;

          if (fscanf(f, "%*d %llu", &rss) != 1) {
            fclose(f);
            return 0;
          }
          unsigned long long page_size =
              (unsigned long long)sysconf(_SC_PAGE_SIZE);
          return rss * page_size;
        }
      }
    }
  }
  closedir(d);
  return rss;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: ./pids pid\n");
    printf("Now start simlation...\n");
    pid_t pid = getpid();
    printf("pid=%i\n", pid);
    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, &func, NULL);
    pthread_create(&thread2, NULL, &func, NULL);
    void* data;
    pthread_join(thread1, &data);
    pthread_join(thread2, &data);
    return 0;
  }
  pid_t test_pid = atoi(argv[1]);
  printf("test_pid=%i\n", test_pid);
  printf("rss=%llu\n", get_rss_by_pid(0, test_pid));
  return 0;
}