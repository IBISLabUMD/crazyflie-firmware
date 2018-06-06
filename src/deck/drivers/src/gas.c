/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2012 BitCraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define DEBUG_MODULE "GasDesk"
#include "FreeRTOS.h"
#include "task.h"

#include "deck.h"
#include "system.h"
#include "debug.h"
#include "log.h"
#include "param.h"

#include "i2cdev.h"
#include "gas.h"

#include "stabilizer_types.h"

/*  16 bit gas integer */
static uint16_t gas = 1000;

static I2C_Dev *I2Cx;
static bool isInit;
static uint16_t measurement_timing_budget_ms;

void gasInit(DeckInfo* info)
{
  if (isInit)
    return;

  i2cdevInit(I2C1_DEV);
  I2Cx = I2C1_DEV;
  DEBUG_PRINT("TESTING GAS TASK");
  xTaskCreate(gasTask, GAS_TASK_NAME, GAS_TASK_STACK_SIZE, NULL, GAS_TASK_PRI, NULL);

  isInit = true;

  measurement_timing_budget_ms = 10;
}

bool gasTest(void)
{
  if (!isInit)
    return false;
  return true;
}

void gasTask(void* arg)
{
  systemWaitStart();
  TickType_t xLastWakeTime;

  /*My buffer*/
  uint8_t gas_arr[3] = {};                                          
  /* Loop that was here before */
  while (1) {
	 
    xLastWakeTime = xTaskGetTickCount();
	/* My code */
	/* gas sensor writing config */
	i2cdevWriteByte(I2Cx, 0x68,0xD0, 0x80);
	/* Gas sensor reading data to buffer */
	i2cdevRead(I2Cx, 0x68, 0xD0, 3,  (uint8_t *)&gas_arr);

	/* parsing bytes to get our 12 bits of sensor data */
	gas = ((uint16_t)((((gas_arr[0] & 0x0F) << 8) |( (gas_arr[1])))));                 

	/*  code that was here before continues */
    xLastWakeTime = xTaskGetTickCount();
	/*  const TickType_t xDelay=5 / portTICK_PERIOD_MS;
	vTaskDelay(xDelay);*/
    vTaskDelayUntil(&xLastWakeTime, M2T(measurement_timing_budget_ms));
  }
}

static const DeckDriver gas_deck = {
  .vid = 0xBC, /* change this */
  .pid = 0x09, /* change this */
  .name = "gas_sensor",
  .usedGpio = 0x0C, /* change this */

  .init = gasInit,
  .test = gasTest,
};

DECK_DRIVER(gas_deck);

PARAM_GROUP_START(deck)
PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, gas_sensor, &isInit)
PARAM_GROUP_STOP(deck)

LOG_GROUP_START(Sensor)
LOG_ADD(LOG_UINT16, gas, &gas)
/*  LOG_ADD(LOG_UINT8, gas0, &gas0)
LOG_ADD(LOG_UINT8, gas1, &gas1)
LOG_ADD(LOG_UINT8, gas2, &gas2)
*/
LOG_GROUP_STOP(Sensor)
