/*
 * bme280.c
 *
 *  Created on: May 1, 2024
 *      Author: yahya
 */

#include "bme280.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define filterSize 100


int currentIndex = 0;

static void bme280_getVals(bme280_struct_t* BME, uint32_t* ut, uint32_t* up, uint32_t* uh)
{
	uint8_t params[8];
	HAL_StatusTypeDef retVal = HAL_I2C_Mem_Read(BME->device_config.BME_I2C, BME280_ADD, BME280_STATUS, I2C_MEMADD_SIZE_8BIT, params, 1, 30);
	BME->flags.is_bme_updated_1 = 0;
	BME->flags.is_bme_updated_3 = 0;
	if((params[0] & 0x01) == 0x00)
	{
		retVal = HAL_I2C_Mem_Read(BME->device_config.BME_I2C, BME280_ADD, BME280_P_MSB_ADD, I2C_MEMADD_SIZE_8BIT, params, 8, 20);
		if (retVal == HAL_OK){
			BME->flags.is_bme_updated_1 = 1;
			BME->flags.is_bme_updated_3 = 1;
			*ut = 	((int32_t)params[3] << 12) | ((int32_t)params[4] << 4) | ((int32_t)params[5]  >> 4);
			*up =	((int32_t)params[0] << 12) | ((int32_t)params[1] << 4) | ((int32_t)params[2]  >> 4);
			*uh =	((int32_t)params[6] << 8) | ((int32_t)params[7]);
		}
	}
	UNUSED(retVal);
}

static void bme280_get_altitude(bme280_struct_t* BME)
{
	float p_seaLevel = 1013.25;		//hPa
	float alt = 44330.0 * (1.0 - pow((BME->datas.pressure / p_seaLevel), (1.0 / 5.255)));
	BME->datas.height = alt;
	BME->datas.altitude = alt - BME->parameters->base_alt;
	if(BME->datas.altitude > BME->parameters->max_alt && BME->parameters->base_alt != 0.0)
	{
		BME->parameters->max_alt = BME->datas.altitude;
	}
}
void bme280_config(bme280_struct_t* BME)
{
	uint8_t params[25];
	HAL_StatusTypeDef retVal;

	BME->parameters->base_alt = 0.0;
//	uint8_t resetData = BME280_SOFT_RESET;
//	retVal = HAL_I2C_Mem_Write(I2C_bme, BME280_ADD, BME280_RESET, I2C_MEMADD_SIZE_8BIT, &resetData, 1, 50);		//Soft Reset.
//	HAL_Delay(50);
	HAL_I2C_DeInit(BME->device_config.BME_I2C);
	HAL_Delay(5);
	HAL_I2C_Init(BME->device_config.BME_I2C);
	HAL_Delay(5);

	retVal = HAL_I2C_Mem_Read(BME->device_config.BME_I2C, BME280_ADD, BME280_PARAM1_START, I2C_MEMADD_SIZE_8BIT, params, 25, 200);
	BME->parameters->dig_T1 = params[0] | (uint16_t)(params[1] << 8);
	BME->parameters->dig_T2 = params[2] | ((int16_t)params[3] << 8);
	BME->parameters->dig_T3 = params[4] | ((int16_t)params[5] << 8);
	BME->parameters->dig_P1 = params[6] | ((uint16_t)params[7] << 8);
	BME->parameters->dig_P2 = params[8] | ((int16_t)params[9] << 8);
	BME->parameters->dig_P3 = params[10] | ((int16_t)params[11] << 8);
	BME->parameters->dig_P4 = params[12] | ((int16_t)params[13] << 8);
	BME->parameters->dig_P5 = params[14] | ((int16_t)params[15] << 8);
	BME->parameters->dig_P6 = params[16] | ((int16_t)params[17] << 8);
	BME->parameters->dig_P7 = params[18] | ((int16_t)params[19] << 8);
	BME->parameters->dig_P8 = params[20] | ((int16_t)params[21] << 8);
	BME->parameters->dig_P9 = params[22] | ((int16_t)params[23] << 8);
	BME->parameters->dig_H1 = params[24];

	retVal = HAL_I2C_Mem_Read(BME->device_config.BME_I2C, BME280_ADD, BME280_PARAM2_START, I2C_MEMADD_SIZE_8BIT, params, 7, 50);
	BME->parameters->dig_H2 = params[0] | ((int16_t)params[1] << 8);
	BME->parameters->dig_H3	= params[2];
	BME->parameters->dig_H4 = (params[4] & 0xF) | ((int16_t)params[3] << 4);
	BME->parameters->dig_H5 = ((params[4] & 0xF0) >> 4) | ((int16_t)params[5] << 4);
	BME->parameters->dig_H6 = params[6];

	uint8_t data_ctrl = 0;
	data_ctrl = BME->device_config.over_sampling;
	retVal = HAL_I2C_Mem_Write(BME->device_config.BME_I2C, BME280_ADD, BME280_CTRL_HUM, I2C_MEMADD_SIZE_8BIT, &data_ctrl, 1, 50);		//Humidity sensor over sampling set to OS.
	data_ctrl = 0;
	data_ctrl = BME->device_config.mode | (BME->device_config.over_sampling << 2) | (BME->device_config.over_sampling << 5);																		//Mode has been chosed.
	retVal = HAL_I2C_Mem_Write(BME->device_config.BME_I2C, BME280_ADD, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &data_ctrl, 1, 50);		//Temp and pressure sensors' over sampling set to OS.
	data_ctrl = 0;
	data_ctrl = (BME->device_config.filter << 2) | (BME->device_config.period << 5);
	retVal = HAL_I2C_Mem_Write(BME->device_config.BME_I2C, BME280_ADD, BME280_CONFIG, I2C_MEMADD_SIZE_8BIT, &data_ctrl, 1, 50);

	float base = 0.0;
	HAL_Delay(100);

	for(int i = 0; i < 30; i++)		//Taking base altitude
	{
	  bme280_update(BME);
	  base +=  BME->datas.altitude;
	  HAL_Delay(30);
	}
	BME->parameters->base_alt = (base / 30.0);
	BME->flags.is_bme_updated_2 = 1;
	bme280_update(BME);
	BME->parameters->max_alt = 0.0;

	UNUSED(retVal);
}

