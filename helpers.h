#ifndef HELPERS_H
#define HELPERS_H

#include <sys/types.h>

// ЛОГГЕР
void logger_init();
void o(char*, ...);  // 210603130702 ... информация
void w(char*, ...);  // 210603130702!... внимание (проблема со щенками)
void e(char*, ...);  // 210603130702*... ошибка (самой собаки)
void d(char*, ...);  // 210603130702>... отладочные сообщения
void m(char*, ...);  // обычный printf без префикса и \n

// ДРУГОЕ
char* qstrcat(char* dest, char* src);

char* itoa(int i);
char* uitoa(unsigned int i);
int exists(const char*);

char** string_to_string_array(const char*);
char** join_string_arrays(const char**, const char**);
void free_string_array(char** a);

const char** string_to_const_string_array(const char* s);
void free_const_string_array(const char** a);

const char** make_const_ref_to_string_array(char** s);
void free_const_ref_to_string_array(const char** s);

// Рекурсивная функция поиска соответсвтия шаблона строке.
// Использование: match(pattern, string, 0, 0))
// В случае успеха возвращает 1.
int match(const char* pattern, const char* candidate, int p, int c);

// Удаляет начальные и конечные пробелы, конечный символ перехода
// на новую строку.
char* strip(char* s);

// Очищаем окружение и выставляем новое из строки вида "A=a B=b C=$B:d"
void setup_environ_from_string(const char* s);

// Удаляет управляющие последовательности (ANSI escape code)
// т.е. всё между символами \033[ и m, включая их самих.
char* strip_ansi_escape_codes(char* s);

unsigned long long count_rss(const pid_t pid);
unsigned long long count_rss_recurse(unsigned long long rss, const pid_t pid,
                                     int depth);

#endif  // HELPERS_H
