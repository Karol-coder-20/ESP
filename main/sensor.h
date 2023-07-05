/*
 * sensor.h
 *
 *  Created on: 30 cze 2023
 *      Author: kkohut
 */

#ifndef MAIN_SENSOR_H_
#define MAIN_SENSOR_H_

#include "vl53l1_api.h"

typedef struct {
    int interrupt_pin;
    int power_pin;
    int xshut_pin;
} Sensor_pins_t;

VL53L1_Error sensor_init(int sensor_index, Sensor_pins_t pins, int i2c_address);

int16_t get_distance(int sensor_index);


#endif /* MAIN_SENSOR_H_ */
