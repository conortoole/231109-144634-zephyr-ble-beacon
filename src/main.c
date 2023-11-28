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
/*
 * Set Advertisement data. Based on the Eddystone specification:
 * https://github.com/google/eddystone/blob/master/protocol-specification.md
 * https://github.com/google/eddystone/tree/master/eddystone-url
 */

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void bt_ready(int err)
{
	uint8_t temp[16];
	// temp1[10];
	//uint8_t temp2[10];
	int count = 0;

	for (int i = 0; i < 16; i++){
		temp[count] = gpsIdentifier[i];
		count++;
	}

	char addr_s[BT_ADDR_LE_STR_LEN];
	struct bt_data ad[] = {
		BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
        BT_DATA(BT_DATA_MANUFACTURER_DATA, temp, 16),
		//BT_DATA(BT_DATA_MANUFACTURER_DATA, temp1, 10),
		//BT_DATA(BT_DATA_MANUFACTURER_DATA, temp2, 10),
	};

	bt_addr_le_t addr = {0};
	size_t count2 = 1;

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");
	//setup_advertisement_data(ad, gpsIdentifier);
	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}


	/* For connectable advertising you would use
	 * bt_le_oob_get_local().  For non-connectable non-identity
	 * advertising an non-resolvable private address is used;
	 * there is no API to retrieve that.
	 */

	bt_id_get(&addr, &count2);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
}

void main(void)
{
	const struct device *uart_dev;
    uint8_t gps_data[256];
    int gps_data_index = 0;
	int j = 0;
	int k = 0;
    //int longitude = 0;
    //int latitude = 0;
	//int count = 0;
	bool flag = true;
	bool foundLat = false;
	bool foundLong = false;

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
            if (foundLat){
                // Store the received character in the buffer
                if (gps_data_index < 256 - 1) {
                    gps_data[gps_data_index++] = rx_data;
					gpsIdentifier[gps_data_index] = rx_data;
					
                } else {
                    // Buffer overflow, handle it accordingly
                    printk("GPS Data Buffer Overflow\n");
                    gps_data_index = 0; // Reset the index
                }
				
				if (j > 6){
					foundLat = false;
				}
				j++;
			}

			if (foundLong){
                // Store the received character in the buffer
                if (gps_data_index < 256 - 1) {
                    gps_data[gps_data_index++] = rx_data;
					gpsIdentifier[gps_data_index] = rx_data;
					
                } else {
                    // Buffer overflow, handle it accordingly
                    printk("GPS Data Buffer Overflow\n");
                    gps_data_index = 0; // Reset the index
                }
				
				if (k > 6){
					foundLong = false;
				}
				k++;
			}

            if (rx_data == 'A') {
				foundLat = true;
			}

			if (rx_data == 'N') {
				foundLong = true;
			}

			if (gpsIdentifier[15] != '!'){
				flag = false;
			}
        }
       // k_sleep(K_MSEC(50));  // Sleep for 1 second (adjust as needed)
		// if (count > 15){
		// 	flag = false;
		// }
    }
	test = true;
	int err;
	printk("Starting Beacon Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);	

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	
}
