/*
 * Game.h
 *
 *  Created on: Oct 17, 2024
 *      Author: joerg
 */

#ifndef INC_GAME_H_
#define INC_GAME_H_

#include "ili9341.h"
#include "ili9341_gfx.h"

#include "Character.h"
#include "usart.h"

#define SCREEN_X 320
#define SCREEN_Y 240
#define SCREEN_CENTER_X 160
#define SCREEN_CENTER_Y 120
#define MAX_TILE_X SCREEN_X/10
#define MAX_TILE_Y SCREEN_Y/10
#define STEP_SIZE 2

#define NUM_PLAYERS 2
#define LOCAL_PLAYER_ID 0
#define ENEMY_PLAYER_ID 1

// accelerometre
#define MPU6050_ADDR 0x68<<1
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define CONFIG_REG 0x1A
#define INT_ENABLE_REG 0x38
#define ACCEL_XOUT_H_REG 0x3B
// accelerometre

// uart
#define UART_BUFFER_SIZE 8
// uart

// son
#define pi 3.14
#define TABLE_LENGTH 1000
// son

typedef enum {
	CHOOSE_PLAYER, INIT_MAZE, WANDER_MAZE, BATTLE
} game_state_t;

typedef struct {
	uint8_t is_x_ok;
	uint8_t is_y_ok;
	uint8_t is_enemy;
} boundary_check_t;

typedef struct {
	 uint16_t width;
	 uint16_t height;
	 int id;
	 uint16_t *data;
} sprite_metadata_t;

sprite_metadata_t *getSprite(uint16_t id);
sprite_metadata_t * getFigureSprite(position_t delta, character_type_t type);
void drawBitmap(ili9341_t *lcd, uint16_t *data, position_t pos, uint16_t width, uint16_t height);
void drawMaze(ili9341_t *lcd, player_t *players);

uint16_t getTileCoord(int16_t x, int16_t y);
void checkBoundary(int16_t x, int16_t y, player_t *players, uint16_t *maze, boundary_check_t *bc);
uint8_t updatePosition(ili9341_t *lcd, position_t new_pos, player_t *players);
void drawRemotePlayer(ili9341_t *lcd, player_t *player);

// accelerometre
extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
extern void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
extern float Ax, Ay, Az;
extern volatile char flag_i2c, flag_exti;
extern I2C_HandleTypeDef hi2c1;
extern uint8_t Rec_Data[6];
// accelerometre

// uart
extern uint8_t RxData_temp;
extern volatile int state;
extern int RxData_i;
typedef union TxDataUnion {
	struct {
		char FFByte, sizeByte, var1, var2, var3, var4, var5, var6;
	} TxDataStruct;
	char TxDataArray[UART_BUFFER_SIZE];
} TxDataUnion;
typedef union RxDataUnion {
	struct {
		char var1, var2, var3, var4, var5, var6;
	} RxDataStruct;
	char RxDataArray[UART_BUFFER_SIZE-2];
} RxDataUnion;
extern RxDataUnion RxData;
extern TxDataUnion TxData;
// uart

//son
extern volatile int timerMesure;
extern volatile int flagMesure;
extern volatile int distance; 
extern void trigger (void);
extern void JouerNote(int dist);
// son

// timer
extern volatile int flag_timer_4;
// timer

#endif /* INC_GAME_H_ */
