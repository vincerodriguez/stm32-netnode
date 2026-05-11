/*
 * bme68x_driver.h
 *
 *  Created on: Apr 24, 2026
 *      Author: Vince's PC
 */

#ifndef BME68X_DRIVER_H
#define BME68X_DRIVER_H

#include "bme68x.h"

int8_t BME688_Init(void);
int8_t BME688_SetGasEnable(uint8_t enable);   /* 1 = heater on, 0 = off */
int8_t BME688_ReadForced(struct bme68x_data *data);

// Heater profile helpers (used in Step 2)
int8_t BME688_ConfigParallelHeater(void);
int8_t BME688_ReadParallel(struct bme68x_data *data, uint8_t *n_fields);

#endif
