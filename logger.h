#ifndef LOGGER_H
#define LOGGER_H

void logger_init();

// Функция записывает в лог то, что ей передали, o === output.
void o(char *, ...);
void w(char *, ...);
void e(char *, ...);
void m(char *, ...);

#endif  // LOGGER_H
