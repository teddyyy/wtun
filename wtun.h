#ifndef _WTUN_H_
#define _WTUN_H_

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/inet.h>
#include <linux/socket.h>

extern char *if_name;
extern char *dst_addr;
extern int dst_port;
extern int src_port;

static inline int u_inet_pton(int af, const char *src, void *dst)
{
    if (AF_INET == af) 
        return in4_pton(src, strlen(src), (u8*)dst, '\0', NULL);
    else if (AF_INET6 == af) 
        return in6_pton(src, strlen(src), (u8*)dst, '\0', NULL);
    else 
        return -1;
}

#endif
