// for using execvpe we have to define macro
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "main.h"

#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>

#include "cirbuf.h"
#include "data.h"
#include "defines.h"
#include "helpers.h"
#include "list.h"
#include "logger.h"
#include "mongoose.h"
#include "mongoose_helper.h"

static pthread_mutex_t lock;

void gen_json(struct Data *data) {
  const int s = 4096;  // оперативный буфер, большой не нужен
  char b[s];
  char *p = data->json;
  data->json[0] = '\0';
  p = qstrcat(p, "{");

  p = qstrcat(p, "\"hostname\":\"");
  p = qstrcat(p, data->hostname);
  p = qstrcat(p, "\",");

  p = qstrcat(p, "\"env\":\"");
  p = qstrcat(p, data->env);
  p = qstrcat(p, "\",");

  p = qstrcat(p, "\"pwd\":\"");
  p = qstrcat(p, data->pwd);
  p = qstrcat(p, "\",");

  p = qstrcat(p, "\"timestamp\":");
  p = qstrcat(p, data->timestamp);
  p = qstrcat(p, ",\"boot_id\":\"");
  p = qstrcat(p, data->boot_id);
  p = qstrcat(p, "\",");
  p = qstrcat(p, "\"uptime\":");
  snprintf(b, s, "%ld,", data->uptime);
  p = qstrcat(p, b);
  snprintf(b, s, "\"loadavg\":%.2f,", data->loadavg);
  p = qstrcat(p, b);
  p = qstrcat(p, "\"cpu\":{");
  p = qstrcat(p, "\"usage\":");
  snprintf(b, s, "%u", data->cpu.usage);
  p = qstrcat(p, b);
  strcat(data->json, ",");
  p = qstrcat(p, "\"temperature\":");
  snprintf(b, s, "%u", data->cpu.temperature);
  p = qstrcat(p, b);
  strcat(data->json, ",");
  p = qstrcat(p, "\"cores\":[");
  for (int i = 0; i < data->cpu.count; ++i) {
    p = qstrcat(p, "{\"usage\":");
    snprintf(b, s, "%u", data->cpu.cores_usage[i]);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (i < data->cpu.count - 1) p = qstrcat(p, ",");
  }
  p = qstrcat(p, "]");
  p = qstrcat(p, "},");
  p = qstrcat(p, "\"ram\":{");
  p = qstrcat(p, "\"total\":");
  snprintf(b, s, "%lld", data->ram.total);
  p = qstrcat(p, b);
  p = qstrcat(p, ",");
  p = qstrcat(p, "\"usage\":");
  snprintf(b, s, "%u", data->ram.usage);
  p = qstrcat(p, b);
  p = qstrcat(p, "},");

  p = qstrcat(p, "\"disks\":[");
  struct Disk *disk;
  SL_FOREACH(data->disks_head, disk) {
    p = qstrcat(p, "{");
    p = qstrcat(p, "\"path\":\"");
    p = qstrcat(p, disk->path);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"total\":");
    snprintf(b, s, "%lld,", disk->total);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"used\":");
    snprintf(b, s, "%lld", disk->used);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (disk->next) p = qstrcat(p, ",");
  }
  p = qstrcat(p, "],");

  p = qstrcat(p, "\"net\":[");
  for (unsigned int i = 0; i < data->net_count; i++) {
    p = qstrcat(p, "{");
    p = qstrcat(p, "\"iface\":\"");
    p = qstrcat(p, data->net[i].iface);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"ip\":\"");
    p = qstrcat(p, data->net[i].ip);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"hw\":\"");
    p = qstrcat(p, data->net[i].hw);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"carrier\":");
    snprintf(b, s, "%u,", data->net[i].carrier);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"speed\":");
    snprintf(b, s, "%u,", data->net[i].speed);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"rx\":");
    snprintf(b, s, "%lld,", data->net[i].rx);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"tx\":");
    snprintf(b, s, "%lld,", data->net[i].tx);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"current_rx_speed\":");
    snprintf(b, s, "%lld,", data->net[i].current_rx_speed);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"current_tx_speed\":");
    snprintf(b, s, "%lld,", data->net[i].current_tx_speed);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"max_rx_speed\":");
    snprintf(b, s, "%lld,", data->net[i].max_rx_speed);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"max_tx_speed\":");
    snprintf(b, s, "%lld", data->net[i].max_tx_speed);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (i < data->net_count - 1) p = qstrcat(p, ",");
  }
  p = qstrcat(p, "],");
  p = qstrcat(p, "\"watching\":[");
  struct Process *process;
  SL_FOREACH(data->processes_head, process) {
    p = qstrcat(p, "{");
    p = qstrcat(p, "\"id\":");
    snprintf(b, s, "%u", process->id);
    p = qstrcat(p, b);
    p = qstrcat(p, ",");
    p = qstrcat(p, "\"pwd\":\"");
    p = qstrcat(p, process->pwd);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"env\":\"");
    p = qstrcat(p, process->env);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"cmd\":\"");
    p = qstrcat(p, process->cmd);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"pid\":");
    snprintf(b, s, "%u", process->pid);
    p = qstrcat(p, b);
    p = qstrcat(p, ",\"rss\":");
    snprintf(b, s, "%llu", process->rss);
    p = qstrcat(p, b);
    p = qstrcat(p, ",\"restarts_counter\":");
    snprintf(b, s, "%u", process->restarts_counter);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (process->next) p = qstrcat(p, ",");
  }
  p = qstrcat(p, "],");
  p = qstrcat(p, "\"msgs\":[");
  struct Msg *msg;
  SL_FOREACH(data->msgs_head, msg) {
    p = qstrcat(p, "\"");
    p = qstrcat(p, msg->msg);
    p = qstrcat(p, "\"");
    if (msg->next) p = qstrcat(p, ",");
  }
  p = qstrcat(p, "]");
  p = qstrcat(p, "}");
}

