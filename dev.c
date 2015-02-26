#include "wtun.h"
#include "dev.h"
#include "net.h"

static struct wtun_hw *whw = NULL;
static struct station_data *sta_data = NULL;
static struct vint_data *vif_data = NULL;

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

static void transmit_wtun_dev(struct ieee80211_hw *phw,
           			struct sk_buff *skb);

void send_by_tunnel(struct sk_buff *skb);

static struct ieee80211_ops wtun_ops = {
	.start = start_wtun_dev,
	.stop = stop_wtun_dev,

	.add_interface = add_interface_wtun_dev,
	.remove_interface = remove_interface_wtun_dev,

	.config = config_wtun_dev,
	.configure_filter = configure_filter_wtun_dev,
 	.bss_info_changed = changed_bss_info_wtun_dev,

	 /* State routines, not importent, but mandatory. */
  	.sta_add = sta_add_wtun_dev,
  	.sta_remove = sta_remove_wtun_dev,
  	.sta_notify = sta_notify_wtun_dev,

	.tx = transmit_wtun_dev,
};

static struct device_driver wtun_dev_driver = {
  	.name = "wtun-mac80211"
};

static int start_wtun_dev(struct ieee80211_hw *phw)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if (hw != NULL)
		hw->active = true;
		
	return 0;
}

static void stop_wtun_dev(struct ieee80211_hw *phw)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if (hw != NULL)
		hw->active = false;
}

static int add_interface_wtun_dev(struct ieee80211_hw *phw,
                			struct ieee80211_vif *pvif)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;
	pr_info("%s\n", __func__);

	vif->active = true;

	return 0;
}

static void remove_interface_wtun_dev(struct ieee80211_hw *phw,
                			struct ieee80211_vif *pvif)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;
	pr_info("%s\n", __func__);

	vif->active = false;
}

static int config_wtun_dev(struct ieee80211_hw *phw, u32 changed)
{
	struct wtun_hw *hw = NULL;
	pr_info("%s\n", __func__);

  	if (phw != NULL) {
    		hw = (struct wtun_hw *)phw->priv;

    		if (hw != NULL) 
      			hw->idle = !!(phw->conf.flags & IEEE80211_CONF_IDLE);
	}

  	return 0;
}

static void configure_filter_wtun_dev(struct ieee80211_hw *phw,
					unsigned int changed_flags,
					unsigned int *total_flags,
					u64 multicast)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if ((hw != NULL) && (total_flags != NULL)) 
		*total_flags &= (FIF_PROMISC_IN_BSS | FIF_ALLMULTI);
}

