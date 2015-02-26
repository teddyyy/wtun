#include "wtun.h"
#include "net.h"

static void encap_tunnel_udp_header(struct sk_buff* skb, 
									int udplen, 
									u32 srcip, 
									u32 dstip)
{
	struct udphdr* udph = NULL;

   	skb_push(skb, sizeof(struct udphdr));
   	skb_reset_transport_header(skb);

   	udph = udp_hdr(skb);
   	udph->source = htons(dst_port);
   	udph->dest = htons(dst_port);
   	udph->len = htons(udplen);

   	udph->check = 0;
   	udph->check = csum_tcpudp_magic(srcip, dstip, udplen, 
									IPPROTO_UDP, 
									csum_partial(udph, udplen, 0));
}

static void encap_tunnel_ip_header(struct sk_buff* skb, 
									int iplen, 
									u32 srcip, 
									u32 dstip)
{
   	static atomic_t ip_ident;

   	struct iphdr* iph = NULL;

   	skb_push(skb, sizeof(struct iphdr));
   	skb_reset_network_header(skb);

   	iph = ip_hdr(skb);
   	// the following line is equivalent to iph->version = 4; iph->ihl = 5;
   	put_unaligned(0x45, (unsigned char*)iph);
   	iph->tos = 0;
   	put_unaligned(htons(iplen), &(iph->tot_len));
   	iph->id = htons(atomic_inc_return(&ip_ident));
   	iph->frag_off = 0;
   	iph->ttl = 64;
   	iph->protocol = IPPROTO_UDP;
   	iph->check = 0;
   	put_unaligned(srcip, &(iph->saddr));
   	put_unaligned(dstip, &(iph->daddr));

   	iph->check = ip_fast_csum((unsigned char*)iph, iph->ihl);
}

static void encap_tunnel_eth_header(struct sk_buff* skb, 
									u8* srcmac, 
									u8* dstmac)
{
   	struct ethhdr* eth = (struct ethhdr*)skb_push(skb, ETH_HLEN);
   	skb_reset_mac_header(skb);

   	skb->protocol = eth->h_proto = htons(ETH_P_IP);
   	memcpy(eth->h_source, srcmac, ETH_ALEN);
   	memcpy(eth->h_dest, dstmac, ETH_ALEN);
}

static struct rtable* find_routing_table(u32 dstip)
{
   	struct flowi4 fl4 = {.daddr = dstip};
   	struct rtable* rtbl = NULL;
   	rtbl = ip_route_output_key(&init_net, &fl4);

   	// NOTICE:
   	//
   	// ip_route_output_key will return an NON-NULL address even if no routing table found.
   	// system will be crached (or stuck?) if accessing it.
   	//
   	// it is necessary that do an extra check by IS_ERR().

   	return (rtbl && !IS_ERR(rtbl)) ? rtbl : NULL;
}

static u32 find_source_ip(struct rtable* rtbl)
{
   	struct in_device* indev = __in_dev_get_rtnl(rtbl->dst.dev);
   	return indev->ifa_list->ifa_local;
}

static void get_destination_mac(u32 dstip, 
								struct rtable* rtbl, 
								u8* dstmac)
{
   	extern struct neigh_table arp_tbl;
   	struct neighbour* neigh = neigh_lookup(&arp_tbl, &dstip, rtbl->dst.dev);
   	if (!neigh || IS_ERR(neigh)) {
       	// do broadcast if no neighbour exists.
       	// i know it is a little dirty, but works.
       	memset(dstmac, 0xff, ETH_ALEN);
   	} else {
       	memcpy(dstmac, neigh->ha, ETH_ALEN);
   	}
}

static int encap_tunnel_skb(struct sk_buff* skb)
{
   	// i copy some contents in netpoll_send_udp(), modify them,
   	// and separate them into the following functions:
   	//
   	// 1. this function (encap_tunnel_skb)
   	// 2. encap_tunnel_udp_header
   	// 3. encap_tunnel_ip_header
   	// 4. encap_tunnel_eth_header
   	//
   	// if you want to know more,
   	// see netpoll_send_udp() in "netpoll.c" of linux kernel source.

   	int payloadlen, udplen, iplen;
   	u32 srcip, dstip;
   	u8  srcmac[ETH_ALEN], dstmac[ETH_ALEN];
   	struct rtable* rtbl = NULL;
   	int ret;

   	payloadlen = skb->len;
   	udplen = payloadlen + sizeof(struct udphdr);
   	iplen = udplen + sizeof(struct iphdr);

   	ret = u_inet_pton(AF_INET, dst_addr, &dstip);
	if (ret < 0)  
    	goto error;

   	rtbl = find_routing_table(dstip);
	if (rtbl < 0)  
       	goto error;

   	srcip = find_source_ip(rtbl);

   	memcpy(&srcmac, rtbl->dst.dev->dev_addr, ETH_ALEN);
   	get_destination_mac(dstip, rtbl, dstmac);

   	encap_tunnel_udp_header(skb, udplen, srcip, dstip);
   	encap_tunnel_ip_header(skb, iplen, srcip, dstip);
   	encap_tunnel_eth_header(skb, srcmac, dstmac);

   	skb->dev = rtbl->dst.dev;

   	return 0;

error:
   	return -1;
}


void send_by_tunnel(struct sk_buff *skb)
{
	struct sk_buff* newskb = NULL;
   	// tunnel header will be packed by ourself
   	// we can limit ip header size to 20 bytes = sizeof(struct iphdr)
   	int headroom = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
   	int ret;

   	// skb_realloc_headroom will return a new skb which is created 
	// by skb_clone or skb_copy in different cases.

   	newskb = skb_realloc_headroom(skb, headroom);
	if (newskb == NULL)  
       	goto error;

   	ret = encap_tunnel_skb(newskb);
	if (ret < 0)  
       	goto error;

   	ret = dev_queue_xmit(newskb);
	if (ret < 0)  
       	goto error;

   	return;

error:
   	if (newskb) 
	kfree_skb(newskb);
}

