#include "wtun.h"
#include "dev.h"
#include "net.h"

static struct wtun_hw *whw = NULL;

static int start_wtun_dev(struct ieee80211_hw *phw);
static void stop_wtun_dev(struct ieee80211_hw *phw);

static int add_interface_wtun_dev(struct ieee80211_hw *phw,
								  struct ieee80211_vif *pvif);
static void remove_interface_wtun_dev(struct ieee80211_hw *phw,
									  struct ieee80211_vif *pvif);
static int config_wtun_dev(struct ieee80211_hw *phw, u32 changed);

static void configure_filter_wtun_dev(struct ieee80211_hw *phw,
									  unsigned int changed_flags,
									  unsigned int *total_flags,
									  u64 multicast);
static void changed_bss_info_wtun_dev(struct ieee80211_hw *phw,
									  struct ieee80211_vif *pvif,
									  struct ieee80211_bss_conf *bss,
									  u32 changed);

static int sta_add_wtun_dev(struct ieee80211_hw *phw,
							struct ieee80211_vif *vif,
							struct ieee80211_sta *sta);
static int sta_remove_wtun_dev(struct ieee80211_hw *phw,
							   struct ieee80211_vif *vif,
							   struct ieee80211_sta *sta);
static void sta_notify_wtun_dev(struct ieee80211_hw *phw,
								struct ieee80211_vif *vif,
								enum sta_notify_cmd emd,
								struct ieee80211_sta *sta);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
static void transmit_wtun_dev(struct ieee80211_hw *hw,
                              struct ieee80211_tx_control *control,
                              struct sk_buff *skb);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
static void transmit_wtun_dev(struct ieee80211_hw *hw, 
							  struct sk_buff *skb);
#endif

static int conf_tx_wtun_dev(struct ieee80211_hw *hw,
                            struct ieee80211_vif *vif,
                            u16 queue,
                            const struct ieee80211_tx_queue_params *param);


void send_by_tunnel(struct sk_buff *skb);

static struct ieee80211_ops wtun_ops = {
	.start = start_wtun_dev,
	.stop = stop_wtun_dev,

	.add_interface = add_interface_wtun_dev,
	.remove_interface = remove_interface_wtun_dev,

 	.bss_info_changed = changed_bss_info_wtun_dev,
	.configure_filter = configure_filter_wtun_dev,

	 /* State routines, not importent, but mandatory. */
  	.sta_add = sta_add_wtun_dev,
  	.sta_remove = sta_remove_wtun_dev,
  	.sta_notify = sta_notify_wtun_dev,

	.tx = transmit_wtun_dev,
	.config = config_wtun_dev,

	.conf_tx = conf_tx_wtun_dev,
};

static struct device_driver wtun_dev_driver = {
  	.name = "wtun-mac80211"
};

static int start_wtun_dev(struct ieee80211_hw *phw)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if (NULL != hw)
		hw->radio_active = true;
		
	return 0;
}

static void stop_wtun_dev(struct ieee80211_hw *phw)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if (NULL != hw)
		hw->radio_active = false;
}

static int add_interface_wtun_dev(struct ieee80211_hw *phw,
                					struct ieee80211_vif *pvif)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;

	pr_info("%s\n", __func__);

	if (NULL != vif)
		vif->active = true;

	return 0;
}

static void remove_interface_wtun_dev(struct ieee80211_hw *phw,
                						struct ieee80211_vif *pvif)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;
	pr_info("%s\n", __func__);

	if (NULL != vif)
		vif->active = false;
}

static int config_wtun_dev(struct ieee80211_hw *phw, u32 changed)
{
	struct wtun_hw *hw = NULL;
	pr_info("%s\n", __func__);

  	if (NULL != phw) {
    	hw = (struct wtun_hw *)phw->priv;
    	if (NULL != hw) 
      		hw->idle = !!(phw->conf.flags & IEEE80211_CONF_IDLE);
		else 
			pr_info("error hw (%p)\n", hw);
	} else 
			pr_info("error phw (%p) changed (%d)\n", phw, changed);
	
  	return 0;
}

