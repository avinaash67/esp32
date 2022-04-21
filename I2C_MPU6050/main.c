/* Simple I2C Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"

#define PIN_SDA 23
#define PIN_CLK 22

#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_PWR_MGMT_1   0x6B

#define I2C_ADDRESS 0x68 // I2C address register of MPU6050

/*
 * The following registers contain the primary data we are interested in
 * 0x3B MPU6050_ACCEL_XOUT_H
 * 0x3C MPU6050_ACCEL_XOUT_L
 * 0x3D MPU6050_ACCEL_YOUT_H
 * 0x3E MPU6050_ACCEL_YOUT_L
 * 0x3F MPU6050_ACCEL_ZOUT_H
 * 0x50 MPU6050_ACCEL_ZOUT_L
 * 0x41 MPU6050_TEMP_OUT_H
 * 0x42 MPU6050_TEMP_OUT_L
 * 0x43 MPU6050_GYRO_XOUT_H
 * 0x44 MPU6050_GYRO_XOUT_L
 * 0x45 MPU6050_GYRO_YOUT_H
 * 0x46 MPU6050_GYRO_YOUT_L
 * 0x47 MPU6050_GYRO_ZOUT_H
 * 0x48 MPU6050_GYRO_ZOUT_L
 */

void app_main(void)
{
/*  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html#i2c-api-configure-driver
 *
 * To establish I2C communication, start by configuring the driver.
 * This is done by setting the parameters of the structure i2c_config_t:
 *
 * Set I2C mode of operation - slave or master from i2c_mode_t
 * Configure communication pins
 *
 * Assign GPIO pins for SDA and SCL signals
 *
 * Set whether to enable ESP32’s internal pull-ups
 * (Master only) Set I2C clock speed
 *
 */
	printf("Setting Drivers for I2C\n");

    i2c_config_t conf; //i2c_config_t is a "struct" in i2c_types.h

    conf.mode = I2C_MODE_MASTER; //Setting mode of the esp as master
	conf.sda_io_num = PIN_SDA;   //Setting the pin of SDA
	conf.scl_io_num = PIN_CLK;   //Setting the pin of SCL
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;


	i2c_param_config(I2C_NUM_0, &conf); // I2C_NUM_0 = '0' sets the I2C protocol to its default value

/* As I2C is a master-centric bus, data can only go from
 * the slave to the master at the master’s request.
 * Therefore, the slave will usually have a send buffer where the
 * slave application writes data. The data remains in the send buffer to
 * be read by the master at the master’s own discretion.
 *
 */
	i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER, 0,0,0); //Installing the I2C driver in esp32

	printf("I2C Driver installed");

	i2c_cmd_handle_t cmd; //This creates a handle for the bit stream data
	vTaskDelay(200/portTICK_PERIOD_MS); //Simple Delay of 200 milliseconds

	// Setting MPU6050_PWR_MGMT_1 register
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, MPU6050_PWR_MGMT_1, 1);
	i2c_master_write_byte(cmd, 0, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

/* ESP32’s I2C controller operating as master is responsible for
 * establishing communication with I2C slave devices and sending
 * commands to trigger a slave to action, for example, to take a
 * measurement and send the readings back to the master.
 *
 * For better process organization, the driver provides a container, called a “command link”,
 *  that should be populated with a sequence of commands and then
 *  passed to the I2C controller for execution.
 */

	// Local Variables
	uint8_t data[6];
	short accel_x;

	while(1)
	{
		// Tell the MPU6050 to position the internal register pointer to register
		// MPU6050_ACCEL_XOUT_H.

		cmd = i2c_cmd_link_create(); // Creating a link
		i2c_master_start(cmd);// Populating first bit of the link with start bit (SDA goes from 1 to 0 in this case)
		i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1); //Setting the I2C register
		i2c_master_write_byte(cmd, MPU6050_ACCEL_XOUT_H, 1);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);


		cmd = i2c_cmd_link_create();

		// Populating cmd(i.e. cmd link) with the I2C protocol data
		i2c_master_start(cmd);  //appending Start bit to cmd
		i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1); //appending
		i2c_master_read_byte(cmd, data,   0);
		i2c_master_read_byte(cmd, data+1, 0);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		accel_x = (data[0] << 8) | data[1];
		printf("accel_x: %d\n",accel_x);
		vTaskDelay(500/portTICK_PERIOD_MS);

	}

}
