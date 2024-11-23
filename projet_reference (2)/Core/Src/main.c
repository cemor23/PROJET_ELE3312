/* USER CODE BEGIN Header */
/** 
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program for the ELE3312 project
	* Authors         :
	* Group           :
	*
  ******************************************************************************
	*
  * 
  * This file contains the main logic of the game.
  * 
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "ili9341_gfx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "Graphics.h"
#include "Menu.h"
#include "Game.h"
#include "Battle.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//accelerometre
#define MPU6050_ADDR 0x68<<1
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define CONFIG_REG 0x1A
#define INT_ENABLE_REG 0x38
#define ACCEL_XOUT_H_REG 0x3B
//accelerometre
//uart
#define UART_BUFFER_SIZE 4
//uart
//son
#define pi 3.14
#define TABLE_LENGTH 1000
//son

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// accelerometre
float Ax;
float Ay;
float Az;
volatile int tag_started = 0;
uint8_t Rec_Data[6];
volatile int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
volatile int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;
volatile int flagPIN11 = 1;
volatile int flag_done = 1;
char buf[100];
HAL_StatusTypeDef status;
uint8_t data;
//accelerometre

//uart
uint8_t RxData_temp;
volatile int state =0;
int RxData_i = 0;
char RxData[UART_BUFFER_SIZE];

union TxDataUnion {
    struct {
        char FFByte, sizeByte, xpos, ypos, etcVar;
    } TxDataStruct;
    char TxDataArray[UART_BUFFER_SIZE];
} TxData;
//uart

//son
uint32_t tab_value0[TABLE_LENGTH];
uint32_t tab_value1[TABLE_LENGTH];
volatile uint32_t ic_val1 = 0;
volatile uint32_t ic_val2 = 0;
volatile uint8_t is_first_capture = 0; // Drapeau pour savoir quel front est capturé
volatile float distance = 0.0;
volatile int timerMesure = 0;
volatile char flagMesure =0;
volatile float value;
volatile int flag_tableau =0;
enum note {B3,C4,D4,D4s,E4,F4s,G4,A4,B4,C5,D5,D5s,E5,F5s,G5};
//






ili9341_t *_screen;
player_t players[NUM_PLAYERS] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

//accelerometre
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	UNUSED(GPIO_Pin);
	if ((GPIO_Pin == GPIO_PIN_11) & tag_started) {
		flagPIN11 = 1;
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	UNUSED(hi2c);
	if (hi2c->Instance == I2C1) {
		flag_done = 1;
		
		Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data [1]);
		Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data [3]);
		Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data [5]);
		Ax = Accel_X_RAW*100/16384.0;
		Ay = Accel_Y_RAW*100/16384.0;
		Az = Accel_Z_RAW*100/16384.0;
	}
}

void AccelInnit(void) {
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18,ILI9341_WHITE,	ILI9341_BLACK,0,0};
	
	// HAL_I2C_IsDeviceReady
	status = HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR, 10, 100);
	while (status!=HAL_OK){
		status = HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR, 1000, 1000);
		if (status == HAL_BUSY) sprintf(buf,"I2C busy... %#04x", status);
		else if (status == HAL_ERROR) sprintf(buf,"I2C Error... %#04x", status);
		else sprintf(buf,"status: %#04x", status);
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	}
	
	// WHO_AM_I
	uint8_t data_WhoAmI;
	status = HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &data_WhoAmI, 1);
	while	(status != HAL_OK) {
		status = HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &data_WhoAmI, 1);
		sprintf(buf,"CHECKING ACCELEROMETER CONNECTION...");
		text_attr.origin_y = 20;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	};
	
	sprintf(buf,"ACCELEROMETER CONNECTED!");
	ili9341_draw_string(_screen, text_attr, buf);
	
	
	// RESET SLEEP MODE, SELECT INTERNAL CLOCK
	data = 0;
	status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data, 1);
	while (status != HAL_OK) {
		status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data, 1);
		sprintf(buf,"WHO_AM_I...");
		text_attr.origin_y = 20;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	};
	
	sprintf(buf,"WHO_AM_I -> %u", data_WhoAmI);
	text_attr.origin_y = 20;
	ili9341_draw_string(_screen, text_attr, buf);
	
	
	// FS_SEL = ±250°/s, DESACTIVER SELFTEST
	data = 0;
	status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1);
	while (status != HAL_OK) {
		status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1);
		sprintf(buf,"PWR_MGMT_1...");
		text_attr.origin_y = 40;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	}; 
	sprintf(buf,"PWR_MGMT_1 : OK!");
	text_attr.origin_y = 40;
	ili9341_draw_string(_screen, text_attr, buf);

	// AFS_SEL = ±2g, DESACTIVER SELFTEST
	data = 0;
	status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1);
	while (status != HAL_OK) {
		status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1);
		sprintf(buf,"GYRO_CONFIG...");
		text_attr.origin_y = 60;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	}; 
	sprintf(buf,"GYRO_CONFIG : OK!");
	text_attr.origin_y = 60;
	ili9341_draw_string(_screen, text_attr, buf);
	
	// Bandwidth = 94
	data = 2;
	status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, CONFIG_REG, 1, &data, 1);
	while (status != HAL_OK) {
		status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, CONFIG_REG, 1, &data, 1);
		sprintf(buf,"ACCEL_CONFIG...");
		text_attr.origin_y = 80;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	};
	sprintf(buf,"ACCEL_CONFIG : OK!");
	text_attr.origin_y = 80;
	ili9341_draw_string(_screen, text_attr, buf);
	
	// DATA_RDY_EN = 1
	data = 1;
	status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, INT_ENABLE_REG, 1, &data, 1);
	while (status != HAL_OK) {
		status = HAL_I2C_Mem_Write_DMA(&hi2c1, MPU6050_ADDR, INT_ENABLE_REG, 1, &data, 1);
		sprintf(buf,"CONFIG...");
		text_attr.origin_y = 100;
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(10);
	}; 
	sprintf(buf,"CONFIG : OK!");
	text_attr.origin_y = 100;
	ili9341_draw_string(_screen, text_attr, buf);
	
	
	sprintf(buf,"INT_ENABLE : OK!");
	text_attr.origin_y = 120;
	ili9341_draw_string(_screen, text_attr, buf);
	
	HAL_Delay(2000);
	ili9341_fill_screen(_screen, ILI9341_BLACK);
	text_attr.origin_y = 0;
	
};
//accelerometre

//uart
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if (huart == &huart5){
		if (state == 0){
			if (RxData_temp == 0xFF) state = 1;
		}
		else if (state ==1){
			if (RxData_temp <= UART_BUFFER_SIZE) state = 2;
			else state = 0;
		}
		else {
			RxData[RxData_i] = RxData_temp;
			RxData_i++;
			if (RxData_i>=UART_BUFFER_SIZE){ 
				RxData_i = 0;
				state = 0;
			}
		}
		HAL_UART_Receive_IT(&huart5, &RxData_temp, 1);
	}
}
//uart

//son
void HAL_DelayMicroseconds(uint32_t us) {
    uint32_t start = DWT->CYCCNT;  // Current cycle count
    uint32_t ticks = (HAL_RCC_GetHCLKFreq() / 1000000) * us;  // Cycles for 'us' microseconds

    while ((DWT->CYCCNT - start) < ticks);  // Wait until the elapsed cycles reach the target
}


void trigger (void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_DelayMicroseconds(10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}
	
	

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	// Vérifie que l'interruption vient du bon canal
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {

		if (is_first_capture == 0) {
			// Capture du front montant
			ic_val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // Lit la première valeur de capture
			is_first_capture = 1;                                     // Marque que le front montant a été capturé
		}
		else if (is_first_capture == 1) {
			// Capture du front descendant
			ic_val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // Lit la seconde valeur de capture
			__HAL_TIM_SET_COUNTER(htim, 0);                           // Réinitialise le compteur pour la prochaine mesure

			// Calcul de la durée d'impulsion
			uint32_t diff = (ic_val2 > ic_val1) ? (ic_val2 - ic_val1) : ((0xFFFF - ic_val1) + ic_val2 + 1);
			distance = (float)(diff * 0.0343) / 2; // Convertit la durée en distance (en cm)
			
			
			is_first_capture = 0; // Réinitialise pour la prochaine mesure
		}
	}
}

void HAL_SYSTICK_Callback(void) {
	if	(timerMesure++ ==100)
	{timerMesure=0;
	flagMesure = 1;}
}

void JouerNote(float dist) {
	

	
	int note = 0;
	
if (3<dist && dist<=6){
	note = (int)(40000/789.99f);
}

else if (6<dist && dist<=9){
	note = (int)(40000/698.46f);
}

else if (9<dist && dist<=12){
	note = (int)(40000/659.26f);
}

else if (12<dist && dist<=15){
	note = (int)(40000/622.25f);
}

else if (15<dist && dist<=18){
	note = (int)(40000/587.33f);
}

else if (18<dist && dist<=21){
	note = (int)(40000/523.25f);
}

else if (21<dist && dist<=24){
	note = (int)(40000/493.88f);
}

else if (24<dist && dist<=27){
	note = (int)(40000/440.0f);
}

else if (27<dist && dist<=30){
	note = (int)(40000/392.00f);
}

else if (30<dist && dist<=33){
	note = (int)(40000/369.99f);
}

else if (33<dist && dist<=36){
	note = (int)(40000/329.63f);
}

else if (36<dist && dist<=39){
	note = (int)(40000/311.13f);
}

else if (39<dist && dist<=42){
	note = (int)(40000/293.66f);
}

else if (42<dist && dist<=45){
	note = (int)(40000/261.63f);
}

else if (45<dist){
	note = (int)(40000/246.94f);
}

if (flag_tableau == 0){		
    for (int i = 0; i<TABLE_LENGTH; i++) {
        if(i% note < (note / 2))
				{
            value = 4000;
            tab_value0[i]= value;
        }

        else{
            value = 0;
            tab_value0[i]=value;
        }

}		

if (flag_tableau == 1){
    for (int i = 0; i<TABLE_LENGTH; i++) {
        if(i% note < (note / 2))
				{
            value = 4000;
            tab_value1[i]=value;
        }

        else{
            value = 0;
            tab_value1[i]=value;
        }
			}
    }
	}
}


//son
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	HAL_Delay(500);
	/* Default players setting */
	static game_state_t game_state = CHOOSE_PLAYER;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	//son
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
					CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // Enable DWT
			}
			DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;  // Enable the cycle counter
			DWT->CYCCNT = 0;  // Reset the cycle counter
	//son
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_UART5_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	
	// Initialize the screen
	_screen = ili9341_new(
		  &hspi1,
		  Void_Display_Reset_GPIO_Port, Void_Display_Reset_Pin,
		  TFT_CS_GPIO_Port, TFT_CS_Pin,
		  TFT_DC_GPIO_Port, TFT_DC_Pin,
		  isoLandscapeFlip,
		  NULL, NULL,
		  NULL, NULL,
		  itsNotSupported,
		  itnNormalized);
	char text[40];
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18,  ILI9341_WHITE, ILI9341_BLACK, 10, 0};
	ili9341_fill_screen(_screen, ILI9341_BLACK);
	
	//son
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim5);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	for (int i=0;i<TABLE_LENGTH;i++) 
		{
				value = 0;
				tab_value0[i]=(int) value;
		}
	
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, tab_value0,TABLE_LENGTH, DAC_ALIGN_12B_R);
	//son
	//accelerometre
	
	//AccelInnit();
	tag_started = 1;
	//accelerometre
	//uart
	TxData.TxDataStruct.FFByte = 0xFF;
	TxData.TxDataStruct.sizeByte = UART_BUFFER_SIZE;
	HAL_UART_Transmit_DMA(&huart5, TxData.TxDataArray, UART_BUFFER_SIZE);
	HAL_UART_Receive_IT(&huart5, &RxData_temp, 1);
	//uart
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	player_t* player = &players[LOCAL_PLAYER_ID];
	player_t* enemy = &players[ENEMY_PLAYER_ID];
	
	int16_t x = 0, y = 0;
	
	/* Infinite loop */
  while (1)
  {
		//uart
		//HAL_UART_Transmit_DMA(&huart5, TxData.TxDataArray, UART_BUFFER_SIZE);
		//uart
		
		//son
		
		while(1){
		trigger();
	if(flagMesure ==1) {
		char buffer[20] = {0};	
			sprintf(buffer,"%f", distance);
			JouerNote(distance);
			ili9341_text_attr_t time_attr = {&ili9341_font_11x18,
			ILI9341_WHITE, ILI9341_BLACK,0,0};
			ili9341_draw_string(_screen, time_attr,buffer);
			flagMesure=0;
	}
		}
		//son
		
		
		
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		//accelerometre
		if (flagPIN11 & flag_done) {
			flagPIN11 = 0;
			flag_done = 0;
			HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 6);
		}
		//accelerometre
		
		
		
		
		
		HAL_Delay(20); // � remplacer avec un timer
		
		switch(game_state) {
		case CHOOSE_PLAYER:
			choosePlayer(_screen, players);
			game_state = INIT_MAZE;
			break;
		
		case INIT_MAZE:
			player->current_pos = player->start_pos;
			enemy->current_pos = enemy->start_pos;
			drawMaze(_screen, players);			
			game_state = WANDER_MAZE;
			break;
			
		case WANDER_MAZE:
			// Obtenir la nouvelle position d�sir�e
			x = player->current_pos.x;
			y = player->current_pos.y + 0.5 * STEP_SIZE;
			// Obtenir et mettre à jour la position de l'adversaire
			// ...
			// Vérifier la rencontre avec l'adversaire
			if(updatePosition(_screen, (position_t){x, y}, players)){
				game_state = BATTLE;
				continue;
			}
			break;
			
		case BATTLE:
			battle(_screen, players);
			game_state = INIT_MAZE;
			break;
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
// Ici on alterne entre les deux tableaux.
	if (flag_tableau == 0) {
		HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, tab_value0, TABLE_LENGTH, DAC_ALIGN_12B_R);
		
					
	} else {
		HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, tab_value1, TABLE_LENGTH, DAC_ALIGN_12B_R);
	   
					
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
