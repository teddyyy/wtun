#include "wtun.h"
#include "dev.h"
#include "filter.h"

static struct nf_hook_ops wtun_hook = {};

static bool is_tunnel_data(struct sk_buff *skb)
{
	struct iphdr *iph = (struct iphdr*)skb_network_header(skb);
    struct udphdr *udph;
    u32 srcip;
    int ret;

    ret = u_inet_pton(AF_INET, dst_addr, &srcip);
    if (ret < 0)
  		return false;

    if (!iph)
        return false;

    skb->transport_header = skb->network_header + (iph->ihl * 4);

    if (iph->saddr == srcip) {
		if (iph->protocol == IPPROTO_UDP) {
        	udph = (struct udphdr *)skb_transport_header(skb);
            if (!udph)
            	return false;

            if (ntohs(udph->dest) == dst_port)
            	return true;
		}
	}

	return false;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0))
static unsigned int wtun_hook_funk(unsigned int hooknum,
									struct sk_buff *skb,
									const struct net_device *in,
                       				const struct net_device *out,
                       				int (*okfn)(struct sk_buff*))
{
	if (is_tunnel_data(skb)) {
		int iphlen, rest_wtun_header_len;
		struct ieee80211_rx_status stat;
		struct wtun_hw *whw = NULL;
		struct iphdr *iph = ip_hdr(skb);

        iphlen = iph->ihl << 2;
        rest_wtun_header_len = iphlen + sizeof(struct udphdr);
		whw = get_wtun_dev();

		if (NULL != whw) {
			skb_pull(skb, rest_wtun_header_len);
			if (NULL != skb) {
				memset(&stat, 0, sizeof(stat));
			
				stat.band = (u32)whw->hw->conf.channel->band;
				stat.freq = (u32)whw->hw->conf.channel->center_freq;	
				stat.signal = (u32)whw->hw->conf.power_level;	
				stat.rate_idx = 1;
				memcpy(IEEE80211_SKB_RXCB(skb), &stat, sizeof(stat));

				if ((whw->radio_active) && (whw->active)) {
					ieee80211_rx(whw->hw, skb);
					return NF_STOLEN;
				}
			}
		}
	}

	return NF_ACCEPT;
}

#else 
static unsigned int wtun_hook_funk(const struct nf_hook_ops *ops,
									struct sk_buff *skb,
									const struct net_device *in,
									const struct net_device *out,
                       				int (*okfn)(struct sk_buff*))
{
	if (is_tunnel_data(skb)) {
		int iphlen, rest_wtun_header_len;
		struct wtun_hw *whw = NULL;
		struct iphdr *iph = ip_hdr(skb);

        iphlen = iph->ihl << 2;
        rest_wtun_header_len = iphlen + sizeof(struct udphdr);
		whw = get_wtun_dev();

		if (NULL != whw) {
			skb_pull(skb, rest_wtun_header_len);
			if (NULL != skb) {
				if ((whw->radio_active) && (whw->active)) {
					ieee80211_rx(whw->hw, skb);
					return NF_STOLEN;
				}
			}
		}
	}

	return NF_ACCEPT;
}
#endif

int create_netfilter_hook(void)
{
	pr_info("%s\n", __func__);

    wtun_hook.pf = PF_INET;
    wtun_hook.hooknum = NF_INET_LOCAL_IN;
    wtun_hook.priority = NF_IP_PRI_FIRST;
    wtun_hook.hook = wtun_hook_funk;

    return nf_register_hook(&wtun_hook);
}

int destroy_netfilter_hook(void)
{
    pr_info("%s\n", __func__);

	nf_unregister_hook(&wtun_hook);

    return 0;
}
