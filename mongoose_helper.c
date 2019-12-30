#include "mongoose_helper.h"
#include <string.h>
int mg_get_query_string_var(const struct mg_str *buf, const char *name,
                            char *dst, size_t dst_len) {
  int len = 0;
  int dst_last_available_index = dst_len - 1;
  const int matches_total = strlen(name);  // Длина "name".
  int matches_left = matches_total;
  char previouse_char = '&';
  for (size_t i = 0; i < buf->len; ++i) {
    char current_query_char = buf->p[i];
    if (previouse_char == '&' && current_query_char == name[0]) {
      // Начало возможного совпадения "&name=" - "&" и первый символ "name".
      matches_left = matches_total - 1;
    } else if (matches_left < matches_total && matches_left > 0 &&
               current_query_char == name[matches_total - matches_left]) {
      // В процессе сопоставления "&name=".
      --matches_left;
    } else if (matches_left == 0 && current_query_char == '=') {
      // Проверили, что "name" оканчивается знаком "=". Выставляем matches_left
      // в -1 чтобы знать, что с этой позиции идёт значение параметра.
      matches_left = -1;
    } else if (matches_left == -1 && current_query_char != '&') {
      // Посимвольно считываем значение параметра, но не больше, чем dst_len.
      if (len < dst_last_available_index) {
        dst[len++] = current_query_char;
      }
    } else {
      matches_left = matches_total;
    }
    previouse_char = buf->p[i];
  }
  dst[len] = '\0';
  return len;
}
