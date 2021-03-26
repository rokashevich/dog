#include <stdio.h>
#include <unistd.h>
int main() {
  while (1) {
    fprintf(stderr, "\033[0;30m");  // Чёрный.
    fprintf(stderr, "Black\n");

    fprintf(stderr, "\033[0;31m");  // Красный.
    fprintf(stderr, "Red\n");

    fprintf(stderr, "\033[0;32m");  // Зелёный.
    fprintf(stderr, "Green\n");

    fprintf(stderr, "\033[0;33m");  // Жёлтый
    fprintf(stderr, "Yellow\n");

    fprintf(stderr, "\033[0;34m");  // Синий.
    fprintf(stderr, "Blue\n");

    fprintf(stderr, "\033[0;35m");  // Фиолетовый.
    fprintf(stderr, "Purple\n");

    fprintf(stderr, "\033[0;36m");  // Голубой.
    fprintf(stderr, "Cyan\n");

    fprintf(stderr, "\033[0;37m");  // Белый.
    fprintf(stderr, "White\n");

    fprintf(stderr, "\033[0m");  // Дефолтный.
    fprintf(stderr, "Default\n");

    // https://superuser.com/questions/380772/removing-ansi-color-codes-from-text-stream
    // https://en.wikipedia.org/wiki/ANSI_escape_code

    sleep(1);
  }
}
