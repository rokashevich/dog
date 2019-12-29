#ifndef DATA_H
#define DATA_H
// Структура для хранения сырых данных мониторинга:
// worker в неё пишет, handle_status из неё читатет

#include <time.h>

struct Disk {
  char* path;
  unsigned long long total;
  unsigned long long used;
  struct Disk* next;
};

struct Data {
  char hostname[64];
  char boot_id[37]; // /proc/sys/kernel/random/boot_id > 7ef3d79c-15e2-42fe-81b2-947c123fbf4b
  struct timespec time;
  long uptime;

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
    char iface[16]; // Имя сетевого интерфейса, пример: "enp5s0",
    // 16 символов должно хватить.
    char link;      // 1 - кабель подключен, 0 - нет.
    unsigned int carrier;        // Поднят?
    unsigned int speed;      // Скорость подключения.
    char ip[16];    // Ip-адреc, 16 символов, пример: "192.168.100.101".
    char hw[18];    // Mac-адрес, 18 символов, пример: "78:92:9c:9c:4d:de".
    int mtu;        // MTU.
    unsigned long long rx; // Получено байт с начала загрузки системы.
    unsigned long long tx; // Отправлено байт.
    unsigned long long current_rx_speed; // Текущая скорость rx байт/сек.
    unsigned long long current_tx_speed; // Текущая скорость tx байт/сек.
    unsigned long long max_rx_speed; // Максимальная скорость rx байт/сек.
    unsigned long long max_tx_speed; // Максимальная скорость tx байт/сек.
  } *net;
  unsigned int net_count; // Количество сетевых интерфейсов.

  char json[8192];
};

struct Data* get_data(void);

int get_hostname(struct Data *data);

void prepare_data(struct Data *data); // Выполняется раз при запуске программы.

void update_data(struct Data *data); // Выполняется с периодичностью.

#endif // DATA_H