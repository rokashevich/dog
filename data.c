#include <arpa/inet.h>
#include <dirent.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <string.h>
#include <stdlib.h>

#include "data.h"

const char *thermal_zone_type[] = {
  "x86_pkg_temp",    // x64
  "imx_thermal_zone", // imx.6
  NULL
};

static struct Data data;

struct Data* get_data(void)
{
  return &data;
}

void prepare_data(struct Data *data) {
  char buf[64];
  unsigned int i;
  FILE *f;

  clock_gettime(CLOCK_REALTIME, &data->time);

  // Узнаём hostname.
  f = fopen("/proc/sys/kernel/hostname", "r");
  if (!f || !fgets(data->hostname, sizeof(data->hostname), f)) {
    printf("fail read /proc/sys/kernel/hostname\n");
  }
  fclose(f);
  data->hostname[strlen(data->hostname)-1] = 0;

  // Узнаём boot_id:
  f = fopen("/proc/sys/kernel/random/boot_id", "r");
  if (!f || !fgets(data->boot_id, sizeof(data->boot_id), f)) {
    printf("fail read /proc/sys/kernel/random/boot_id\n");
  }
  fclose(f);
  data->boot_id[strlen(data->boot_id)-1] = 0;

  // Узнаём количество ядер процессоров.
  data->cpu.count = 0;
  f = fopen("/proc/cpuinfo","r");
  if (!f) {
    printf("fail open /proc/cpuinfo\n");
  }
  while(fgets(buf, sizeof(buf), f) != NULL){
    if (strncmp(buf,"processor",sizeof("processor")-1)==0){
      data->cpu.count += 1;
    }
  }
  fclose(f);
  data->cpu.cores_usage = calloc(data->cpu.count, sizeof(data->cpu.cores_usage));
  data->cpu.stat_prev_idle = calloc(data->cpu.count, sizeof(data->cpu.stat_prev_idle));
  data->cpu.stat_prev_total = calloc(data->cpu.count, sizeof(data->cpu.stat_prev_total));

  // Ищем thermal_zone, соответствующий CPU.
  DIR *dp;
  struct dirent *ep;
  dp = opendir("/sys/class/thermal");
  if (!dp) {
    printf("fail look /sys/class/thermal\n");
  } else {
    while ((ep = readdir(dp))) {
      if (strncmp("thermal_zone", ep->d_name, sizeof("thermal_zone")-1) == 0) {
        // Нашли все thermal_zone, проверяем thermal_zone_type.
        snprintf(buf, sizeof(buf), "/sys/class/thermal/%s/type", ep->d_name);
        FILE *f = fopen(buf, "r");
        if (f == NULL) {
          printf("fail open %s\n", buf);
        }
        if (fgets(buf, sizeof(buf), f) == NULL) { // в t записан type
          printf("fail read %s\n", buf);
        }
        fclose(f);

        // Сравниваем thermal_zone/type со списком (const char *thermal_zone_type[])
        // и если совпадает - значит нашли отвечающий за температуру процессора.
        i = 0;
        while(thermal_zone_type[i]) {
          if (strncmp(buf, thermal_zone_type[i], sizeof(thermal_zone_type[i])-1) == 0) { // type соотвествует CPU
            snprintf(data->cpu.sys_class_thermal_thermal_zoneCPU_temp,
                     sizeof(data->cpu.sys_class_thermal_thermal_zoneCPU_temp),
                     "/sys/class/thermal/%s/temp",ep->d_name);
            break;
          }
          i++;
        };
      }
    }
    (void)closedir(dp);
  }

  data->disks_head = NULL;

  // Ищем имена присутствующих сетевых интерфейсов, за исключение lo.
  // https://stackoverflow.com/questions/19227781/linux-getting-all-network-interface-names
  struct if_nameindex *if_nidxs, *intf;
  if_nidxs = if_nameindex();
  if (if_nidxs != NULL) {
    // Считаем количество интерфейсов и выделяем под них память.
    i = 0;
    for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
      if (strcmp(intf->if_name, "lo")) {
        i++;
      }
    }
    data->net_count = i;
    data->net = calloc(data->net_count, sizeof(*data->net));
    // Теперь заново проходим по интерфейсам и сохраняем имя и мак-адрес.
    for (intf = if_nidxs, i = 0; intf->if_index != 0 || intf->if_name != NULL; intf++) {
      if (strcmp(intf->if_name, "lo")) {
        // Cохраняем имя.
        strncpy(data->net[i].iface, intf->if_name, sizeof(data->net[i].iface) - 1);
        // Сохраняем мак-адрес
        struct ifreq s;
        int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (fd < 0) {
          strcpy(data->net[i].hw, "");
        } else {
          strcpy(s.ifr_name, intf->if_name);
          if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
            sprintf(data->net[i].hw, "%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char) s.ifr_addr.sa_data[0],
                (unsigned char) s.ifr_addr.sa_data[1],
                (unsigned char) s.ifr_addr.sa_data[2],
                (unsigned char) s.ifr_addr.sa_data[3],
                (unsigned char) s.ifr_addr.sa_data[4],
                (unsigned char) s.ifr_addr.sa_data[5]);
          }
          close(fd);
        }

        // Обнуляем прочие начальные значения.
        data->net[i].rx = 0;
        data->net[i].tx = 0;
        data->net[i].current_rx_speed = 0;
        data->net[i].current_tx_speed = 0;
        data->net[i].max_rx_speed = 0;
        data->net[i].max_tx_speed = 0;

        i++; // Переходим к следующему интерфейсу.
      }
    }
  }
}


