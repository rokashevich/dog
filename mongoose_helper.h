#ifndef MONGOOSE_HELPER_H
#define MONGOOSE_HELPER_H
#include "mongoose.h"
// В mongoose почему-то нет встроенной функции получения значений параметров,
// поэтому сделал такую, аналогичную существующей mg_get_http_var для POST
// запросов.
int mg_get_query_string_var(const struct mg_str *buf, const char *name,
                            char *dst, size_t dst_len);
#endif // MONGOOSE_HELPER_H
