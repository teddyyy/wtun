#include <linux/ip.h>
#include <linux/udp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_arp.h>

int create_netfilter_hook(void);
int destroy_netfilter_hook(void);
