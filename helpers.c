#include "helpers.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
// void print_trace(void) {
//  void *array[10];
//  char **strings;
//  int size, i;

//  size = backtrace(array, 10);
//  strings = backtrace_symbols(array, size);
//  if (strings != NULL) {
//    printf("Obtained %d stack frames.\n", size);
//    for (i = 0; i < size; i++) printf("%s\n", strings[i]);
//  }

//  free(strings);
//}

static pthread_mutex_t lock;

void logger_init() {
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stdout, "pthread_mutex_init():%s\n", strerror(errno));
  }
}

void logger_reset() {}

void printer(int with_timestamp, const char* suffix, char* format,
             va_list arg) {
  pthread_mutex_lock(&lock);
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm* ptm = localtime(&ts.tv_sec);

  if (with_timestamp) {
    fprintf(stdout, "%d%02d%02d%02d%02d%02d", ptm->tm_year - 100,
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
            ptm->tm_sec);
    fprintf(stdout, "%s", suffix);
  }
  char* traverse = format;
  unsigned int i;
  char* s;
  while (*traverse) {
    if (*traverse != '%') {
      fprintf(stdout, "%c", *traverse);
      traverse++;
      continue;
    } else
      traverse++;

    switch (*traverse) {
      case 'c':
        i = va_arg(arg, int);
        fprintf(stdout, "%c", i);
        break;

      case 'd':
        i = va_arg(arg, int);
        fprintf(stdout, "%d", i);
        break;

      case 's':
        s = va_arg(arg, char*);
        fprintf(stdout, "%s", s);
        break;

      default:
        putchar(*traverse);
        traverse++;
        continue;
    }
    traverse++;
  }
  if (with_timestamp) fprintf(stdout, "\n");
  fflush(stdout);
  pthread_mutex_unlock(&lock);
}

void o(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(1, " ", format, args);
  va_end(args);
}

void w(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(1, "!", format, args);
  va_end(args);
}

void e(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(1, "*", format, args);
  va_end(args);
}

void d(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(1, ">", format, args);
  va_end(args);
}

void m(char* format, ...) {
  va_list args;
  va_start(args, format);
  printer(0, NULL, format, args);
  va_end(args);
}

char* qstrcat(char* dest, char* src) {
  while (*dest) dest++;
  while ((*dest++ = *src++))
    ;
  return --dest;
}

// warning: this function is not reentrant!
char* itoa(int i) {
  // declare local buffer, and write to it back-to-front
  static char buff[12];
  int ut, ui;
  char minus_sign = 0;
  char* p = buff + sizeof(buff) - 1;
  *p-- = 0;  // nul-terminate buffer

  // deal with negative numbers while using an unsigned integer
  if (i < 0) {
    minus_sign = '-';
    ui = -i;
  } else {
    ui = i;
  }

  // core code here...
  while (ui > 9) {
    ut = ui;
    ui /= 10;
    *p-- = (char)((ut - (ui * 10)) + '0');
  }
  *p = (char)(ui + '0');

  if (minus_sign) *--p = minus_sign;
  return p;
}

// warning: this function is not reentrant!
char* uitoa(unsigned int i) {
  char const digit[] = "0123456789";
  static char b[] = "2147483647";
  char* p = b;
  unsigned int shifter = i;
  do {  // Move to where representation ends
    ++p;
    shifter = shifter / 10;
  } while (shifter);
  *p = '\0';
  do {  // Move back, inserting digits as u go
    *--p = digit[i % 10];
    i = i / 10;
  } while (i);
  return b;
}

int exists(const char* path) {
  struct stat buffer;
  return stat(path, &buffer);
}

char** string_to_string_array(const char* s) {
  // Функция перевода строки вида "DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY",
  // в массив указателей на отдельные элементы для передачи в функцию exec -
  // {"DISPLAY=$DISPLAY", "XAUTHORITY=$XAUTHORITY", NULL}
  size_t s_len = strlen(s) + 1;
  char* c = malloc(s_len * sizeof(char));
  memcpy(c, s, s_len);

  // Подсчитываем кол-во элементов итогового массива.
  // Оно равно кол-ву пробелов в строке, плюс два: признак конца строки \0
  // и под NULL.
  // TODO: Не учитываются внутренние кавычки или обратный слэш экранирования
  //       пробела!
  size_t chunk = 2;
  for (size_t i = 0; i < s_len; ++i) {  // Считаем кол-во пробелов.
    if (c[i] == ' ') {
      c[i] = '\0';
      ++chunk;
    }
  }

  char** t = malloc(chunk * sizeof(char*));
  chunk = 0;
  for (size_t i = 0, chunk_len = 1; i < s_len; ++i, ++chunk_len) {
    if (c[i] == '\0') {
      t[chunk] = malloc(chunk_len * sizeof(char*));  // +1 для \0
      memcpy(t[chunk], c + i - (chunk_len - 1), chunk_len);
      chunk_len = 1;
      ++chunk;
      ++i;
    }
  }
  free(c);
  // t[chunk] = malloc(sizeof(char*));
  t[chunk] = NULL;
  return t;
}