uint8_t bme280_init(bme280_struct_t* BME)
{
	uint8_t buf[1];
	if(BME->parameters == NULL)
	{
		BME->parameters = malloc(sizeof(*BME->parameters));
	}
	HAL_I2C_Mem_Read(BME->device_config.BME_I2C, BME280_ADD, BME280_ID, I2C_MEMADD_SIZE_8BIT, buf, 1, 20);

	if(*buf == 0x60)
		return 0;

	return 1;
}


void bme280_update(bme280_struct_t* BME){
	int32_t var1_t, var2_t, T, adc_T;
	uint32_t	ut, up, uh;

	bme280_getVals(BME, &ut, &up, &uh);

	if(BME->flags.is_bme_updated_1 == 1)
	{
		//For tempereature
		adc_T =	ut;
		var1_t = ((((adc_T >> 3 ) - ((int32_t)BME->parameters->dig_T1 << 1))) * ((int32_t)BME->parameters->dig_T2)) >> 11;
		var2_t = (((((adc_T >> 4) - ((int32_t)BME->parameters->dig_T1)) * ((adc_T >> 4) - ((int32_t)BME->parameters->dig_T1))) >> 12) * ((int32_t)BME->parameters->dig_T3)) >> 14;
		int32_t t_fine = var1_t + var2_t;
		T = (t_fine * 5 + 128) >> 8;
		BME->datas.temperature = (float)T / 100.0;

		//For pressure
		int64_t var1_p, var2_p, P, adc_P;
		adc_P = (int64_t)up;
		var1_p = ((int64_t)t_fine) - 128000;
		var2_p = var1_p * var1_p * (int64_t)BME->parameters->dig_P6;
		var2_p = var2_p + ((var1_p *(int64_t)BME->parameters->dig_P5) <<17);
		var2_p = var2_p + (((int64_t)BME->parameters->dig_P4) << 35);
		var1_p = ((var1_p * var1_p * (int64_t)BME->parameters->dig_P3) >> 8) + ((var1_p * (int64_t)BME->parameters->dig_P2) << 12);
		var1_p = (((((int64_t)1) <<47 ) + var1_p)) * ((int64_t) BME->parameters->dig_P1) >> 33;
		if(var1_p == 0)
		{
			P = 0;
		}else
		{
		P = 1048576 - adc_P;
		P = (((P << 31) - var2_p) * 3125) / var1_p;
		var1_p = (((int64_t) BME->parameters->dig_P9) * (P >> 13) * (P >> 13)) >> 25;
		var2_p = (((int64_t) BME->parameters->dig_P8) * P) >> 19;
		P = (( P + var1_p + var2_p) >> 8) + (((int64_t)BME->parameters->dig_P7) << 4);
		}

		BME->datas.pressure = ((float)P / 256.0 / 100.0);

		//for humidity
		uint32_t var_h, adc_H;
		adc_H = uh;

		var_h = (t_fine - ((int32_t)76800));
		var_h = (((((adc_H << 14) - (((int32_t)BME->parameters->dig_H4) << 20) - (((int32_t)BME->parameters->dig_H5) * var_h)) + ((int32_t)16384)) >> 15) * (((((((var_h *((int32_t)BME->parameters->dig_H6)) >> 10) * (((var_h * ((int32_t)BME->parameters->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)BME->parameters->dig_H2) + 8192) >> 14));
		var_h = (var_h - (((((var_h >> 15) * (var_h >> 15)) >> 7) * ((int32_t)BME->parameters->dig_H1)) >> 4));
		var_h = (var_h < 0 ? 0 : var_h);
		var_h = (var_h > 419430400 ? 419430400 : var_h);
		BME->datas.humidity = ((float)(var_h >> 12)) / 1024.0;

		// Get time of update.
		BME->datas.time_of_update = HAL_GetTick();

		//get altitude
		bme280_get_altitude(BME);
		BME->flags.is_bme_updated_1 = 0;
	}

}
/*
float moving_average(float newVal)
{
	float sum = 0.0;
	if (currentIndex == filterSize - 1)
		currentIndex = 0;
	array[currentIndex] = newVal;
	for(int i = 0; i < filterSize; i++)
	{
		sum += array[i];
	}
	sum /= (float)filterSize;
	currentIndex++;
	return sum;
}

*/




