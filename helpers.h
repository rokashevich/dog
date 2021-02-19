#ifndef HELPERS_H
#define HELPERS_H

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

#endif  // HELPERS_H
