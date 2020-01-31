#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "mac_functions.h"

int getMAC(char **mac)
{
    struct ifreq s;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(s.ifr_name, "eth0");
    if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
    {
        char _mac[13];
        sprintf(_mac, "%02x%02x%02x%02x%02x%02x", s.ifr_addr.sa_data[0], s.ifr_addr.sa_data[1], s.ifr_addr.sa_data[2], s.ifr_addr.sa_data[3], s.ifr_addr.sa_data[4], s.ifr_addr.sa_data[5]);
        *mac = strdup(_mac);
        return 0;
    }
    return 1;
}
