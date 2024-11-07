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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ili9341_t *_screen;
float xx = 350;
float yy = 1450;
int zz = 500;
int freq = 40000;
int temps;
int curTime = 0;
#define TABLE_LENGTH 10000
uint32_t tab_value1[TABLE_LENGTH];
uint32_t tab_value2[TABLE_LENGTH];
double gain;
volatile int flag_done = 0;
volatile int flag_end = 0;
volatile int flag_table = 1;
volatile int flag_high = 0;

#define TABLE_LENGTH_AMP 1000
uint32_t tab_valueAmp[TABLE_LENGTH_AMP];
float k = 0.7; // amplitude du vibrato
float ampIndex = 0;
float A = 0;

//#define TABLE_LENGTH_s 10000
//uint32_t tab_value_s[TABLE_LENGTH_s];
//float k = 0.7;
//float value_S = 100;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	

	for (int i=0;i<TABLE_LENGTH_AMP;i++) {
		float value_s = 1.+k*sin(3.1415*(((float)i)/(500)));
		tab_valueAmp[i]= value_s * 1000;
	}

	HAL_TIM_Base_Start(&htim2);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, tab_value1, TABLE_LENGTH, DAC_ALIGN_12B_R);
	gain = pow ((double)(yy/xx), (double)zz/6000);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		while (flag_end == 0){};
		flag_end = 0;
		temps++;
		curTime = temps*(TABLE_LENGTH*1000/freq);
		
		if (curTime%zz == 0){
			if (curTime <= 6000){
				xx *= gain;
				for (int i=0;i<TABLE_LENGTH;i++) {
					A = tab_valueAmp[(int)floor((double)ampIndex/20)%TABLE_LENGTH_AMP];
					if (i%((int) (freq/xx/2)) == 0) {flag_high = !flag_high;}
					float value = (flag_high)*((100)* A/1000);
					if (flag_table) tab_value2[i]=(int) value;
					else tab_value1[i]=(int) value;
					ampIndex = ampIndex + 1;
				}
			}
			else {
				xx /= gain;
				for (int i=0;i<TABLE_LENGTH;i++) {
					A = tab_valueAmp[(int)floor((double)ampIndex/20)%TABLE_LENGTH_AMP];
					if (i%((int) (freq/xx/2)) == 0) {flag_high = !flag_high;}
					float value = (flag_high)*((100)* A/1000);
					if (flag_table) tab_value2[i]=(int) value;
					else tab_value1[i]=(int) value;
					ampIndex = ampIndex + 1;
				}
			}
			
			if (curTime >= 12000) {temps = 0;}
		}


//		ili9341_fill_screen(_screen, ILI9341_BLACK);
//		char buf[80];
//		sprintf(buf,"Temps:%i\r\nXX:%f\r\ntest:%f", curTime, xx, A/1000);
//		ili9341_text_attr_t text_attr = {&ili9341_font_11x18,ILI9341_WHITE,
//		ILI9341_BLACK,40,0};
//		ili9341_draw_string(_screen, text_attr, buf);
		;
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

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
	flag_end=1;
	if (flag_table){
		HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, tab_value2, TABLE_LENGTH, DAC_ALIGN_12B_R);
		flag_table = !flag_table;

	}
	else {
		HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, tab_value1, TABLE_LENGTH, DAC_ALIGN_12B_R);
		flag_table = !flag_table;

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
