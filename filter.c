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
                         else
                                return false;

                } else
                        return false;
        }

        return false;
}

static unsigned int wtun_hook_funk(unsigned int hooknum,
					struct sk_buff *skb,
					const struct net_device *in,
                        		const struct net_device *out,
                        		int (*okfn)(struct sk_buff*))
{
	if (is_tunnel_data(skb)) {
		pr_info("tunnel_data: true %s\n", __func__);
		struct iphdr *iph = ip_hdr(skb);
                int iphlen = iph->ihl << 2;
                int rest_wtun_header_len = iphlen + sizeof(struct udphdr);
		struct wtun_hw *whw = get_wtun_dev();

		skb_pull(skb, rest_wtun_header_len);

		ieee80211_rx_irqsafe(whw->hw, skb);
		
		return NF_STOLEN;

        }

	return NF_ACCEPT;
}

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