void handle_status(struct mg_connection *nc) {
  pthread_mutex_lock(&lock);

  struct Data *data = get_data();
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc, data->json);
  mg_send_http_chunk(nc, "", 0);

  pthread_mutex_unlock(&lock);
}

void *process_worker(void *voidprocess) {
  struct Data *data = get_data();

  int fd[2];
  if (pipe(fd)) {
    e("pipe():%s", strerror(errno));
    return NULL;
  }

  struct Process *process = (struct Process *)voidprocess;
  pid_t pid = fork();

  while (1) {
    if (pid == 0) {  // CHILD
      prctl(PR_SET_PDEATHSIG, SIGKILL);

      close(fd[PIPE_READ]);
      while ((dup2(fd[PIPE_WRITE], STDOUT_FILENO) == -1) && (errno == EINTR))
        ;
      while ((dup2(fd[PIPE_WRITE], STDERR_FILENO) == -1) && (errno == EINTR))
        ;
      close(fd[PIPE_WRITE]);

      if (strlen(process->pwd)) {
        if (chdir(process->pwd) != 0) {
          fprintf(stderr, "chdir(%s):%s\n", process->pwd, strerror(errno));
          exit(1);
        }
      }

      // Выставляем переменные окружения.
      if (strlen(process->env))
        setup_environ_from_string(process->env);
      else if (strlen(data->env))
        setup_environ_from_string(data->env);

      // Запускаем процесс.
      char **cmds = string_to_string_array(process->cmd);
      extern char **environ;
      execvpe(cmds[0], cmds, environ);
      fprintf(stderr, "***********\n");
      exit(1);
    }
    // PARENT
    pthread_mutex_lock(&lock);
    process->pid = pid;
    pthread_mutex_unlock(&lock);

    int status = -1;
    ssize_t nread;
    char buffer[1024];
    close(fd[PIPE_WRITE]);
    while (1) {
      int flags = fcntl(fd[PIPE_READ], F_GETFL, 0);
      fcntl(fd[PIPE_READ], F_SETFL, flags | O_NONBLOCK);
      nread = read(fd[PIPE_READ], &buffer[0], sizeof(buffer));

      if (nread == -1) {
        if (errno == EINTR) {
          continue;
        } else if (errno == EAGAIN) {
          if (waitpid(process->pid, &status, WNOHANG) == 0) {
            sleep(1);
            continue;
          } else {
            break;
          }
        } else {
          break;
        }

      } else if (nread == 0)
        break;

      // Обновляем циклически буфер процесса в общей стркутре.
      pthread_mutex_lock(&lock);
      const int cir_buf_siz =
          sizeof(process->circular_buffer) / sizeof(*process->circular_buffer);
      unsigned long buffer_right_side_size =
          sizeof(process->circular_buffer) / sizeof(*process->circular_buffer) -
          process->circular_buffer_pos;
      if (nread > cir_buf_siz) {
        memcpy(process->circular_buffer, buffer + (nread - cir_buf_siz),
               (unsigned long)cir_buf_siz);
        process->circular_buffer_pos = 0;
      } else if (buffer_right_side_size >= (unsigned long)nread) {
        // Полученный stdout/stderr влезает в правую часть буфера целиком.
        memcpy(process->circular_buffer + process->circular_buffer_pos, buffer,
               (unsigned long)nread);
        process->circular_buffer_pos += (unsigned long)nread;
      } else {  // Полученный stdout/stderr надо разбивать на две части.
        memcpy(process->circular_buffer + process->circular_buffer_pos, buffer,
               buffer_right_side_size);
        process->circular_buffer_pos =
            (unsigned long)nread - buffer_right_side_size;
        memcpy(process->circular_buffer, buffer + buffer_right_side_size,
               process->circular_buffer_pos);
      }
      pthread_mutex_unlock(&lock);
    }
    close(fd[PIPE_READ]);

    // Обрабатываем завершение процесса.
    if (status == -1)
      while (waitpid(pid, &status, 0) == -1)
        if (errno != EINTR) e("waitpid()%s", strerror(errno));
    pthread_mutex_lock(&lock);
    int siz = sizeof(process->previous_exit_reason) /
              sizeof(*process->previous_exit_reason);
    if (WIFEXITED(status)) {
      snprintf(process->previous_exit_reason, siz, "Exit code %d",
               WEXITSTATUS(status));
    } else if (WIFSTOPPED(status)) {
      snprintf(process->previous_exit_reason, siz,
               "Child stopped by signal %d (%s)", WSTOPSIG(status),
               strsignal(WSTOPSIG(status)));
    } else if (WIFSIGNALED(status)) {
      snprintf(process->previous_exit_reason, siz,
               "Child killed by signal %d (%s)", WTERMSIG(status),
               strsignal(WTERMSIG(status)));
    } else
      snprintf(process->previous_exit_reason, siz, "Exit reason unknown!");
    o("quit %d│%s│%s", process->restarts_counter, process->previous_exit_reason,
      process->cmd);
    const int lines = sizeof(process->previous_exit_log) /
                      sizeof(**process->previous_exit_log) /
                      sizeof(*process->previous_exit_log);
    for (int i = 0; i < lines; ++i) {
      const char *line = process->previous_exit_log[i];
      if (strlen(line)) m("│%s\n", line);
    }

    // Решаем, что делать с процессом дальше.

    if (process->action == ACTION_KILL) {
      SL_DELETE(data->processes_head, process);
      free(process);
      pthread_mutex_unlock(&lock);
      break;
    } else if (process->action == ACTION_PAUSE) {
      process->pid = -1;
      process->restarts_counter = 0;
      pthread_mutex_unlock(&lock);
      break;
    }
    // Процесс завершился самопроизвольно - перезапускаем!
    // Сохраняем из буффера перехваченных stdout/stderr последние n строк
    // в буффер хранения окончания вывода погибшего процесса.
    const int src_siz =
        sizeof(process->circular_buffer) / sizeof(*process->circular_buffer);
    const int width = sizeof(*process->previous_exit_log);
    cirbuf_copy_lines(process->circular_buffer, src_siz,
                      process->circular_buffer_pos, process->previous_exit_log,
                      lines, width);
    cirbuf_clear(process->circular_buffer, src_siz,
                 &process->circular_buffer_pos);
    process->restarts_counter++;

    if (pipe(fd) == -1) {
      e("pipe():%s", strerror(errno));
      return NULL;
    }
    pthread_mutex_unlock(&lock);
    sleep(SLEEP);
    pid = fork();
  }
  return voidprocess;
}

