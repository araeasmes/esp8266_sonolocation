//Copyright 2015, 2018 <>< Charles Lohr, Adam Feinstein see LICENSE file.

/*==============================================================================
 * Includes
 *============================================================================*/

#include "mem.h"
#include "c_types.h"
#include "espnow.h"
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "user_interface.h"

#include "partition_util.h"

struct button_info {
    uint8 is_down;
};

struct button_info g_button;


// Pin defines

// pin D6 on NodeMCU
#define LED_PIN FUNC_GPIO12 
#define LED_NUM 12

// pin D5 on NodeMCU
#define BUTTON_PIN FUNC_GPIO14
#define BUTTON_NUM 14

struct send_data_t {
    uint32_t counter;
} __attribute__((__packed__));

#define DATA_LEN sizeof(struct send_data_t)
struct send_data_t send_data;
uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 


static os_timer_t some_timer;

#define switchTaskPrio        0
#define switchTaskQueueLen    1
os_event_t    switchTaskQueue[switchTaskQueueLen];

static void ICACHE_FLASH_ATTR switchTask(os_event_t *events)
{
    os_printf("signal\n");
    esp_now_send(broadcast_mac, (u8*) &send_data, DATA_LEN);
    send_data.counter++;
}

/**
 * This is a timer set up in user_main() which is called every 100ms, forever
 * @param arg unused
 */
static void ICACHE_FLASH_ATTR timer100ms(void *arg)
{
    uint32 button_status = 0;
    button_status = GPIO_INPUT_GET(BUTTON_NUM);

    // GPIO_OUTPUT_SET(LED_NUM, button_status);
    // GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_NUM)); // - didn't work
}

void buttonISR(void *data)
{
    struct button_info *button = (struct button_info*) data;
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    if (gpio_status & BIT(BUTTON_NUM)) {
        gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_NUM), GPIO_PIN_INTR_DISABLE);
        // copied the next line from example at driver/key.c, but it should be sufficient
        // to just write BIT(BUTTON_NUM), is it not?
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(BUTTON_NUM));

        if (button->is_down == 0) {
            button->is_down = 1;

            gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_NUM), GPIO_PIN_INTR_NEGEDGE);

            system_os_post(switchTaskPrio, 0, 0 );

            GPIO_OUTPUT_SET(LED_NUM, 1);
        } else {
            button->is_down = 0;

            gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_NUM), GPIO_PIN_INTR_POSEDGE);

            GPIO_OUTPUT_SET(LED_NUM, 0);
        }
    } else {
        os_printf("wadafak\n");
    }
}

void ICACHE_FLASH_ATTR
button_isr_init()
{
    g_button.is_down = 0;
    ETS_GPIO_INTR_ATTACH(buttonISR, (void*) &g_button);
    ETS_GPIO_INTR_DISABLE();

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, BUTTON_PIN); // pin D5 on NodeMCU

    gpio_output_set(0, 0, 0, GPIO_ID_PIN(BUTTON_NUM));

    gpio_register_set(GPIO_PIN_ADDR(BUTTON_NUM), 
            GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE) 
            | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
            | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    // clear gpio14 status???????? - comment from driver/key.c from esp8266 nonos sdk examples
    // seems like W1TC and W1TS are used for setting pins to 1 and 0, being faster than the set macros 
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(BUTTON_NUM));
    
    // enable interrupt
    gpio_pin_intr_state_set(GPIO_ID_PIN(BUTTON_NUM), GPIO_PIN_INTR_NEGEDGE);

    ETS_GPIO_INTR_ENABLE();
}

/**
 * UART RX handler, called by the uart task. Currently does nothing
 *
 * @param c The char received on the UART
 */
void ICACHE_FLASH_ATTR charrx( uint8_t c )
{
	//Called from UART.
}

/**
 * This is called on boot for versions ESP8266_NONOS_SDK_v1.5.2 to
 * ESP8266_NONOS_SDK_v2.2.1. system_phy_set_rfoption() may be called here
 */
void user_rf_pre_init(void)
{
	; // nothing
}


void ICACHE_FLASH_ATTR user_pre_init(void)
{
	LoadDefaultPartitionMap(); //You must load the partition table so the NONOS SDK can find stuff.
}

/**
 * The default method, equivalent to main() in other environments. Handles all
 * initialization
 */
void ICACHE_FLASH_ATTR user_init(void)
{
	// Initialize the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

	os_printf("\r\nesp82XX stripped\r\n%s\b", VERSSTR);

    bool res = wifi_set_opmode(1); // station mode
    os_printf("station mode set = %d\n", res);

    int int_res = esp_now_init();
    os_printf("esp_now_init = %d\n", int_res);
    int_res = esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    os_printf("set self as controller = %d\n", int_res);
    u8 wifi_channel = wifi_get_channel();
    os_printf("wifi channel is #%u\n", wifi_channel);

    os_bzero((void*) &send_data, DATA_LEN); 




    button_isr_init();

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, LED_PIN); // pin D6 on NodeMCU
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, LED_PIN); // built-in led pin

	// Set timer100ms to be called every 100ms
	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *)timer100ms, NULL);
	os_timer_arm(&some_timer, 100, 0);

	os_printf( "Boot Ok.\n" );

    system_os_task(switchTask, switchTaskPrio, switchTaskQueue, switchTaskQueueLen);
}

