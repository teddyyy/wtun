#include "wtun.h"

char	*dst_addr = NULL;
int	dst_port = 5055;
int	src_port = 5065;

module_param(dst_addr, charp, S_IRUSR);
module_param(dst_port, int, S_IRUSR);

int create_wtun_dev(void);
int destroy_wtun_dev(void);

int create_netfilter_hook(void);
int destroy_netfilter_hook(void);

static int __init wtun_init(void)
{
	int ret;

	if (NULL == dst_addr) {
		pr_err("No destination address\n");
		return -1;
	}

	ret = create_wtun_dev();
	if (ret < 0) {
		pr_err("Failed create wtun device\n");
		return -1;
	}

	ret = create_netfilter_hook();
	if (ret < 0) {
		pr_err("Failed create wtun netfilter\n");
		return -1;
	}

	pr_info("%s: Sucessfully module inited\n", __func__);

	return 0;
}

static void __exit wtun_exit(void)
{
	destroy_netfilter_hook();
	destroy_wtun_dev();

	pr_info("%s: Sucessfully module removed\n", __func__);
}

module_init(wtun_init);
module_exit(wtun_exit);

MODULE_AUTHOR("KIMOTO Mizuki");
MODULE_DESCRIPTION("Wireless Tunnel Kernel Module");
MODULE_LICENSE("GPL");