static void changed_bss_info_wtun_dev(struct ieee80211_hw *phw,
                   struct ieee80211_vif *pvif,
                   struct ieee80211_bss_conf *bss,
                   u32 changed)
{
	struct vint_data *vif = (struct vint_data *)pvif->drv_priv;
	struct wtun_hw *whw = (struct wtun_hw *)phw->priv;
	pr_info("%s\n", __func__);

	if ((vif != NULL ) && (whw != NULL)) {
		if (vif->active == true) {
			if (changed & BSS_CHANGED_BEACON_INT) {
				whw->ubeacons = (bss->beacon_int * HZ) >> 10;
				if (whw->ubeacons == 0)
					whw->ubeacons = 1;
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

	pr_info("%s\n", __func__);

	if (vint->active == true)
		sta_data->active = true;

	return 0;
}

static int sta_remove_wtun_dev(struct ieee80211_hw *phw,
				struct ieee80211_vif *vif,
				struct ieee80211_sta *sta)
{
	struct station_data *sta_data = (struct station_data *)sta->drv_priv;
	struct vint_data *vint = (struct vint_data *)vif->drv_priv;

	pr_info("%s\n", __func__);

	if (vint->active == true)
		sta_data->active = false;

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

void transmit_beacon(void *p, u8* mac,
			struct ieee80211_vif *vif)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)p;
	struct wtun_hw *whw = (struct wtun_hw *)hw->priv;
	struct sk_buff *skb = NULL;

	if (true == whw->active) {
		pr_info("%s: true\n", __func__);
		skb = ieee80211_beacon_get(hw, vif);

		if (skb == NULL)
			pr_info("%s: skb is null\n", __func__);

		if (NULL != skb) {
				pr_info("transmit beacon by tunnel\n");
				send_by_tunnel(skb);
		}
		whw->ubeacons_count++;
		dev_kfree_skb(skb);
	}
			
}

static int transmit_thread(void *p)
{
	struct wtun_hw *phw = (struct wtun_hw *)p;
	unsigned long ctime;
	int var = 3;

	pr_info("%s\n", __func__);

	set_user_nice(current, -20);

	ctime = jiffies + phw->ubeacons;
	while ((false == kthread_should_stop()) &&
		(false != phw->active)) {
		wait_event_interruptible(phw->plist, var != 0);
		if (true == phw->active) {
			if (jiffies > ctime) {
				pr_info("iterate_active_interfaces\n");
				ieee80211_iterate_active_interfaces_atomic(phw->hw,
									   transmit_beacon,
									   phw->hw);
				ctime = jiffies + phw->ubeacons;
				phw->ubeacons_count++;		
			}
		}
		msleep(1000);
	}
						

	while ((false == kthread_should_stop())) {
		msleep(1000);
	}

	return 0;

}

static void transmit_wtun_dev(struct ieee80211_hw *phw,
				struct sk_buff *skb)
{
	struct wtun_hw *hw = (struct wtun_hw *)phw->priv;

	pr_info("%s\n", __func__);

	if ((hw != NULL) && (skb != NULL)) {
 		struct ieee80211_hdr *ieh = (struct ieee80211_hdr *)skb->data;
		if ((hw->active == true) && (ieh != NULL)) {
			if (skb->len < 10) {
				dev_kfree_skb(skb);
				pr_info("dev_kfree_skb\n");
			}
			else {
				send_by_tunnel(skb);
			}
		}
	}
		
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
	
	if (hw == NULL)
		return -1;
	
	whw = (struct wtun_hw *)hw->priv;
	whw->hw = hw;

	if (whw->class == NULL) {
		whw->class = class_create(THIS_MODULE, "wtun");
		if (IS_ERR(whw->class)) {
			ret = PTR_ERR(whw->class);
			pr_err("class_create err %d\n", ret);
			goto class_del;
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
		return ret;
    	}

	whw->active = true;
	whw->idle = true;
	whw->ubeacons = (1024 * HZ) >> 10;
	init_waitqueue_head(&whw->plist);       
	whw->ubeacons_count = 0;

	/* Specify the supported driver name. */
    	whw->dev->driver = &wtun_dev_driver;
    	SET_IEEE80211_DEV(whw->hw, whw->dev);

    	whw->hw->channel_change_time = 1;
    	whw->hw->queues = 4;
    	whw->hw->wiphy->n_addresses = 1;

    	random_ether_addr((u8 *)&whw->maddr);
    	whw->hw->wiphy->addresses = &whw->maddr;

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

	whw->hw->sta_data_size = sizeof(sta_data);	
	whw->hw->vif_data_size = sizeof(vif_data);	

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

	sprintf(whw->pname, "wtunsendbeacon");
	whw->pth = (void *)kthread_run(transmit_thread, (void *)whw, whw->pname);

	ret = ieee80211_register_hw(whw->hw);
	if (ret < 0) {
		device_unregister(whw->dev);
		ieee80211_free_hw(whw->hw);
	}

	return 0;

class_del:
	class_destroy(whw->class);

	return ret;
}

int destroy_wtun_dev(void)
{
	pr_info("%s\n", __func__);

	if (whw->pth != NULL) {
		kthread_stop((struct task_struct *)whw->pth);
		whw->pth = NULL;
	}

	ieee80211_unregister_hw(whw->hw);
	device_unregister(whw->dev);
	class_destroy(whw->class);
	ieee80211_free_hw(whw->hw);

	return 0;
}
