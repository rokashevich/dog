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
#include <unistd.h>

#include "data.h"
#include "defines.h"
#include "helpers.h"
#include "logger.h"
#include "mongoose.h"
#include "mongoose_helper.h"

pthread_mutex_t lock;

void gen_json(struct Data *data) {
  const int s = 4096;  // оперативный буфер, большой не нужен
  char b[s];
  char *p = data->json;
  data->json[0] = '\0';
  p = qstrcat(p, "{");
  p = qstrcat(p, "\"hostname\":\"");
  p = qstrcat(p, data->hostname);
  p = qstrcat(p, ",\"timestamp\":");
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
  struct Disk *current_disk = data->disks_head;
  while (current_disk != NULL) {
    p = qstrcat(p, "{");
    p = qstrcat(p, "\"path\":\"");
    p = qstrcat(p, current_disk->path);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"total\":");
    snprintf(b, s, "%lld,", current_disk->total);
    p = qstrcat(p, b);
    p = qstrcat(p, "\"used\":");
    snprintf(b, s, "%lld", current_disk->used);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (current_disk->next) p = qstrcat(p, ",");
    current_disk = current_disk->next;
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
  struct Process *current_process = data->processes_head;
  while (current_process != NULL) {
    p = qstrcat(p, "{");
    p = qstrcat(p, "\"id\":");
    snprintf(b, s, "%u", current_process->id);
    p = qstrcat(p, b);
    p = qstrcat(p, ",");
    p = qstrcat(p, "\"pwd\":\"");
    p = qstrcat(p, current_process->pwd);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"env\":\"");
    p = qstrcat(p, current_process->env);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"cmd\":\"");
    p = qstrcat(p, current_process->cmd);
    p = qstrcat(p, "\",");
    p = qstrcat(p, "\"pid\":");
    snprintf(b, s, "%u", current_process->pid);
    p = qstrcat(p, b);
    p = qstrcat(p, ",\"rss\":");
    snprintf(b, s, "%llu", current_process->rss);
    p = qstrcat(p, b);
    p = qstrcat(p, ",\"restarts_counter\":");
    snprintf(b, s, "%u", current_process->restarts_counter);
    p = qstrcat(p, b);
    p = qstrcat(p, "}");
    if (current_process->next) p = qstrcat(p, ",");
    current_process = current_process->next;
  }
  p = qstrcat(p, "],");
  p = qstrcat(p, "\"msgs\":[");
  struct Msg *current_msg = data->msgs_head;
  while (current_msg != NULL) {
    p = qstrcat(p, "\"");
    p = qstrcat(p, current_msg->msg);
    p = qstrcat(p, "\"");
    current_msg = current_msg->next;
    if (current_msg) p = qstrcat(p, ",");
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
  struct Process *process = (struct Process *)voidprocess;
  struct Data *data = get_data();
  int filedes[2];
  if (pipe(filedes) == -1) {
    e("pipe():%s", strerror(errno));
    return NULL;
  }
  process->pid = fork();
  while (1) {
    if (process->pid == -1) {
      e("fork():%s", strerror(errno));
    } else if (process->pid == 0) {  // Внутри child-а.
      // Убиваем child, если parent завершился.
      prctl(PR_SET_PDEATHSIG, SIGKILL);
      while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        ;
      while ((dup2(filedes[1], STDERR_FILENO) == -1) && (errno == EINTR))
        ;
      close(filedes[0]);
      close(filedes[1]);

      fprintf(stdout, "exec #%d cd '%s'&&%s %s\n", process->restarts_counter,
              process->pwd, process->env, process->cmd);

      if (strlen(process->pwd)) {
        if (chdir(process->pwd) != 0) {
          fprintf(stdout, "chdir(%s):%s\n", process->pwd, strerror(errno));
          return NULL;
        }
      }
      execvpe(process->cmds[0], process->cmds, process->envs);
      fprintf(stdout, "execvpe():%s", strerror(errno));
      exit(1);
    } else {  // parent
      o("exec #%d cd '%s'&&%s %s", process->restarts_counter, process->pwd,
        process->env, process->cmd);
      close(filedes[1]);

      ssize_t nread;
      char buffer[1024];
      while ((nread = read(filedes[0], &buffer[0], sizeof(buffer))) > 0) {
        unsigned long buffer_right_side_size =
            sizeof(process->circular_buffer) - process->circular_buffer_pos;
        if (buffer_right_side_size >= (unsigned long)nread) {
          // Полученный stdout/stderr влезает в правую часть буфера целиком.
          memcpy(process->circular_buffer + process->circular_buffer_pos,
                 buffer, (unsigned long)nread);
          process->circular_buffer_pos += (unsigned long)nread;
        } else {  // Полученный stdout/stderr надо разбивать на две части.
          memcpy(process->circular_buffer + process->circular_buffer_pos,
                 buffer, buffer_right_side_size);
          process->circular_buffer_pos =
              (unsigned long)nread - buffer_right_side_size;
          memcpy(process->circular_buffer, buffer + buffer_right_side_size,
                 process->circular_buffer_pos);
        }
      }
      int status;

      while (waitpid(process->pid, &status, 0) == -1) {
        if (errno != EINTR) {
          fprintf(stdout, "waitpid():%s", strerror(errno));
        }
      }
      if (WIFEXITED(status)) {
        fprintf(stdout, "Child exited with status %d\n", WEXITSTATUS(status));
      } else if (WIFSTOPPED(status)) {
        fprintf(stdout, "Child stopped by signal %d (%s)\n", WSTOPSIG(status),
                strsignal(WSTOPSIG(status)));
      } else if (WIFSIGNALED(status)) {
        fprintf(stdout, "Child killed by signal %d (%s)\n", WTERMSIG(status),
                strsignal(WTERMSIG(status)));
      } else {
        fprintf(stdout, "Unknown child status\n");
      }
      close(filedes[0]);

      if (process->action == ACTION_KILL) {
        pthread_mutex_lock(&lock);
        struct Process *current_process = data->processes_head;
        struct Process *prev_process = NULL;
        struct Process *free_process = NULL;
        while (current_process != NULL) {
          if (process == current_process) {
            free_process = current_process;
            if (prev_process == NULL) {
              current_process = data->processes_head = current_process->next;
            } else {
              prev_process->next = current_process->next;
              current_process = prev_process->next;
            }
            free(free_process->pwd);
            free(free_process->env);
            free(free_process->cmd);
            free_string_array(free_process->envs);
            free_string_array(free_process->cmds);
            free(free_process);
          } else {
            prev_process = current_process;
            current_process = current_process->next;
          }
        }
        pthread_mutex_unlock(&lock);
        break;
      } else if (process->action == ACTION_PAUSE) {
        process->pid = 0;
        process->restarts_counter = 0;
        break;
      } else {
        sleep(1);
        process->restarts_counter++;
        if (pipe(filedes) == -1) {
          e("pipe():%s", strerror(errno));
          continue;
        }
        process->pid = fork();
      }
    }
  }
  return voidprocess;
}

