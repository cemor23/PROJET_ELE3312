/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
	


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"

#include "ili9341.h"
#include "ili9341_gfx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MPU6050_ADDR 0x68<<1
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define CONFIG_REG 0x1A
#define INT_ENABLE_REG 0x38
#define ACCEL_XOUT_H_REG 0x3B

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ili9341_t *_screen;
float Ax;
float Ay;
float Az;
volatile int tag_started = 0;
uint8_t Rec_Data[6];
volatile int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
volatile int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;
char buf[100];
HAL_StatusTypeDef status;
uint8_t data;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	UNUSED(GPIO_Pin);
	if ((GPIO_Pin == GPIO_PIN_11) & tag_started) {
		Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data [1]);
		Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data [3]);
		Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data [5]);
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

void MPU6050_Read_Accel (void)
{
	HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 6);
	
	Ax = Accel_X_RAW*100/16384.0;
	Ay = Accel_Y_RAW*100/16384.0;
	Az = Accel_Z_RAW*100/16384.0;
}

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_DAC_Init();
  MX_I2C1_Init();
	HAL_I2C_DeInit(&hi2c1);
	__HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_BERR);  // Bus error
	__HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ARLO);  // Arbitration lost
	__HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_OVR);   // Overrun/Underrun error
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	_screen = ili9341_new(
		&hspi1,
		Void_Display_Reset_GPIO_Port, Void_Display_Reset_Pin,
		TFT_CS_GPIO_Port, TFT_CS_Pin,
		TFT_DC_GPIO_Port, TFT_DC_Pin,
		isoLandscape,
		NULL, NULL,
		NULL, NULL,
		itsNotSupported,
		itnNormalized);
		
	ili9341_fill_screen(_screen, ILI9341_BLACK);
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18, ILI9341_WHITE, ILI9341_BLACK,0,0};
	
	HAL_TIM_Base_Start(&htim2);
	AccelInnit();
	tag_started = 1;
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		MPU6050_Read_Accel();
		sprintf(buf,"x: %f g\r\ny: %f g\r\nz: %f g", Ax, Ay, Az);
		ili9341_draw_string(_screen, text_attr, buf);
		HAL_Delay(100);
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
 /* Place your implementation of fputc here */ 
 /* e.g. write a character to the USART2 and Loop until the end of transmission */ 
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF); 
	return ch; 
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