void handle_watch(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Process *new_process = malloc(sizeof(struct Process));

  // Счётчик последнего выданного id.
  static unsigned int largest_id = 0;
  new_process->id = ++largest_id;

  // Читаем HTTP POST запрос вида:
  // curl -X POST http://localhost:14157/watch -d "
  // pwd=/opt
  // &env=LD_LIBRARY_PATH=.:../lib DISPLAY=$DISPLAY
  // &cmd=/usr/bin/app -a -r -g -u -m -e -n -t -s"
  // и заполняем соотвествующие поля в структуре Process.

  char buf[1024];

  mg_get_http_var(&hm->body, "pwd", buf, sizeof(buf));
  strncpy(new_process->pwd, buf,
          sizeof new_process->pwd / sizeof *new_process->pwd);

  mg_get_http_var(&hm->body, "env", buf, sizeof(buf));
  strncpy(new_process->env, buf,
          sizeof new_process->env / sizeof *new_process->env);

  mg_get_http_var(&hm->body, "cmd", buf, sizeof(buf));
  strncpy(new_process->cmd, buf,
          sizeof new_process->cmd / sizeof *new_process->cmd);

  new_process->circular_buffer_pos = 0;
  memset(new_process->circular_buffer, '\0',
         sizeof(new_process->circular_buffer) /
             sizeof(*new_process->circular_buffer));

  // Инициаллизация нулями буферов-хранилищ информации о падении процесса.
  memset(new_process->previous_exit_log, '\0',
         sizeof(new_process->previous_exit_log) /
             sizeof(**new_process->previous_exit_log));
  memset(new_process->previous_exit_reason, '\0',
         sizeof(new_process->previous_exit_reason) /
             sizeof(*new_process->previous_exit_reason));

  new_process->restarts_counter = 0;
  new_process->pid = 0;
  new_process->action = ACTION_NONE;
  SL_APPEND(get_data()->processes_head, new_process);

  pthread_t t;
  if (pthread_create(&t, NULL, &process_worker, new_process) != 0)
    e("pthread_create():%s", errno);
  pthread_detach(t);

  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_toggle(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();
  char pattern[128];
  if (mg_get_query_string_var(&hm->query_string, "pattern", pattern,
                              sizeof(pattern)) > 0) {
    struct Process *process;
    SL_SEARCH(data->processes_head, cmp_process_by_pattern, pattern, process);
    if (process) {
      if (process->pid > 0) {
        process->action = ACTION_PAUSE;
        kill(process->pid, SIGKILL);
      } else {
        process->action = ACTION_NONE;
        pthread_t t;
        if (pthread_create(&t, NULL, &process_worker, process) != 0)
          e("resume");
        pthread_detach(t);
      }
    }
  }
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_message(struct mg_connection *nc, struct http_message *hm) {
  struct Data *data = get_data();
  pthread_mutex_lock(&lock);
  char buf[1024];

  if (mg_get_http_var(&hm->body, "del", buf, sizeof(buf)) > 0) {
    struct Msg *msg;
    SL_FOREACH_SAFE(data->msgs_head, msg) {
      if (match(buf, msg->msg, 0, 0)) {
        SL_DELETE(data->msgs_head, msg);
        free(msg->msg);
        free(msg);
      }
    }
  }

  if (mg_get_http_var(&hm->body, "add", buf, sizeof(buf)) > 0) {
    struct Msg *msg = malloc(sizeof(struct Process));
    msg->msg = malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(msg->msg, buf);
    SL_APPEND(data->msgs_head, msg);
  }

  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_out(struct mg_connection *nc, struct http_message *hm) {
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain; charset=UTF-8\r\n"
            "Transfer-Encoding: chunked\r\n\r\n");

  struct Data *data = get_data();
  char buf_id[2];
  if (mg_get_query_string_var(&hm->query_string, "id", buf_id, sizeof(buf_id)) >
      0) {
    unsigned int request_id = (unsigned int)atoi(buf_id);
    struct Process *current_process = data->processes_head;
    while (current_process != NULL) {
      if (current_process->id == request_id) {
        const unsigned long siz =
            sizeof(current_process->previous_exit_log) /
                sizeof(**current_process->previous_exit_log) +
            sizeof(current_process->previous_exit_reason) /
                sizeof(*current_process->previous_exit_reason) +
            strlen(current_process->pwd) + strlen(current_process->env) +
            strlen(current_process->cmd) +
            sizeof(current_process->circular_buffer) /
                sizeof(*current_process->circular_buffer) +
            512;
        char buf[siz];
        memset(buf, 0, sizeof(buf) / sizeof(*buf));
        sprintf(buf, ">>> cd '%s'&&%s %s\n", current_process->pwd,
                current_process->env, current_process->cmd);
        sprintf(buf + strlen(buf), ">>> fails: %d reason: %s%s",
                current_process->restarts_counter,
                current_process->restarts_counter
                    ? current_process->previous_exit_reason
                    : "-",
                current_process->restarts_counter ? " stdout/stderr:\n" : "");
        const int lines = sizeof(current_process->previous_exit_log) /
                          sizeof(**current_process->previous_exit_log) /
                          sizeof(*current_process->previous_exit_log);
        for (int i = 0; i < lines; ++i) {
          const char *line = current_process->previous_exit_log[i];
          if (strlen(line)) {
            strcat(buf, line);
            strcat(buf, "\n");
          }
        }

        strcat(buf, "\n>>> current stdout/stderr:\n");
        unsigned long length = strlen(buf);
        for (unsigned long i = current_process->circular_buffer_pos; i < siz;
             ++i, ++length) {
          char c = current_process->circular_buffer[i];
          if (c == '\0') break;
          buf[length] = c;
        }
        for (unsigned long i = 0; i < current_process->circular_buffer_pos;
             ++i, ++length) {
          char c = current_process->circular_buffer[i];
          buf[length] = c;
        }
        buf[length] = '\0';
        mg_printf_http_chunk(nc, buf);
        break;
      }
      current_process = current_process->next;
    }
  }
  mg_send_http_chunk(nc, "", 0);
}

void handle_setup(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();
  mg_get_http_var(&hm->body, "env", data->env,
                  sizeof data->env / sizeof *data->env);
  mg_get_http_var(&hm->body, "pwd", data->pwd,
                  sizeof data->env / sizeof *data->pwd);
  pthread_mutex_unlock(&lock);
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

void handle_reset(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();

  // Очистка всех df.
  while (data->disks_head != NULL) {
    struct Disk *delete_disk = data->disks_head;
    // o("reset df %s", delete_disk->path);
    data->disks_head = data->disks_head->next;
    free(delete_disk->path);
    free(delete_disk);
  }

  // Отстрел всех процессов.
  struct Process *process;
  SL_FOREACH(data->processes_head, process) {
    if (process->pid > 0) {
      process->action = ACTION_KILL;
      kill(process->pid, SIGKILL);
    }
  }

  // Очищаем сообщения.
  struct Msg *prev_msg = NULL;
  struct Msg *current_msg = data->msgs_head;
  while (current_msg != NULL) {
    o("reset message '%s'", current_msg->msg);
    if (data->msgs_head == current_msg) {
      data->msgs_head = current_msg->next;
      current_msg->next = NULL;
      free(current_msg->msg);
      free(current_msg);
      current_msg = data->msgs_head;
    } else if (prev_msg != NULL) {
      prev_msg->next = current_msg->next;
      current_msg->next = NULL;
      free(current_msg->msg);
      free(current_msg);
      current_msg = prev_msg->next;
    }
  }
  pthread_mutex_unlock(&lock);
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

static const char *s_http_port = "14157";

static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *)ev_data;
  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      // Логгируем http запрос - пока так сложно...
      char buf[1024];
      size_t siz = sizeof(buf) / sizeof(*buf);
      size_t i = 0;

      for (size_t j = 0; j < hm->uri.len && j < siz; ++j, ++i) {
        buf[i] = hm->uri.p[j];
      }

      if (hm->query_string.len) {
        buf[i++] = '?';
        for (size_t j = 0; j < hm->query_string.len && j < siz; ++j, ++i) {
          buf[i] = hm->query_string.p[j];
        }
      }
      if (hm->body.len) {
        buf[i++] = ' ';
        for (size_t j = 0; j < hm->body.len && j < siz; ++j, ++i) {
          buf[i] = hm->body.p[j];
        }
      }
      buf[i] = 0;
      o("http %s", buf);

      if (mg_vcmp(&hm->uri, "/status") == 0)
        handle_status(nc);
      else if (mg_vcmp(&hm->uri, "/watch") == 0)
        handle_watch(nc, hm);
      else if (mg_vcmp(&hm->uri, "/toggle") == 0)
        handle_toggle(nc, hm);
      else if (mg_vcmp(&hm->uri, "/message") == 0)
        handle_message(nc, hm);
      else if (mg_vcmp(&hm->uri, "/out") == 0)
        handle_out(nc, hm);
      else if (mg_vcmp(&hm->uri, "/setup") == 0)
        handle_setup(nc, hm);
      else if (mg_vcmp(&hm->uri, "/reset") == 0)
        handle_reset(nc, hm);
      else
        mg_serve_http(nc, hm, s_http_server_opts);
      break;
    }
    default:
      break;
  }
}

