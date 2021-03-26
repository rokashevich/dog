#ifndef DATA_H
#define DATA_H
#ifndef __USE_GNU
#define __USE_GNU
#endif
// Структура для хранения сырых данных мониторинга:
// worker в неё пишет, handle_status из неё читатет

#include <limits.h>
#include <time.h>

#include "defines.h"

enum Action {
  ACTION_NONE,
  ACTION_STOP,
  ACTION_START,
  ACTION_KILL,
  ACTION_FREE,
  ACTION_PAUSE,
  ACTION_RESUME
};

struct Process {
  // Для хранения параметров запуска в переданном через POST запрос виде.
  unsigned int id;  // Уникальный идентификатор (не pid!).
  char pwd[1024];  // /opt/sandbox/bin
  char env[1024];  // DISPLAY=$DISPLAY LD_LIBRARY_PATH=.:../lib
  char cmd[1024];  // program --arg-one 1 --arg-two 2

  char circular_buffer[4096];  // Для stdout + stderr.
  unsigned long circular_buffer_pos;
  char previous_exit_reason[512];
  char previous_exit_log[10][200];  // Для последних строк перед падением.

  pid_t pid;
  unsigned int restarts_counter;
  struct Process *next;

  int action;
  unsigned long long rss;
};

struct Disk {
  char *path;
  char *name;
  char *type;
  unsigned long long total;
  unsigned long long used;
  struct Disk *next;
};

struct Data {
  struct Process *processes_head;
  struct Msg *msgs_head;

  char env[4096];  // должно хватить :)
  char pwd[PATH_MAX];
  char version[32];
  char hostname[64];
  char boot_id[37];    // /proc/sys/kernel/random/boot_id >
                       // 7ef3d79c-15e2-42fe-81b2-947c123fbf4b
  char timestamp[15];  // YYYYMMDDhhmmss\0
  struct timespec time;
  long uptime;
  float loadavg;

  struct Cpu {
    char sys_class_thermal_thermal_zoneCPU_temp[sizeof(
        "/sys/class/thermal/thermal_zoneCPU/temp")];
    int count;
    unsigned int temperature;
    unsigned int usage;
    unsigned int *cores_usage;
    int *stat_prev_idle;
    int *stat_prev_total;
  } cpu;

  struct Ram {
    unsigned long long total;
    unsigned int usage;
  } ram;

  struct Disk *disks_head;

  struct Net {
    char iface[16];  // Имя сетевого интерфейса, пример: "enp5s0",
    // 16 символов должно хватить.
    char link;             // 1 - кабель подключен, 0 - нет.
    unsigned int carrier;  // Поднят?
    unsigned int speed;    // Скорость подключения.
    char ip[16];  // Ip-адреc, 16 символов, пример: "192.168.100.101".
    char hw[18];  // Mac-адрес, 18 символов, пример: "78:92:9c:9c:4d:de".
    int mtu;  // MTU.
    unsigned long long rx;  // Получено байт с начала загрузки системы.
    unsigned long long tx;  // Отправлено байт.
    unsigned long long current_rx_speed;  // Текущая скорость rx байт/сек.
    unsigned long long current_tx_speed;  // Текущая скорость tx байт/сек.
    unsigned long long max_rx_speed;  // Максимальная скорость rx байт/сек.
    unsigned long long max_tx_speed;  // Максимальная скорость tx байт/сек.
  } * net;
  unsigned int net_count;  // Количество сетевых интерфейсов.

  char json[16384];
};

struct Data *get_data(void);

int get_hostname(struct Data *data);

void prepare_data(struct Data *data);  // Выполняется раз при запуске.

void update_data(struct Data *data);  // Выполняется с периодичностью.

void update_timestamp(char *timestamp);

static inline void get_current_rx_tx_for_iface(unsigned long long *rx,
                                               unsigned long long *tx,
                                               const char *iface);

static inline void get_rss_by_pid(unsigned long long *rss, const pid_t pid);

#endif  // DATA_H
