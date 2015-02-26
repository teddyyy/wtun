#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inetdevice.h>

#include <net/route.h>
#include <asm/unaligned.h>

void send_by_tunnel(struct sk_buff *skb);

