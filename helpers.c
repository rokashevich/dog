#include "helpers.h"

#include <ctype.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

unsigned long long get_rss_by_pid(unsigned long long rss, const pid_t pid) {
  DIR* d;
  FILE *f1, *f2;
  f1 = f2 = NULL;
  struct dirent* dir;
  char tasks[PATH_MAX];
  sprintf(tasks, "/proc/%i/task", pid);
  d = opendir(tasks);
  if (!d) return rss;
  while ((dir = readdir(d)) != NULL) {
    if (!(strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))) continue;
    const pid_t task_pid = atoi(dir->d_name);
    char children[PATH_MAX];
    sprintf(children, "/proc/%i/task/%i/children", pid, task_pid);
    f1 = fopen(children, "r");
    if (!f1) break;

    char c;
    char chunk[16] = "";
    memset(chunk, 0, sizeof(chunk));
    int chunks = 0;
    while ((c = fgetc(f1)) && !feof(f1)) {
      if (c == ' ') {
        chunks += 1;
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
      f2 = fopen(statm, "r");
      if (!f2) break;
      unsigned long long pages;
      if (fscanf(f2, "%*d %llu", &pages) != 1) break;
      unsigned long long page_size = (unsigned long long)sysconf(_SC_PAGE_SIZE);
      const unsigned long long new_rss = pages * page_size;
      return new_rss;
    }
  }
  if (f1) fclose(f1);
  if (f2) fclose(f2);
  if (d) closedir(d);
  return rss;
}