const char** string_to_const_string_array(const char* s) {
  // Функция перевода строки вида "DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY",
  // в массив указателей на отдельные элементы для передачи в функцию exec -
  // {"DISPLAY=$DISPLAY", "XAUTHORITY=$XAUTHORITY", NULL}
  size_t s_len = strlen(s) + 1;
  char* c = malloc(s_len * sizeof(char));
  memcpy(c, s, s_len);

  // Подсчитываем кол-во элементов итогового массива.
  // Оно равно кол-ву пробелов в строке, плюс два: признак конца строки \0
  // и под NULL.
  // TODO: Не учитываются внутренние кавычки или обратный слэш экранирования
  //       пробела!
  size_t chunk = 2;
  for (size_t i = 0; i < s_len; ++i) {  // Считаем кол-во пробелов.
    if (c[i] == ' ') {
      c[i] = '\0';
      ++chunk;
    }
  }
  const char** t = malloc(chunk * sizeof(char*));
  chunk = 0;

  for (size_t i = 0, chunk_len = 1; i < s_len; ++i, ++chunk_len) {
    if (c[i] == '\0') {
      char* p = malloc(chunk_len * sizeof(char*));  // +1 для \0
      // t[chunk] = malloc(chunk_len * sizeof(char *)); // +1 для \0
      memcpy(p, c + i - (chunk_len - 1), chunk_len);
      // memcpy(t[chunk], c+i-(chunk_len-1), chunk_len);
      t[chunk] = (const char*)p;
      chunk_len = 1;
      ++chunk;
      ++i;
    }
  }

  free(c);
  // t[chunk] = malloc(sizeof(char*));
  t[chunk] = NULL;
  return t;
}

const char** make_const_ref_to_string_array(char** s) {
  // Функция для формирования и получения массива ссылок на константные строки
  size_t size_src = 0;
  for (; s[size_src]; ++size_src) {
  };
  const char** list = malloc((size_src + 1) * sizeof(char*));
  for (size_t i = 0; i <= size_src; ++i) {
    list[i] = s[i];
  }
  return list;
}

void free_const_ref_to_string_array(const char** s) {
  free(s);
  s = NULL;
}

char** join_string_arrays(const char** a, const char** b) {
  size_t size_a = 0;
  size_t size_b = 0;
  for (; a[size_a]; ++size_a) {
  };
  for (; b[size_b]; ++size_b) {
  };
  char** list = malloc((size_a + size_b + 1) * sizeof(char*));
  for (int i = 0; a[i]; ++i) {
    size_t len = strlen(a[i]) + 1;
    list[i] = malloc(len * sizeof(char));
    memcpy(list[i], a[i], len);
  }
  for (size_t i = 0; b[i]; ++i) {
    size_t len = strlen(b[i]) + 1;
    list[size_a + i] = malloc(len);
    memcpy(list[size_a + i], b[i], len);
  }
  list[size_a + size_b] = NULL;
  return list;
}

void free_string_array(char** a) {
  for (int i = 0; a[i]; ++i) {
    free(a[i]);
  }
  free(a);
  a = NULL;
}

void free_const_string_array(const char** a) {
  for (int i = 0; a[i]; ++i) {
    char* p;
    memcpy(&p, &(a[i]), sizeof(p));
    free(p);
    // free(a[i]);
  }
  free(a);
  a = NULL;
}

int match(const char* pattern, const char* candidate, int p, int c) {
  if (pattern[p] == '\0') {
    return candidate[c] == '\0';
  } else if (pattern[p] == '*') {
    for (; candidate[c] != '\0'; c++) {
      if (match(pattern, candidate, p + 1, c)) return 1;
    }
    return match(pattern, candidate, p + 1, c);
  } else if (pattern[p] != '?' && pattern[p] != candidate[c]) {
    return 0;
  } else {
    return match(pattern, candidate, p + 1, c + 1);
  }
}

char* strip(char* s) {
  const int len = strlen(s);
  printf("%d-", len);
  return s;
}

