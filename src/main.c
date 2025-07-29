#include "main.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "bme280.h"

typedef struct bckp_sram_datas
{
	bme_parameters_t	bme_params;
}bckp_sram_datas_t;

#define PRESSURE_I2C_HNDLR	hi2c1
#define TTL_HNDLR			huart1

static bme280_struct_t bme_sensor_s;
bckp_sram_datas_t* backup_datas = (bckp_sram_datas_t*) BKPSRAM_BASE;
uint8_t str[200];

uint8_t bme280_begin();
void serial_println(char* str);


int main(void)
{
  HAL_PWR_EnableBkUpAccess();
  RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
  HAL_PWR_EnableBkUpReg();

  HAL_TIM_Base_Start_IT(&htim2);

  uint8_t bme_ret = bme280_begin();
  if(bme_ret)
  {
	  serial_println("bme sensor fail");
  }
  else
  {
	  serial_println("bme sensor success");
  }

  bme280_config(&bme_sensor_s);

  while (1)
  {
    	if(bme_sensor_s.flags.is_bme_updated_2)
    	{
    		bme280_update(&bme_sensor_s);
    		sprintf((char*)str, "alt: %f \t hum: %f \t temp: %f", bme_sensor_s.datas.altitude, bme_sensor_s.datas.humidity, bme_sensor_s.datas.temperature);
    		serial_println((char*)str);
    	}
      HAL_Delay(200);
  }
}

uint8_t bme280_begin()
{
	bme_sensor_s.device_config.bme280_filter = BME280_FILTER_8;
	bme_sensor_s.device_config.bme280_mode = BME280_MODE_NORMAL;
	bme_sensor_s.device_config.bme280_output_speed = BME280_OS_8;
	bme_sensor_s.device_config.BME_I2C = &PRESSURE_I2C_HNDLR;
	bme_sensor_s.parameters = &backup_datas->bme_params; //if no backup data, write NULL 
	return bme280_init(&bme_sensor_s);
}

void serial_println(char* str)
{

	HAL_UART_Transmit(&TTL_HNDLR, (uint8_t*)str, strlen(str), 50);
	HAL_UART_Transmit(&TTL_HNDLR, (uint8_t*)"\r\n", 2, 50);
}