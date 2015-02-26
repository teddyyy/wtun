#include <net/mac80211.h>
#include <net/cfg80211.h>
#include <linux/etherdevice.h>
#include <linux/device.h>
#include <linux/kthread.h>

#define CHANNEL_SIZE ARRAY_SIZE(wtun_channel)
#define RATE_SIZE ARRAY_SIZE(wtun_rate)

int create_wtun_dev(void);
int destroy_wtun_dev(void);
struct wtun_hw* get_wtun_dev(void);

static struct ieee80211_channel wtun_channel[] = {
        {
                .band = IEEE80211_BAND_2GHZ,
                .center_freq = 2412,
                .hw_value = 2412,
                .max_power = 25,
        },
        {
                .band = IEEE80211_BAND_2GHZ,
                .center_freq = 2437,
                .hw_value = 2437,
                .max_power = 25,
        },
        {
                .band = IEEE80211_BAND_2GHZ,
                .center_freq = 2462,
                .hw_value = 2462,
                .max_power = 25,
        },
};

static const struct ieee80211_rate wtun_rate[] = {
        { .bitrate = 10,  .flags = 0 },
        { .bitrate = 20,  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
        { .bitrate = 55,  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
        { .bitrate = 110, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
        { .bitrate = 60,  .flags = 0 },
        { .bitrate = 120, .flags = 0 },
        { .bitrate = 240, .flags = 0 },
};

struct station_data {
	bool active;
};

struct vint_data {
	bool active;
};

struct wtun_hw {
	struct ieee80211_hw *hw;
	struct device *dev;
	struct class *class;

	int power;
	bool idle;
	unsigned long ubeacons;
	unsigned long ubeacons_count;
	char dev_name[64];
	struct mac_address maddr;

	bool active;
	
	struct ieee80211_supported_band band;		
	struct ieee80211_channel channel[CHANNEL_SIZE];
	struct ieee80211_rate rate[RATE_SIZE];
	
	char pname[64];
	void *pth;
	spinlock_t pspin;
	wait_queue_head_t plist;
};