void handle_watch(struct mg_connection *nc, struct http_message *hm) {
  pthread_mutex_lock(&lock);
  struct Data *data = get_data();
  struct Process *new_process;
  if (data->processes_head == NULL) {
    data->processes_head = malloc(sizeof(struct Process));
    data->processes_head->next = NULL;
    new_process = data->processes_head;
  } else {
    new_process = data->processes_head;
    while (new_process->next != NULL) {
      new_process = new_process->next;
    }
    new_process->next = malloc(sizeof(struct Process));
    new_process = new_process->next;
    new_process->next = NULL;
  }

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
  int siz;

  mg_get_http_var(&hm->body, "pwd", buf, sizeof(buf));
  siz = strlen(buf) + 1;
  new_process->pwd = malloc(siz * sizeof(char));
  strncpy(new_process->pwd, buf, siz);

  mg_get_http_var(&hm->body, "env", buf, sizeof(buf));
  siz = strlen(buf) + 1;
  new_process->env = malloc(siz * sizeof(char));
  strncpy(new_process->env, buf, siz);

  mg_get_http_var(&hm->body, "cmd", buf, sizeof(buf));
  siz = strlen(buf) + 1;
  new_process->cmd = malloc(siz * sizeof(char));
  strncpy(new_process->cmd, buf, siz);

  new_process->circular_buffer_pos = 0;
  memset(new_process->circular_buffer, '\0', BUFFER_OUT_SIZE);

  // ctrl+shitf+u,2502
  o("request watch pwd,env,cmd=│%s│%s│%s│", new_process->pwd, new_process->env,
    new_process->cmd);

  new_process->envs = string_to_string_array(new_process->env);
  new_process->cmds = string_to_string_array(new_process->cmd);

  new_process->restarts_counter = 0;
  new_process->pid = 0;

  pthread_mutex_unlock(&lock);

  // Запускаем поток отслеживания запущенного запрошенного процесса.
  pthread_t tid[2];
  if (pthread_create(&(tid[0]), NULL, &process_worker, new_process) != 0)
    e("pthread_create:%s", errno);

  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

void handle_pause(struct mg_connection *nc, struct http_message *hm) {
  struct Data *data = get_data();
  pthread_mutex_lock(&lock);
  char buf[1024];
  if (mg_get_http_var(&hm->body, "pattern", buf, sizeof(buf))) {
    struct Process *current_process = data->processes_head;
    while (current_process != NULL) {
      if (match(buf, current_process->cmd, 0, 0) && current_process->pid > 0) {
        current_process->action = ACTION_PAUSE;
        kill(current_process->pid, SIGKILL);
        break;
      }
      current_process = current_process->next;
    }
  }
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_resume(struct mg_connection *nc, struct http_message *hm) {
  struct Data *data = get_data();
  char buf[1024];
  mg_get_http_var(&hm->body, "pattern", buf, sizeof(buf));
  struct Process *current_process = data->processes_head;
  while (current_process != NULL) {
    if (match(buf, current_process->cmd, 0, 0) && current_process->pid == 0) {
      current_process->action = ACTION_NONE;
      // Запускаем поток отслеживания дога.
      pthread_t tid[2];
      if (pthread_create(&(tid[0]), NULL, &process_worker, current_process) !=
          0) {
        printf("!!! Warning: pthread_create: process_worker\n");
      }
      break;
    }
    current_process = current_process->next;
  }
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

void handle_undf(struct mg_connection *nc) {
  pthread_mutex_lock(&lock);
  //  struct Process *current_process = processes_head;
  //  while (current_process != NULL) {
  //    current_process->action = ACTION_KILL;
  //    if (current_process->pid > 0) {
  //      kill(current_process->pid, SIGKILL);
  //    }
  //    current_process = current_process->next;
  //  }
  //  struct Msg *current_msg = msgs_head;
  //  while (current_msg != NULL) {
  //    current_msg->action = ACTION_FREE;
  //    current_msg = current_msg->next;
  //  }

  struct Data *data = get_data();
  while (data->disks_head != NULL) {
    struct Disk *delete_disk = data->disks_head;
    data->disks_head = data->disks_head->next;
    free(delete_disk->path);
    free(delete_disk);
  }

  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_killall(struct mg_connection *nc) {
  struct Data *data = get_data();
  pthread_mutex_lock(&lock);
  struct Process *current_process = data->processes_head;
  struct Process *prev_process = NULL;
  struct Process *free_process = NULL;
  while (current_process != NULL) {
    if (current_process->pid > 0) {
      current_process->action = ACTION_KILL;
      kill(current_process->pid, SIGKILL);
      current_process = current_process->next;
    } else {  // процесс на пузе
      free_process = current_process;
      if (prev_process == NULL) {
        current_process = data->processes_head = current_process->next;
      } else {
        prev_process->next = current_process->next;
        current_process = prev_process->next;
      }
      free(free_process->pwd);
      free(free_process->env);
      free(free_process->cmd);
      free_string_array(free_process->envs);
      free_string_array(free_process->cmds);
      free(free_process);
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
    struct Msg *prev_msg = NULL;
    struct Msg *current_msg = data->msgs_head;
    while (current_msg != NULL) {
      if (match(buf, current_msg->msg, 0, 0)) {
        if (data->msgs_head ==
            current_msg) {  // If node to be deleted is head node.
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
      } else {
        prev_msg = current_msg;
        current_msg = current_msg->next;
      }
    }
  }
  if (mg_get_http_var(&hm->body, "add", buf, sizeof(buf)) > 0) {
    struct Msg *new_msg;
    if (data->msgs_head == NULL) {
      data->msgs_head = malloc(sizeof(struct Process));
      data->msgs_head->next = NULL;
      new_msg = data->msgs_head;
    } else {
      new_msg = data->msgs_head;
      while (new_msg->next != NULL) {
        new_msg = new_msg->next;
      }
      new_msg->next = malloc(sizeof(struct Process));
      new_msg->next->next = NULL;
      new_msg = new_msg->next;
    }
    new_msg->msg = malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(new_msg->msg, buf);
  }
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
  pthread_mutex_unlock(&lock);
}

void handle_out(struct mg_connection *nc, struct http_message *hm) {
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");

  struct Data *data = get_data();
  char buf[BUFFER_OUT_SIZE];
  unsigned long length = 0;
  if (mg_get_query_string_var(&hm->query_string, "id", buf, sizeof(buf)) > 0) {
    unsigned int request_id = (unsigned int)atoi(buf);
    struct Process *current_process = data->processes_head;
    while (current_process != NULL) {
      if (current_process->id == request_id) {
        o("request out for cmd=%s", current_process->cmd);
        for (unsigned long i = current_process->circular_buffer_pos;
             i < BUFFER_OUT_SIZE; ++i, ++length) {
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
        break;
      }
      current_process = current_process->next;
    }
  }
  mg_printf_http_chunk(nc, buf);
  mg_send_http_chunk(nc, "", 0);
}

void handle_df(struct mg_connection *nc, struct http_message *hm) {
  struct Data *data = get_data();
  char buf[BUFFER_SIZE_DEFAULT];
  if (mg_get_http_var(&hm->body, "path", buf, sizeof(buf)) > 0) {
    printf("handle_df: path=%s\n", buf);
    struct Disk *new_disk;
    if (data->disks_head == NULL) {
      data->disks_head = malloc(sizeof(struct Disk));
      data->disks_head->next = NULL;
      new_disk = data->disks_head;
    } else {
      new_disk = data->disks_head;
      while (new_disk->next != NULL) {
        new_disk = new_disk->next;
      }
      new_disk->next = malloc(sizeof(struct Disk));
      new_disk->next->next = NULL;
      new_disk = new_disk->next;
    }
    new_disk->path = malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(new_disk->path, buf);
    new_disk->total = 0;
    new_disk->used = 0;
  }
  mg_printf(nc, "%s",
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: "
            "*\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_send_http_chunk(nc, "", 0);
}

void handle_setup(struct mg_connection *nc, struct http_message *hm) {
  char buf[BUFFER_SIZE_DEFAULT];
  if (mg_get_query_string_var(&hm->query_string, "debug", buf, sizeof(buf)) >
      0) {
  }

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
    case MG_EV_HTTP_REQUEST:
      if (mg_vcmp(&hm->uri, "/status") == 0)
        handle_status(nc);
      else if (mg_vcmp(&hm->uri, "/watch") == 0)
        handle_watch(nc, hm);
      else if (mg_vcmp(&hm->uri, "/pause") == 0)
        handle_pause(nc, hm);
      else if (mg_vcmp(&hm->uri, "/resume") == 0)
        handle_resume(nc, hm);
      else if (mg_vcmp(&hm->uri, "/killall") == 0)
        handle_killall(nc);
      else if (mg_vcmp(&hm->uri, "/message") == 0)
        handle_message(nc, hm);
      else if (mg_vcmp(&hm->uri, "/out") == 0)
        handle_out(nc, hm);
      else if (mg_vcmp(&hm->uri, "/df") == 0)
        handle_df(nc, hm);
      else if (mg_vcmp(&hm->uri, "/undf") == 0)
        handle_undf(nc);
      else if (mg_vcmp(&hm->uri, "/setup") == 0)
        handle_setup(nc, hm);
      else
        mg_serve_http(nc, hm, s_http_server_opts);
      break;
    default:
      break;
  }
}

void *worker() {
  struct Data *data = get_data();
  while (1) {
    pthread_mutex_lock(&lock);
    update_data(data);
    gen_json(data);
    pthread_mutex_unlock(&lock);
    sleep(SLEEP);
  }
}

volatile sig_atomic_t stop = 0;
void interrupt_handler(int signum) { stop = 1; }

int main() {
  o("version dog " SOURCES_VERSION);
  setlinebuf(stdout);

  if (pthread_mutex_init(&lock, NULL) != 0) {
    o("fail mutex init\n");
  }

  struct Data *data = get_data();
  pthread_t tid[2];
  prepare_data(data);

  if (pthread_create(&(tid[0]), NULL, &worker, NULL) != 0) {
    o("fail thread worker\n");
  }

  struct mg_mgr mgr;
  struct mg_connection *nc;

  o("version mongoose " MG_VERSION);
  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (!nc) {
    o("fail bind %s\n", s_http_port);
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
