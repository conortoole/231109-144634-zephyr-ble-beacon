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

uint8_t gpsIdentifier[] = {'!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!', '!'};
bool test = false;

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
        BT_DATA(BT_DATA_MANUFACTURER_DATA, temp, 16),
	};

	bt_addr_le_t addr = {0};
	size_t count2 = 1;

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");
	
	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	bt_id_get(&addr, &count2);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
}

void main(void)
{
	const struct device *uart_dev;
    uint8_t gps_data[256];
    int gps_data_index = 0;
	
	bool flag = true;

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

    while(flag) {

        uint8_t rx_data;
        if (uart_poll_in(uart_dev, &rx_data) == 0){
            
                if (gps_data_index < 256 - 1) {
					printk("%u", rx_data);
                    gps_data[gps_data_index++] = rx_data;
					gpsIdentifier[gps_data_index] = rx_data;
					
                } else {
                    // Buffer overflow, handle it accordingly
                    printk("GPS Data Buffer Overflow\n");
                    gps_data_index = 0; // Reset the index
                }
				
			if (gpsIdentifier[15] != '!'){
				flag = false;
			}
        }
    }
	test = true;
	int err;
	printk("Starting Beacon Demo\n");
	flag = true;
	/* Initialize the Bluetooth Subsystem */
	while(flag){
		err = bt_enable(bt_ready);	
	}
    
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	
}
