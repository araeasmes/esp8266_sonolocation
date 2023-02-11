#ifndef _ESPNOW_PACKET
#define _ESPNOW_PACKET

#ifndef DATA_LEN
#define DATA_LEN 4
#endif // DATA_LEN

struct IEEE80211_radiotap {
	uint8_t version;                //= 0;
    uint8_t pad;                    //= 0;
    uint16_t length;                //= 0x00,0x26;
    uint32_t present_1;             //= {0x2f, 0x40, 0x40, 0xa0};
    uint32_t present_2;             //= {0x20, 0x80, 0x00, 0xa0
    uint32_t present_3;             //= {0x20, 0x80, 0x00, 0x00
    uint64_t mac_timestamp;         //= timestamp
    uint8_t flags;          		//= 10;
    uint8_t datarate;               //0x0c, in wireshark it's 0x02?
    uint16_t channel_freq;          //0x6c, 0x09
    uint16_t channel_flags_quarter; //0xc0, 0x00; my = 0xa0, 0x00
    uint8_t dmb_antsignal;          //0xce
    uint8_t zero1;                  // 0x00
    uint64_t zero2;                 // 0x00
    uint64_t timestamp;             // ts
    uint16_t timestamp_acc;         // ts acc; on esp8266 is 0x16 0x00
    uint8_t timestamp_samplpos;     // 0x11
    uint8_t ts_flags_acc;           // 0x03
    uint8_t dmb_antsignal2;         // ~0xca
    uint8_t antenna;                // 0x00
    uint8_t dmb_antsignal3;         // ~0xcb
    uint8_t antenna2;               // 0x01
} __attribute__((__packed__));

struct IEEE80211_vendorspecific {
	uint8_t random_bytes[4];
	uint8_t elementID;		//0xdd
	uint8_t length;			//0xff
	uint8_t	OUI[3];			//0x18,0xfe, 0x34
	uint8_t type;			//0x04
	uint8_t version;		//0x01
	uint8_t payload[DATA_LEN];
	
} __attribute__((__packed__));

struct IEEE80211_actionframe {
	uint8_t category_code;	//0x7f
    uint8_t OUI[3];			//0x18,0xfe, 0x34
	struct IEEE80211_vendorspecific content;
} __attribute__((__packed__));

struct IEEE80211_wlan {
	uint8_t type;					//0xd0
    uint8_t flags;            		//0x00
    uint16_t duration;         //0x3a, 0x01
    uint8_t da[6];             //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint8_t sa[6];             //0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b
    uint8_t bssid[6];          //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint16_t seq;              //0x70, 0x51
	struct IEEE80211_actionframe actionframe;
    uint32_t fcs;				// fcs
} __attribute__((__packed__));

typedef struct {

	struct IEEE80211_radiotap radiotap;
	struct IEEE80211_wlan wlan;

} __attribute__((__packed__)) espnow_packet_t;



#endif // _ESPNOW_PACKET