static void configure_filter_wtun_dev(struct ieee80211_hw *phw,
										unsigned int changed_flags,
										unsigned int *total_flags,
										u64 multicast)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if ((NULL != hw) && (NULL != total_flags)) 
		*total_flags &= (FIF_PROMISC_IN_BSS | FIF_ALLMULTI);
	else
		pr_info("error phw (%p) changed_flags (%d) total_flags (%p) multicast (%lld)\n", 
				phw, changed_flags, total_flags, multicast);
}

static void changed_bss_info_wtun_dev(struct ieee80211_hw *phw,
                   						struct ieee80211_vif *pvif,
                   						struct ieee80211_bss_conf *bss,
                   						u32 changed)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;
	struct wtun_hw *whw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if ((NULL != vif) && (NULL != whw)) {
		if (vif->active) {
			if (changed & BSS_CHANGED_BEACON_INT) {
				if (NULL != whw) {
					whw->ubeacons = (bss->beacon_int * HZ) >> 10;
					if (0 == whw->ubeacons)
						whw->ubeacons = 1;
				}
				pr_info("beacons: %lu\n", whw->ubeacons);
			}
		}
	}
}

static int sta_add_wtun_dev(struct ieee80211_hw *phw,
							struct ieee80211_vif *vif,
							struct ieee80211_sta *sta)
{
	struct station_data *sta_data = (struct station_data *)sta->drv_priv;
	struct vint_data *vint = (struct vint_data *)vif->drv_priv;

	pr_info("%s: %u\n", __func__, sta->aid);

	if ((NULL != sta_data) && (NULL != vint))
		if (vint->active)
			sta_data->active = true;

	pr_info("called phw(%p) vif(%p) sta(%p)\n", phw, vint, sta_data);

	return 0;
}

static int sta_remove_wtun_dev(struct ieee80211_hw *phw,
							   struct ieee80211_vif *vif,
							   struct ieee80211_sta *sta)
{
	struct station_data *sta_data = (struct station_data *)sta->drv_priv;
	struct vint_data *vint = (struct vint_data *)vif->drv_priv;

	pr_info("%s: %u\n", __func__, sta->aid);

	if ((NULL != sta_data) && (NULL != vint))
		if (vint->active)
			sta_data->active = false;

	pr_info("called phw(%p) vif(%p) sta(%p)\n", phw, vint, sta_data);

	return 0;
}

static void sta_notify_wtun_dev(struct ieee80211_hw *phw,
								struct ieee80211_vif *vif,
								enum sta_notify_cmd cmd,
								struct ieee80211_sta *sta)
{
	pr_info("%s\n", __func__);

	return;
}

void transmit_beacon(void *p, u8* mac, struct ieee80211_vif *vif)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)p;
	struct wtun_hw *whw = (struct wtun_hw *)hw->priv;
	struct sk_buff *skb = NULL;

	if (whw->active) {
		if (whw->radio_active) {
			skb = ieee80211_beacon_get(hw, vif);

			if (NULL != skb) {
				if (is_wanted_data(skb)) 
					send_by_tunnel(skb);
				
				whw->ubeacons_count++;
				dev_kfree_skb(skb);
			}
		}
	}
}

static int skb_polling(void *p)
{
	struct wtun_hw *phw = (struct wtun_hw *)p;
	int ret = 0;
	unsigned long flags;

	if (NULL != phw) {
		if (phw->active) {
			spin_lock_irqsave(&phw->pspin, flags);
			if (skb_queue_len(&phw->head_skb) > 0) 
				ret = 1;
			spin_unlock_irqrestore(&phw->pspin, flags);
		} else 
			ret = -1;
	}
	
	return ret;
}

