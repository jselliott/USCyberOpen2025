#ifndef __NETDEV_H
#define __NETDEV_H

#define MAGIC 'n'
#define NETDEV_CHANGE_MODE _IOW(MAGIC, 1, enum netdev_mode)
#define NETDEV_SET_DESTINATION _IOW(MAGIC, 2, struct ip_address)
#define NETDEV_GET_DESTINATION _IOR(MAGIC, 3, struct ip_address *)
#define NETDEV_SEND_ADVERTISEMENT _IOW(MAGIC, 4, struct advertisement *)

enum netdev_mode : char {
    MODE_LOOPBACK = 1,
    MODE_UNICAST = 1 << 1,
    MODE_BROADCAST = 1 << 2,
};

enum ip_version : short {
    IPV4 = 4,
    IPV6 = 6,
};

struct ip_address {
    enum ip_version version;
    union {
        char ipv4[4];
        char ipv6[6];
    } address;
};

struct advertisement {
    char name[16];
    struct ip_address source;
    unsigned long long bandwidth;
};

#endif
