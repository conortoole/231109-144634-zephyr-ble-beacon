/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <device.h>
#include <stdlib.h>
//#include <stdio.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define LED0_NODE DT_ALIAS(led0)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define HOLD_SIZE 200

uint8_t gpsIdentifier[] = {'!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!'};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void bt_ready(int err)
{
	uint8_t temp[16];

	int count = 0;

	for (int i = 0; i < 16; i++){
		temp[count] = gpsIdentifier[i];
		count++;
	}

	char addr_s[BT_ADDR_LE_STR_LEN];
	struct bt_data ad[] = {
		BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
        BT_DATA(BT_DATA_MANUFACTURER_DATA, temp, 16)
	};

	bt_addr_le_t addr = {0};
	size_t count2 = 1;

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));

	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	bt_id_get(&addr, &count2);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

	k_sleep(K_MSEC(5000));

	bt_le_adv_stop();
}

bool countDots(const char* array) {
	int count = 0;
	for (int i = 0; array[i] != '\0'; i++) {
		if (array[i] == '.') {
			count++;
		}
	}
	return count > 4;
}

bool updatePacket(const char* gps_string, const uint8_t* rx_data) {
	// if (countDots(gps_string)) {

		int start = 0;

		for (int i = 0; i < 85; i++) {
			if (rx_data[i] == 'G' && rx_data[i+1] == 'A') {
				start = i + 2;
				break;
			}
		}
		
		gpsIdentifier[0] = rx_data[start];
		gpsIdentifier[1] = rx_data[start+1];
		gpsIdentifier[2] = rx_data[start+2];
		gpsIdentifier[3] = rx_data[start+3];
		gpsIdentifier[4] = rx_data[start+4];
		gpsIdentifier[5] = rx_data[start+5];
		gpsIdentifier[6] = rx_data[start+6];
		gpsIdentifier[7] = rx_data[start+7];
		gpsIdentifier[8] = rx_data[start+8];
		gpsIdentifier[9] = rx_data[start+9];
		gpsIdentifier[10] = rx_data[start+10];
		gpsIdentifier[11] = rx_data[start+11];
		gpsIdentifier[12] = rx_data[start+12];
		gpsIdentifier[13] = rx_data[start+13];
		gpsIdentifier[14] = rx_data[start+14];
		gpsIdentifier[15] = rx_data[start+15];
		
		return true;
	// }
	// return false;
}


void main(void)
{
	const struct device *uart_dev;

    uart_dev = device_get_binding("UART_0");
    if (!uart_dev) {
        printk("Error: could not bind to device.\n");
        return;
    }

    struct uart_config uart_cfg = {
        .baudrate = 9600,
        .data_bits = UART_CFG_DATA_BITS_8,
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
    };

    // uart_configure(uart_dev, &uart_cfg);


	int pos = 0;
	char gps_string[HOLD_SIZE];

	int err;

	for (int i = 0; i < HOLD_SIZE; i++) {
		gps_string[i] = '!';
	}

	while(true) {

			uint8_t rx_data[100];
			uart_fifo_read(uart_dev, rx_data, 100);

			if (pos == 99) {
				
				if (updatePacket(gps_string, rx_data)) {
					
					err = bt_enable(bt_ready);	

					if (err) {
						printk("Bluetooth init failed (err %d)\n", err);
					}	
						
				}
			}
	}
}

