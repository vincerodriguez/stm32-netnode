#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

typedef struct {
    float    temp_f;
    float    humidity;
    float    pressure_hpa;
    float    gas_resistance;
    uint8_t  gas_valid;
    uint8_t  valid;          /* at least one reading done */
} SensorData_t;

extern SensorData_t g_sensor;

void Sensor_Init(void);
void Sensor_Poll(void);          /* call every main-loop iteration */
void Sensor_SetGasEnable(uint8_t enable);  /* 1 = heater on, 0 = off */

#endif /* SENSOR_DATA_H */