void *worker() {
  while (1) {
    pthread_mutex_lock(&lock);
    update_data(get_data());
    gen_json(get_data());
    pthread_mutex_unlock(&lock);
    sleep(SLEEP);
  }
}

volatile sig_atomic_t stop = 0;
void interrupt_handler(int signum) { stop = 1; }

int main() {
  logger_init();
  m("Log format:\n");
  m("YYMMDDhhmmss quit N│reason│cmd\n");
  m("[space]most recent stdout/stderr of the quit process\n");
  m("version program " SOURCES_VERSION "\n");
  m("version mongoose " MG_VERSION "\n");

  if (pthread_mutex_init(&lock, NULL) != 0) {
    e("pthread_mutex_init():%s", strerror(errno));
    return 1;
  }

  struct Data *data = get_data();
  pthread_t tid[2];
  prepare_data(data);

  if (pthread_create(&(tid[0]), NULL, &worker, NULL) != 0) {
    e("pthread_create():%s", strerror(errno));
    return 1;
  }

  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (!nc) {
    e("mg_bind(port %s)", s_http_port);
    return 1;
  }
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";
  s_http_server_opts.enable_directory_listing = "yes";
  // o("start server at %s\n", s_http_port);
  signal(SIGINT, interrupt_handler);
  signal(SIGTERM, interrupt_handler);
  signal(SIGQUIT, interrupt_handler);
  signal(SIGPIPE, SIG_IGN);
  while (!stop) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);
  pthread_mutex_destroy(&lock);
  return 0;
}
