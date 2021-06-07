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
    p = qstrcat(p, "\"name\":\"");
    p = qstrcat(p, disk->name);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"type\":\"");
    p = qstrcat(p, disk->type);
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
    p = qstrcat(p, ",\"restarts\":");
    snprintf(b, s, "%u", process->restarts_counter);
    p = qstrcat(p, b);
    p = qstrcat(p, ",\"log\":\"");
    p = qstrcat(p, process->previous_exit_log);
    p = qstrcat(p, "\"");

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
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n"
            "Transfer-Encoding: chunked\r\n\r\n");
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
  // pid_t pid = fork();
  process->pid = fork();

  while (1) {
    if (process->pid == 0) {  // CHILD
      prctl(PR_SET_PDEATHSIG, SIGKILL);

      close(fd[PIPE_READ]);
      while ((dup2(fd[PIPE_WRITE], STDOUT_FILENO) == -1) && (errno == EINTR))
        ;
      while ((dup2(fd[PIPE_WRITE], STDERR_FILENO) == -1) && (errno == EINTR))
        ;
      close(fd[PIPE_WRITE]);

      const char *pwd = strlen(process->pwd)
                            ? process->pwd
                            : (strlen(data->pwd) ? data->pwd : NULL);
      if (pwd && chdir(pwd)) {
        fprintf(stderr, "bad cfg pwd %s, %s'\n", process->pwd, strerror(errno));
        exit(1);
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
      exit(1);
    }
    close(fd[PIPE_WRITE]);
    // PARENT
    // pthread_mutex_lock(&lock);
    // process->pid = pid;
    // pthread_mutex_unlock(&lock);

    // В бесконечном цикле читаем неблокирующим способом пайп чилда.
    int status = -1;
    ssize_t nread;
    char buffer[4096];
    while (1) {
      int flags = fcntl(fd[PIPE_READ], F_GETFL, 0);
      fcntl(fd[PIPE_READ], F_SETFL, flags | O_NONBLOCK);
      nread = read(fd[PIPE_READ], &buffer[0], sizeof(buffer));
      if (nread <= 0) {
        if (waitpid(process->pid, &status, WNOHANG) > 0) {
          break;  // Процесс завершился.
        } else {  // Процесс еще работает.
          sleep(1);
          continue;
        }
      } else {
        // Обновляем циклически буфер процесса в общей стркутре.
        pthread_mutex_lock(&lock);
        // e(">>>%d\n", process->circular_buffer_pos);
        cirbuf_push(buffer, nread, process->circular_buffer,
                    &process->circular_buffer_pos);
        // char b[cirbuf_size];
        // cirbuf_takeout(process->circular_buffer,
        // process->circular_buffer_pos,
        //                b);
        // e(">%s\n", b);
        pthread_mutex_unlock(&lock);
      }
    }
    close(fd[PIPE_READ]);

    // Обрабатываем завершение процесса.
    pthread_mutex_lock(&lock);
    const int siz = 512;
    char reason[siz];
    if (WIFEXITED(status)) {
      snprintf(reason, siz, "Exit code %d", WEXITSTATUS(status));
    } else if (WIFSTOPPED(status)) {
      snprintf(reason, siz, "Child stopped by signal %d (%s)", WSTOPSIG(status),
               strsignal(WSTOPSIG(status)));
    } else if (WIFSIGNALED(status)) {
      snprintf(reason, siz, "Child killed by signal %d (%s)", WTERMSIG(status),
               strsignal(WTERMSIG(status)));
    } else
      snprintf(reason, siz, "Exit reason unknown!");

    // Решаем, что делать с процессом дальше.
    if (process->action == ACTION_KILL) {
      SL_DELETE(data->processes_head, process);
      free(process);
      pthread_mutex_unlock(&lock);
      break;
    } else if (process->action == ACTION_PAUSE) {
      process->pid = 0;
      process->restarts_counter = 0;
      pthread_mutex_unlock(&lock);
      break;
    }

    // Процесс завершился самопроизвольно - перезапускаем!
    process->restarts_counter++;
    strcpy(process->previous_exit_log, reason);
    cirbuf_takeout(process->circular_buffer, process->circular_buffer_pos,
                   process->previous_exit_log + strlen(reason));
    const buf_max_len = sizeof(process->previous_exit_log) /
                        sizeof(*process->previous_exit_log);
    json_safe(strip_ansi_escape_codes(process->previous_exit_log), buf_max_len);
    // for (int i = 0; i < sizeof(process->circular_buffer) /
    //                         sizeof(*process->circular_buffer);
    //      ++i) {
    //   printf("[%c]", process->circular_buffer[i]);
    // }
    printf("\n");
    w("die %s %d %s", process->cmd, process->restarts_counter,
      process->previous_exit_log);
    // const size_t n = strlen(process->previous_exit_log);
    // char s[n];
    // strcpy(s, process->previous_exit_log);
    // char *p = s;
    // p = strtok(s, "\n");
    // while (p != NULL) {
    //   const size_t n = strlen(p);
    //   char s[n];
    //   strcpy(s, p);
    //   p = strtok(NULL, "\n");
    //   w("die %s %d %s", process->cmd, process->restarts_counter,
    //     strip_ansi_escape_codes(s));
    // }
    cirbuf_fill(process->circular_buffer, &process->circular_buffer_pos, ' ');

    if (pipe(fd) == -1) {
      e("pipe():%s", strerror(errno));
      return NULL;
    }
    pthread_mutex_unlock(&lock);
    sleep(SLEEP);
    process->pid = fork();
  }
  return voidprocess;
}

