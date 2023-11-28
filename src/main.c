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
}

bool countDots(const char* array) {
	int count = 0;
	for (int i = 0; array[i] != '\0'; i++) {
		if (array[i] == '.') {
			count++;
		}
	}
	return count == 6;
}

bool updatePacket(const char* gps_string) {
	if (countDots(gps_string)) {
		
		gpsIdentifier[0] = gps_string[7];
		gpsIdentifier[1] = gps_string[8];
		gpsIdentifier[2] = gps_string[9];
		gpsIdentifier[3] = gps_string[10];
		gpsIdentifier[4] = gps_string[11];
		gpsIdentifier[5] = gps_string[12];
		gpsIdentifier[6] = gps_string[13];
		gpsIdentifier[7] = gps_string[14];
		gpsIdentifier[8] = gps_string[15];
		gpsIdentifier[9] = gps_string[16];
		gpsIdentifier[10] = gps_string[17];
		gpsIdentifier[11] = gps_string[18];
		gpsIdentifier[12] = gps_string[19];
		gpsIdentifier[13] = gps_string[20];
		gpsIdentifier[14] = gps_string[21];
		gpsIdentifier[15] = gps_string[22];
		
		return true;
	}
	return false;
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

    uart_configure(uart_dev, &uart_cfg);

	int pos = 0;
	char gps_string[66];

	int err;
	printk("Starting Beacon Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);	

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}	

	while(true) {

			uint8_t rx_data;
			uart_poll_in(uart_dev, &rx_data);

			if (rx_data == '\n') {
				
				if (pos >= 16 && updatePacket(gps_string)) {
					
					err = bt_enable(bt_ready);	

					if (err) {
						printk("Bluetooth init failed (err %d)\n", err);
					}	
						
				}

				for (int i = 0; i < 66; i++) {
					gps_string[i] = '\0';
				}

				pos = 0;
			}
			else {
				gps_string[pos] = rx_data;	
			}

			pos++;
	}
}

