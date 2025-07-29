#ifndef BME280
#define BME280
#include <stdint.h>
#include <main.h>

#define BME280_ADD			(0x76 << 1)
#define BME280_PARAM1_START	0x88		//First parameters starting address (total 25 bytes).
#define BME280_PARAM2_START	0xE1		//Second parameters starting address (total 7 bytes).
#define BME280_TEMP_ADD		0xFA
#define BME280_P_MSB_ADD	0xF7
#define BME280_STATUS		0xF3
#define BME280_CTRL_MEAS	0xF4
#define BME280_CONFIG		0xF5
#define BME280_CTRL_HUM		0xF2
#define BME280_STATUS		0xF3
#define BME280_RESET		0xE0
#define BME280_ID			0xD0

#define BME280_SOFT_RESET	0xB6

#define BME280_MODE_NORMAL	0x03
#define BME280_MODE_FORCED	0x01
#define BME280_MODE_SLEEP	0x00

#define BME280_OS_NOOS		0x00	//Oversampling of the measurements = 0
#define BME280_OS_1			0x01	//Oversampling of the measurements = 1
#define BME280_OS_2			0x02	//Oversampling of the measurements = 2
#define BME280_OS_4			0x03	//Oversampling of the measurements = 4
#define BME280_OS_8			0x04	//Oversampling of the measurements = 8
#define BME280_OS_16		0x05	//Oversampling of the measurements = 16

#define BME280_STBY_05		0x00	//Standby time = 0.5ms
#define BME280_STBY_62_5	0x01	//Standby time = 62.5ms
#define BME280_STBY_125		0x02	//Standby time = 125ms
#define BME280_STBY_250		0x03	//Standby time = 250ms
#define BME280_STBY_500		0x04	//Standby time = 500ms
#define BME280_STBY_1000	0x05	//Standby time = 1000ms
#define BME280_STBY_10		0x06	//Standby time = 10ms
#define BME280_STBY_20		0x07	//Standby time = 20ms

#define BME280_FILTER_OFF	0x00
#define BME280_FILTER_2		0x01
#define BME280_FILTER_4		0x02
#define BME280_FILTER_8		0x03
#define BME280_FILTER_16	0x04

typedef struct bme_parameters{
	uint16_t 	dig_T1;
	int16_t 	dig_T2;
	int16_t 	dig_T3;
	uint16_t 	dig_P1;
	int16_t 	dig_P2;
	int16_t 	dig_P3;
	int16_t 	dig_P4;
	int16_t 	dig_P5;
	int16_t 	dig_P6;
	int16_t 	dig_P7;
	int16_t 	dig_P8;
	int16_t 	dig_P9;
	uint8_t		dig_H1;
	int16_t		dig_H2;
	uint8_t		dig_H3;
	int16_t		dig_H4;
	int16_t		dig_H5;
	int8_t		dig_H6;
	float 		base_alt;
	float		max_alt;
}bme_parameters_t;

typedef struct bme280_conf
{
	uint8_t		bme280_mode;
	uint8_t		bme280_output_speed;
	uint8_t		bme280_filter;
	I2C_HandleTypeDef	*BME_I2C;
}bme280_conf_t;

typedef struct bme280_flags
{
	uint8_t is_bme_updated_1;
	uint8_t is_bme_updated_2;
	uint8_t is_bme_updated_3;
	uint8_t is_bme_updated_4;
}bme280_flags_t;

typedef struct bme280_datas
{
	float temperature;	//(C)
	float pressure;		//(mBar)
	float humidity;		//percentage (%)
	float height;		//height form the sea level in (m)
	float altitude;		//height form the base level in (m)
	float velocity;		//(m/s)
}bme280_datas_t;

typedef struct bme280_struct
{
	bme_parameters_t 	*parameters;
	bme280_conf_t		device_config;
	bme280_datas_t		datas;
	bme280_flags_t		flags;
}bme280_struct_t;

uint8_t bme280_init(bme280_struct_t* BME);
void bme280_config(bme280_struct_t* BME);
void bme280_update(bme280_struct_t* BME);
//float moving_average(float newVal);
#endif