// https://stackoverflow.com/questions/4229415/c-sysinfo-returning-bad-values-i686
void update_data(struct Data *data) {
  char buf[100];
  FILE *f;

  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);
  long delta = (current_time.tv_nsec - data->time.tv_nsec)
      + (current_time.tv_sec  - data->time.tv_sec) * 1000000000L;
  data->time = current_time;

  //
  // Обновляем данные по температуре процессора.
  //
  char temp[8]; // температура представлена в виде числа, например, 99000
  f = fopen(data->cpu.sys_class_thermal_thermal_zoneCPU_temp, "r");
  if (f && fgets(temp, sizeof(temp), f) != NULL) {
    data->cpu.temperature = atoi(temp)/1000;
    fclose(f);
  }

  // Обновляем данные по загрузке ядер процессора.
  // https://github.com/Leo-G/DevopsWiki/wiki/How-Linux-CPU-Usage-Time-and-Percentage-is-calculated
  if ((f = fopen("/proc/stat","r")) == NULL) {
    printf("*** ERROR: fopen: /proc/stat\n");
    return;
  }

  char c;
  int line = -1;
  int cpu_part_passed;
  int t;
  int n[7]; // Для значений user nice system idle iowait irq softirq steal.
  while ((c = fgetc(f)) != EOF) {
    if (c == '\n') {
      if (line > -1) { // распарсили строку cpu под номером line
        // Формула:
        // int total_cpu_idle_time_since_boot = n[3] + n[4];
        // int total_cpu_usage_time_since_boot = n[0]+n[1]+n[2]+n[3]+n[4]+n[5]+n[6];
        // int diff_idle = total_cpu_idle_time_since_boot - data->cpu.stat_prev_idle[line];
        // int diff_total = total_cpu_usage_time_since_boot - data->cpu.stat_prev_total[line];
        // int diff_usage = 100*(diff_total - diff_idle);
        // int percentage = diff_usage/diff_total;

        data->cpu.cores_usage[line] =
            100*(n[0]+n[1]+n[2]+n[5]+n[6]
            - data->cpu.stat_prev_total[line]
            + data->cpu.stat_prev_idle[line])
            /
            (n[0]+n[1]+n[2]+n[3]+n[4]+n[5]+n[6] - data->cpu.stat_prev_total[line]);

        data->cpu.stat_prev_idle[line] = n[3] + n[4];
        data->cpu.stat_prev_total[line] = n[0]+n[1]+n[2]+n[3]+n[4]+n[5]+n[6];
      }
      cpu_part_passed = -1;
      n[0]=n[1]=n[2]=n[3]=n[4]=n[5]=n[6]=0;
      line += 1;
      t = 0;
      if (line == data->cpu.count)
        break;
    }
    if (line > -1) {
      if (c == ' ') {
        if (t != 0) {
          n[cpu_part_passed] = t;
        }
        t = 0;
        cpu_part_passed += 1;
      }
      else if (cpu_part_passed >= 0) {
        t *= 10;
        t += c-'0';
      }
    }
  }
  fclose(f);

  data->cpu.usage = 0;
  for (int i=0; i<data->cpu.count;++i){
    data->cpu.usage += data->cpu.cores_usage[i];
  }
  data->cpu.usage /= data->cpu.count;

  // Обновляем данные по занятости оперативной памяти.
  struct sysinfo sys_info;
  if(sysinfo(&sys_info) != 0){
    printf("*** ERROR: sysinfo");
  }
  data->uptime = sys_info.uptime;
  data->ram.total = sys_info.totalram *(unsigned long long)sys_info.mem_unit;
  FILE *meminfo = fopen("/proc/meminfo", "r");
  unsigned long long free;
  if(meminfo != NULL) {
    while(fgets(buf, sizeof(buf), meminfo)) {
      if(sscanf(buf, "MemAvailable: %lld kB", &free) == 1) {
        data->ram.usage = (unsigned int)((double)(data->ram.total - free * 1024) / data->ram.total * 100);
        break;
      }
    }
    fclose(meminfo);
  }

  struct Disk *current_disk = data->disks_head;
  while (current_disk != NULL) {
    struct statvfs stat;
    if (statvfs(current_disk->path, &stat) == 0) {
      current_disk->total = stat.f_frsize * stat.f_blocks;
      current_disk->used = current_disk->total - stat.f_frsize * stat.f_bfree;
    }
    current_disk = current_disk->next;
  }

  // Сеть.
  for (unsigned int i=0;i<data->net_count;i++){
    // Проверка подключен ли кабель физически.
    // https://stackoverflow.com/questions/808560/how-to-detect-the-physical-connected-state-of-a-network-cable-connector
    /*sprintf(buf, "/sys/class/net/%s/carrier", data->net[i].iface);
        f = fopen(buf, "r");
        if (f == 0) {
            data->net[i].carrier = 0;
        } else {
            if (fgets(buf, 1, f) == NULL) {
                if (strcmp(buf,"1") == 0) {
                    data->net[i].carrier = 1;
                } else {
                    data->net[i].carrier = 0;
                }
            } else {
                data->net[i].carrier = 0;
            }
            fclose(f);
        }*/

    // Обновляем скорость подключения.
    int sock;
    struct ifreq ifr;
    struct ethtool_cmd edata;
    int rc;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
      data->net[i].speed = (unsigned int)(-1);
      data->net[i].carrier = 0;
    } else {
      strncpy(ifr.ifr_name, data->net[i].iface, IFNAMSIZ);
      ifr.ifr_data = &edata;
      edata.cmd = ETHTOOL_GSET;

      rc = ioctl(sock, SIOCETHTOOL, &ifr);
      if (rc < 0) {
        data->net[i].speed = (unsigned int)(-1);
      } else {
        data->net[i].speed = edata.speed;
      }
      rc = ioctl(sock, SIOCGIFFLAGS, &ifr);
      if (rc < 0) {
        data->net[i].carrier = 0;
      } else {
        if (ifr.ifr_flags & IFF_RUNNING) {
          data->net[i].carrier = 1;
        } else {
          data->net[i].carrier = 0;
        }
      }
      ifr.ifr_addr.sa_family = AF_INET;
      rc = ioctl(sock, SIOCGIFADDR, &ifr);
      if (rc < 0) {
        data->net[i].ip[0] = '\0';
      } else {
        sprintf(data->net[i].ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
      }
    }
    close(sock);

    // Обновляем RX/TX.
    unsigned long long rx_prev;
    unsigned long long tx_prev;
    rx_prev = data->net[i].rx;
    tx_prev = data->net[i].tx;

    sprintf(buf, "/sys/class/net/%s/statistics/rx_bytes", data->net[i].iface);
    f = fopen(buf, "r");
    if (f == 0) {
      data->net[i].rx = 0;
    } else {
      if (fgets(buf, sizeof(buf), f) != NULL) {
        sscanf(buf, "%lld\n", &data->net[i].rx);
      } else {
        data->net[i].rx = 0;
      }
      fclose(f);
    }
    sprintf(buf, "/sys/class/net/%s/statistics/tx_bytes", data->net[i].iface);
    f = fopen(buf, "r");
    if (f == 0) {
      data->net[i].tx = 0;
    } else {
      if (fgets(buf, sizeof(buf), f) != NULL) {
        sscanf(buf, "%lld\n", &data->net[i].tx);
      } else {
        data->net[i].tx = 0;
      }
      fclose(f);
    }

    // Обновляем текущие показания скоростей скачивания/закачивания байт в секунду.
    // tx_bytes и rx_bytes обнуляются после 4Гб, поэтому когда на такое натыкаемся,
    // пропускаем цикл оставляя предыдущее значение.
    double delta_bytes = data->net[i].tx - tx_prev;
    if (delta_bytes >= 0) {
      data->net[i].current_tx_speed = (unsigned long long)(delta_bytes / delta * 1000000000.0);
    }
    delta_bytes = data->net[i].rx - rx_prev;
    if (delta_bytes >= 0) {
      data->net[i].current_rx_speed = (unsigned long long)(delta_bytes / delta * 1000000000.0);
    }
  }
}