void setup_environ_from_string(const char* s) {
  // Очищаем текущее окружение.
  extern char** environ;
  int env_num = 0;
  int max_len = 0;
  for (; environ[env_num]; ++env_num) {
    int cur_len = strlen(environ[env_num]);
    if (cur_len > max_len) max_len = cur_len;
  }
  char env_buf[env_num][max_len];
  for (int i = 0; i < env_num; ++i) {
    strcpy(env_buf[i], environ[i]);
    env_buf[i][strchr(env_buf[i], '=') - env_buf[i]] = 0;
  }
  for (int i = 0; i < env_num; ++i) unsetenv(env_buf[i]);
  // Итерируемся по переданной строке и экспортируем name=val.
  int max_siz = strlen(s);
  char name[max_siz];
  char val[max_siz];
  memset(name, 0, sizeof name / sizeof *name);
  memset(val, 0, sizeof val / sizeof *val);
  bool is_name = true;
  unsigned long pos = 0;
  do {
    const char c = s[pos];
    if (is_name && c != '=') {
      name[strlen(name)] = c;
    } else if (c == '=') {
      is_name = false;
    } else if (!is_name && (c != ' ' && c != 0)) {  // Продвигаемся по val.
      val[strlen(val)] = c;
    } else if (!is_name && (c == ' ' || c == 0)) {  // Дошли до конца val.
      setenv(name, val, 1);
      is_name = true;
      memset(name, 0, sizeof name / sizeof *name);
      memset(val, 0, sizeof val / sizeof *val);
    }
  } while (++pos < strlen(s) + 1);
}

char* strip_ansi_escape_codes(char* s) {
  int j = 0, inside = 0;
  for (int i = 0; s[i] != '\0'; ++i) {
    if (s[i] == '\033') {
      inside = 1;
    }
    s[j] = s[i];
    if (!inside) {
      ++j;
    }
    if (s[i] == 'm') {
      inside = 0;
    }
  }
  s[j - inside] = '\0';
  return s;
}

unsigned long long count_rss(const pid_t pid) {
  return count_rss_recurse(0, pid, 0);
}

unsigned long long count_rss_recurse(unsigned long long rss, const pid_t pid,
                                     int recursion_depth) {
  const int recursion_depth_warning = 16;
  if (recursion_depth > recursion_depth_warning)
    d("recursion_depth=%d", recursion_depth);

  // char depth[] = "---------------------------------------";
  // depth[recursion_depth+1] = 0;

  char tasks[PATH_MAX];
  sprintf(tasks, "/proc/%i/task", pid);
  DIR* dir_stream = opendir(tasks);
  if (dir_stream) {
    // d("%s opened %s",depth, tasks);
    do {
      errno = 0;  // Обнуляем, чтобы отличить ошибку от конца итерации.
      const struct dirent* dir_entry = readdir(dir_stream);
      if (dir_entry == NULL) {
        if (errno) d("readdir(%s):%s", tasks, strerror(errno));
        break;
      }
      if (!(strcmp(dir_entry->d_name, ".") && strcmp(dir_entry->d_name, "..")))
        continue;
      const pid_t task_pid = atoi(dir_entry->d_name);
      char children[PATH_MAX];
      sprintf(children, "/proc/%i/task/%i/children", pid, task_pid);
      // d("%s children %s",depth, children);

      FILE* f1 = fopen(children, "r");
      if (!f1) {
        // d("fopen(%s):%s", children, strerror(errno));
        break;
      }
      char c;
      char chunk[16] = "";
      memset(chunk, 0, sizeof(chunk));
      int chunks = 0;
      while ((c = fgetc(f1)) && !feof(f1)) {
        if (c == ' ') {
          chunks += 1;
          const pid_t child_pid = atoi(chunk);
          // d("%s recurse child pid=%d",depth, child_pid);
          rss += count_rss_recurse(rss, child_pid, recursion_depth + 1);
          memset(chunk, 0, sizeof(chunk));
        } else {
          chunk[strlen(chunk)] = c;
        }
      }
      fclose(f1);

      if (chunks == 0) {
        char statm[PATH_MAX];
        sprintf(statm, "/proc/%i/statm", pid);
        FILE* f2 = fopen(statm, "r");
        if (f2) {
          unsigned long long pages;
          if (fscanf(f2, "%*d %llu", &pages) == 1) {
            unsigned long long page_size =
                (unsigned long long)sysconf(_SC_PAGE_SIZE);
            rss = pages * page_size;
            // d("%s REACHED %s",depth, statm);
          }
          fclose(f2);
        }
      }
    } while (1);
    closedir(dir_stream);
  }
  return rss;
}

char* json_safe(char* text_buf, size_t buf_max_len) {
  char buf[buf_max_len];
  int j = 0;
  for (int i = 0; i < strlen(text_buf), j < buf_max_len; ++i, ++j) {
    const char c = text_buf[i];
    if (c == '\n') {
      buf[j++] = '\\';
      buf[j++] = '\\';
      buf[j] = 'n';
    } else if (c == '"') {
      buf[j++] = '\\';
      buf[j] = '"';
    } else
      buf[j] = c;
  }
  buf[j] = 0;
  strcpy(text_buf, buf);
}