void handle_watch(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Process *process = malloc(sizeof(struct Process));

  // Счётчик последнего выданного id.
  static unsigned int largest_id = 0;
  process->id = ++largest_id;

  // Читаем HTTP POST запрос вида:
  // curl -X POST http://localhost:14157/watch -d "
  // pwd=/opt
  // &env=LD_LIBRARY_PATH=.:../lib DISPLAY=$DISPLAY
  // &cmd=/usr/bin/app -a -r -g -u -m -e -n -t -s"
  // и заполняем соотвествующие поля в структуре Process.

  char buf[1024];

  mg_get_http_var(&hm->body, "pwd", buf, sizeof(buf));
  strncpy(process->pwd, buf, sizeof process->pwd / sizeof *process->pwd);

  mg_get_http_var(&hm->body, "env", buf, sizeof(buf));
  strncpy(process->env, buf, sizeof process->env / sizeof *process->env);

  mg_get_http_var(&hm->body, "cmd", buf, sizeof(buf));
  strncpy(process->cmd, buf, sizeof process->cmd / sizeof *process->cmd);

  cirbuf_fill(process->circular_buffer, &process->circular_buffer_pos, ' ');
  memset(process->previous_exit_log, '\0', sizeof(process->previous_exit_log));

  process->restarts_counter = 0;
  process->pid = 0;
  process->action = ACTION_NONE;
  SL_APPEND(get_data()->processes_head, process);

  pthread_t t;
  if (pthread_create(&t, NULL, &process_worker, process) != 0)
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
  char cmd[128];
  mg_get_http_var(&hm->body, "cmd", cmd, sizeof(cmd));
  if (strlen(cmd) > 0) {
    struct Process *process;
    SL_FOREACH(data->processes_head, process) {
      if (match(cmd, process->cmd, 0, 0)) {
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

void handle_setup(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();
  mg_get_http_var(&hm->body, "env", data->env,
                  sizeof data->env / sizeof *data->env);
  mg_get_http_var(&hm->body, "pwd", data->pwd,
                  sizeof data->env / sizeof *data->pwd);
  o("setup:\ncurl -X POST 127.0.0.1:14157/setup -d \"env=%s&pwd=%s\"\n",
    data->env, data->pwd);
  pthread_mutex_unlock(&lock);
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

void handle_reset(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();

  // Отстрел всех процессов.
  struct Process *process;
  SL_FOREACH_SAFE(data->processes_head, process) {
    if (process->pid > 0) {
      process->action = ACTION_KILL;
      kill(process->pid, SIGKILL);
    } else {
      SL_DELETE(data->processes_head, process);
      free(process);
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

      if (mg_vcmp(&hm->uri, "/status") == 0)
        handle_status(nc);
      else if (mg_vcmp(&hm->uri, "/watch") == 0)
        handle_watch(nc, hm);
      else if (mg_vcmp(&hm->uri, "/toggle") == 0)
        handle_toggle(nc, hm);
      else if (mg_vcmp(&hm->uri, "/message") == 0)
        handle_message(nc, hm);
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
  o("version program " SOURCES_VERSION);
  o("version mongoose " MG_VERSION);

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
  o("port=%s", s_http_port);
  if (!nc) {
    e("mg_bind(port %s)", s_http_port);
    return 1;
  }
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";
  s_http_server_opts.enable_directory_listing = "yes";
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
