#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xef025c67, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x83d7ab1a, __VMLINUX_SYMBOL_STR(neigh_lookup) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xb6b46a7c, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x79aa04a2, __VMLINUX_SYMBOL_STR(get_random_bytes) },
	{ 0x95a49215, __VMLINUX_SYMBOL_STR(arp_tbl) },
	{ 0xc64c88af, __VMLINUX_SYMBOL_STR(dst_release) },
	{ 0x87c18a2d, __VMLINUX_SYMBOL_STR(ieee80211_beacon_get_tim) },
	{ 0x8fa9845, __VMLINUX_SYMBOL_STR(ieee80211_unregister_hw) },
	{ 0xbea03e22, __VMLINUX_SYMBOL_STR(ieee80211_iterate_active_interfaces_atomic) },
	{ 0xad7d03ce, __VMLINUX_SYMBOL_STR(nf_register_hook) },
	{ 0xcc376a7, __VMLINUX_SYMBOL_STR(skb_realloc_headroom) },
	{ 0xc1f2f4d0, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x5d41c87c, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x8cb1ff45, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x3bf9e5b, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xf12f709a, __VMLINUX_SYMBOL_STR(ieee80211_rx) },
	{ 0xbe01f3e0, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0xcf418b81, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x5c0ece6b, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0x72ee5686, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0x5acf135, __VMLINUX_SYMBOL_STR(__secpath_destroy) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xb712e0bc, __VMLINUX_SYMBOL_STR(ieee80211_tx_status_irqsafe) },
	{        0, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0xe973ddbf, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0xaccabc6a, __VMLINUX_SYMBOL_STR(in4_pton) },
	{ 0x5d4d5071, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x2a18c74, __VMLINUX_SYMBOL_STR(nf_conntrack_destroy) },
	{ 0x1b11d960, __VMLINUX_SYMBOL_STR(ip_route_output_flow) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0x34f22f94, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x5c34f43d, __VMLINUX_SYMBOL_STR(ieee80211_register_hw) },
	{ 0x9ac87c05, __VMLINUX_SYMBOL_STR(nf_unregister_hook) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x138168e7, __VMLINUX_SYMBOL_STR(ieee80211_alloc_hw) },
	{ 0xc63fd772, __VMLINUX_SYMBOL_STR(set_user_nice) },
	{ 0xa87912d1, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x88c6bc5a, __VMLINUX_SYMBOL_STR(ieee80211_free_hw) },
	{ 0xfa66f77c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x3890279b, __VMLINUX_SYMBOL_STR(device_unregister) },
	{ 0xe113bbbc, __VMLINUX_SYMBOL_STR(csum_partial) },
	{ 0x80e2835c, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0x4febcff9, __VMLINUX_SYMBOL_STR(dev_queue_xmit) },
	{ 0xa64a404a, __VMLINUX_SYMBOL_STR(__class_create) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=mac80211";


MODULE_INFO(srcversion, "DD9D83F83763B1040E75C8D");
