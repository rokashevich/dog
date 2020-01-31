#ifndef DOG_H
#define DOG_H

#include <pthread.h>
#include <unistd.h>
#include "mongoose.h"

struct Msg {
  char *msg;
  struct Msg *next;

  int action;
};

void gen_json(struct Data *data);

void handle_status(struct mg_connection *nc);

void *process_worker(void *voidprocess);

void handle_watch(struct mg_connection *nc, struct http_message *hm);

void handle_df(struct mg_connection *nc, struct http_message *hm);

void handle_undf(struct mg_connection *nc);

// Останавливает процесс по его уникальному идентификатору,
// но не удаляет его из списка для возможности повторного запуска.
// Для возможности удалённого останова отдельного процесса,
// правки его конфига, например, и перезапуска - часто надо такое.
void handle_pause(struct mg_connection *nc, struct http_message *hm);

// Запускает остановленный предыдущей функцией процесс.
void handle_resume(struct mg_connection *nc, struct http_message *hm);

void handle_message(struct mg_connection *nc, struct http_message *hm);

void handle_killall(struct mg_connection *nc);

void handle_ping(struct mg_connection *nc, struct http_message *hm);

#endif  // DOG_H