static int transmit_thread(void *p)
{
	struct wtun_hw *phw = (struct wtun_hw *)p;
	unsigned long ctime;
	unsigned int uqos = 0;

	set_user_nice(current, -20);

	ctime = jiffies + phw->ubeacons;
	while ((!kthread_should_stop()) && (phw->active)) {
		wait_event_interruptible_timeout(phw->plist, 
										((uqos = skb_polling(phw)) > 0),
										whw->ubeacons);
		if (phw->active) {
			if (phw->radio_active) {
				if (jiffies > ctime) {
					#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0))
					ieee80211_iterate_active_interfaces_atomic(phw->hw,
										   						transmit_beacon,
										   						phw->hw);
					#else  
					ieee80211_iterate_active_interfaces_atomic(phw->hw,
																IEEE80211_IFACE_ITER_NORMAL,
										   						transmit_beacon,
										   						phw->hw);
					#endif
					ctime = jiffies + phw->ubeacons;
					phw->ubeacons_count++;		
				}
			}
		}
	}
						
	while ((!kthread_should_stop())) 
		msleep(1000);

	return 0;
}

static void xmit_skb_dev(struct ieee80211_hw *phw,
								struct sk_buff *skb)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	struct ieee80211_tx_info *tx_info = NULL;

	if ((NULL != hw) && (NULL != skb)) {
		if ((hw->radio_active)) {
			if (skb->len < 10) 
				dev_kfree_skb(skb);
			else 
				if (is_wanted_data(skb)) 
					send_by_tunnel(skb);
 		}

		skb_orphan(skb);
		skb_dst_drop(skb);
		skb->mark = 0;
		secpath_reset(skb);
		nf_reset(skb);

		tx_info = IEEE80211_SKB_CB(skb);
		ieee80211_tx_info_clear_status(tx_info);
		if (!(tx_info->flags & IEEE80211_TX_CTL_NO_ACK))
			tx_info->flags |= IEEE80211_TX_STAT_ACK;
		ieee80211_tx_status(hw->hw, skb);
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
static void transmit_wtun_dev(struct ieee80211_hw *hw,
							  struct ieee80211_tx_control *control,
                              struct sk_buff *skb)
{
	xmit_skb_dev(hw, skb);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
static void transmit_wtun_dev(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	xmit_skb_dev(hw, skb);
}
#endif

static int conf_tx_wtun_dev(struct ieee80211_hw *hw,
							struct ieee80211_vif *vif,
							u16 queue,
							const struct ieee80211_tx_queue_params *param)
{
	struct wtun_hw *whw = NULL;
	pr_info("%s\n", __func__);

	if ((NULL != hw) && (NULL != param)) {
		if (NULL != hw->priv) {
			whw = (struct wtun_hw *)hw->priv;
		}
	}

	return 0;
}	

struct wtun_hw* get_wtun_dev(void)
{
	return whw;
}

int create_wtun_dev(void)
{
	struct ieee80211_hw *hw = NULL;
	int ret;

	pr_info("%s\n", __func__);

	hw = ieee80211_alloc_hw(sizeof(whw), &wtun_ops);
	if (NULL == hw)
		return -1;
	
	whw = (struct wtun_hw *)hw->priv;
	whw->hw = hw;

	if (NULL == whw->class) {
		whw->class = class_create(THIS_MODULE, "wtun");
		if (IS_ERR(whw->class)) {
			ret = PTR_ERR(whw->class);
			pr_err("class_create err %d\n", ret);
			class_destroy(whw->class);
		}
	}

	sprintf(whw->dev_name, "wtun-mac80211");
	/* This is a GPL exported only function. */
   	whw->dev = device_create(whw->class,
									NULL,
                	                0,
                        	        whw->hw,
                                  	whw->dev_name,
                                	0);	

	if (IS_ERR(whw->dev)) {
		ret = PTR_ERR(whw->dev);
   		pr_err("Failed to get create a wireless device\n");
   		ieee80211_free_hw(hw);
		class_destroy(whw->class);
		return ret;
   	}

	/* setup wireless driver */
	whw->active = true;
	whw->radio_active = false;
	whw->idle = true;
	whw->ubeacons = (1024 * HZ) >> 10;
	whw->ubeacons_count = 0;

	spin_lock_init(&whw->pspin);
	init_waitqueue_head(&whw->plist);       
	skb_queue_head_init(&whw->head_skb);

	/* Specify the supported driver name. */
    whw->dev->driver = &wtun_dev_driver;
    SET_IEEE80211_DEV(whw->hw, whw->dev);

	/* generate mac address */
    random_ether_addr((u8 *)&whw->maddr);
    whw->hw->wiphy->addresses = &whw->maddr;

	/* setup hardware data */
    whw->hw->channel_change_time = 1;
    whw->hw->queues = 4;
    whw->hw->wiphy->n_addresses = 1;

	whw->hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_MESH_POINT) |
      								       BIT(NL80211_IFTYPE_ADHOC) |
      									   BIT(NL80211_IFTYPE_AP) |
      									   BIT(NL80211_IFTYPE_P2P_CLIENT) |
      									   BIT(NL80211_IFTYPE_P2P_GO) |
      									   BIT(NL80211_IFTYPE_STATION);

   	whw->hw->flags = IEEE80211_HW_MFP_CAPABLE |
						IEEE80211_HW_SIGNAL_DBM |
      					IEEE80211_HW_SUPPORTS_STATIC_SMPS |
      					IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS |
      					IEEE80211_HW_AMPDU_AGGREGATION;

	whw->hw->sta_data_size = sizeof(struct station_data);	
	whw->hw->vif_data_size = sizeof(struct vint_data);	

	memcpy(whw->channel, wtun_channel, sizeof(wtun_channel));
	whw->band.channels = whw->channel;
   	whw->band.n_channels = CHANNEL_SIZE;

	memcpy(whw->rate, wtun_rate, sizeof(wtun_rate));
   	whw->band.bitrates = whw->rate;
   	whw->band.n_bitrates = RATE_SIZE;
   	whw->band.ht_cap.ht_supported = true;
   	whw->band.ht_cap.cap = 
			IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
      		IEEE80211_HT_CAP_GRN_FLD |
      		IEEE80211_HT_CAP_SGI_40 |
      		IEEE80211_HT_CAP_DSSSCCK40;
   	whw->band.ht_cap.ampdu_factor = 0x3;
   	whw->band.ht_cap.ampdu_density = 0x6;
   	memset(&whw->band.ht_cap.mcs, 0, sizeof(whw->band.ht_cap.mcs));
   	whw->band.ht_cap.mcs.rx_mask[0] = 0xff;
   	whw->band.ht_cap.mcs.rx_mask[1] = 0xff;
   	whw->band.ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
   	whw->hw->wiphy->bands[IEEE80211_BAND_2GHZ] = &whw->band;

	/* create kernel thread for beacon */
	sprintf(whw->pname, "xmit_thread");
	whw->pth = (void *)kthread_run(transmit_thread, (void *)whw, whw->pname);

	ret = ieee80211_register_hw(whw->hw);
	if (ret < 0) {
		device_unregister(whw->dev);
		ieee80211_free_hw(whw->hw);
		whw = NULL;
	}

	return 0;
}

int destroy_wtun_dev(void)
{
	pr_info("%s\n", __func__);

	whw->active = false;
	whw->radio_active = false;

	if (NULL != whw->pth) {
		kthread_stop((struct task_struct *)whw->pth);
		whw->pth = NULL;
	}	

	if (NULL != whw->hw) {
		ieee80211_unregister_hw(whw->hw);
		if (NULL != whw->dev) 
			device_unregister(whw->dev);
		if (NULL != whw->class) 
			class_destroy(whw->class);
		ieee80211_free_hw(whw->hw);
		whw = NULL;
	}

	return 0;
}
