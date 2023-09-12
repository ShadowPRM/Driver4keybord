/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
//#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Задержка автонажания					3с	=3000мс
// Период автонажатия					0,5с=500мс
// Время фиксации простого нажатия		0,1с=100мс
#define PER_OPR_KB 1000		//период опроса клавиатуры х10мкс (10000=>10мс=>100Гц)
#define VREM_FIX_KEY 10000		//время фиксации простого нажатия х10мкс (=100мс)
#define VREM_FIX_PRKEY 300000	//время фиксации зажагого нажатия х10мкс (=3000мс)
#define VREM_POVTOR_KEY 50000	//время/период повтора при долгом нажатии х10мкс (=500мс)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char uartBuf[64]={0,};
volatile uint32_t keyKeybord=0;
volatile uint8_t adrOprosa=0;
volatile _Bool oprosStart=0;
volatile _Bool oprosKB=0;
_Bool printNorm=0;
_Bool key1LongPress=0;
_Bool key2LongPress=0;
_Bool key1NormPress=0;
_Bool key2NormPress=0;
_Bool printLong=0;

volatile uint32_t countTEST1=0;
volatile uint32_t countOprKB=0;
volatile uint32_t countPovtorPrint=0;

uint32_t countPresKey1=0;
uint32_t countPresKey2=0;



uint8_t  numKey1=0;
uint8_t  numKey2=0;
uint8_t  najKey1=0;
uint8_t  najKey2=0;
uint8_t  kolKeyOn=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance==TIM1){		// каждые 10мс
		//if(++countTEST1>=1000){numKey1++;numKey2+=2;}
	}
	if(htim->Instance==TIM2){		// каждые 10мкс
		countPovtorPrint++;
		countPresKey1++;
		countPresKey2++;
		if (adrOprosa) {oprosStart=1;} else {countOprKB++;}
		if (countOprKB>=PER_OPR_KB) {oprosStart=1; countOprKB=0;} // oprosKB=1;
	}
	if(htim->Instance==TIM3){
		//numKey1=15;
		//numKey2=24;
		//if(++countTEST1>=1000){numKey1++;numKey2+=2;countTEST1=0;}
	}
}


uint8_t OprosKeybord (uint8_t adres){
	//uint8_t tekusheeSost=0;
	uint8_t  key1=0;
	uint8_t  key2=0;
	uint8_t  key3=0;

	if (!(adres & 0x01)){
	//Установка кода на HC138 для опроса клавиш (активное состояние л0!)
	if (adres&(1<<1)) {adr0out0_GPIO_Port->ODR |= adr0out0_Pin;} else {adr0out0_GPIO_Port->ODR &=~ adr0out0_Pin;}
	if (adres&(1<<2)) {adr1out1_GPIO_Port->ODR |= adr1out1_Pin;} else {adr1out1_GPIO_Port->ODR &=~ adr1out1_Pin;}
	if (adres&(1<<3)) {adr2out2_GPIO_Port->ODR |= adr2out2_Pin;} else {adr2out2_GPIO_Port->ODR &=~ adr2out2_Pin;}
	//adres &= 0b00000111;	//на всякий случай
	}
	else {
	//Проверка состояния входов (проверяем л0)
		adres=adres>>1;
	if (adr3in0_GPIO_Port->IDR & adr3in0_Pin)   {key1 &=~ (1<<3); keyKeybord &=~ (1<<(0+adres));}
												else {key1 |= (1<<3);  keyKeybord |= (1<<(0+adres)); }

	if (adr4in1_GPIO_Port->IDR & adr4in1_Pin)   {key2 &=~ (1<<4); keyKeybord &=~ (1<<(1+adres));}
												else {key2 |= (1<<4);  keyKeybord |= (1<<(1+adres));}

	if (adr5in2_GPIO_Port->IDR & adr5in2_Pin)   {key3 &=~ (1<<5); keyKeybord &=~ (1<<(2+adres));}
												else {key3 |= (1<<5);  keyKeybord |= (1<<(2+adres));}

	//проверка 3х нажатых в одном столбце
	if (key1 && key2 && key3) {key1=0;key2=0;key3=0; numKey1=0; numKey2=0;  return (0xfe);} //adrOprosa=0;

	//проверка 3х нажатых в общем опросе клавы
	if (numKey1 && numKey2 && (key1|key2|key3)) {key1=0;key2=0;key3=0; numKey1=0; numKey2=0;  return (0xff);} //adrOprosa=0;

	//присвоение
	if (!(numKey1)) {
		if(key1) {numKey1=key1|adres; key1=0;} else if(key2) {numKey1=key2|adres; key2=0;} else if(key3) {numKey1=key3|adres; key3=0;}
	}
	if (!(numKey2)) {
		if(key1) {numKey2=key1|adres;} else if(key2) {numKey2=key2|adres;} else if(key3) {numKey2=key3|adres;}
	}
	}
	//возвращаем количество нажатых
	return (kolKeyOn);
}

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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1){
	uint16_t sizeString=0;
	uint8_t kolNaj=0;
	char uartBuf [64]={0,};


	/* Каждые 10мкс тикает таймер поднимает oprosStart
	 * (т.е. частота опроса группы (1столбец=3клавиши) кнопок 100кГц) (Совокупность времени реакции 74HC138 адрес-выход ~528ns (1,893МГц) вроде бы:) )
	 * (вприципе можно поднять частоту опроса)
	 * В майне запускается опрос одного столбца клавиш
	 * ! Задать частоту опроса всей клавиатуры
	 * ! Включить прерывания на входах
	 * ! От прерывания по входу, отсчитывать время удержания клавиши*/
	if (oprosStart){		// Опрашиваем клавиатуру 8 раз по 3 кнопки (фиксируем нажатые кнопки)
		kolNaj = OprosKeybord(adrOprosa);
		adrOprosa++;
		//у HC 8 состояний выхода (в каждом по 3 кнопки)
		if (adrOprosa>=16){adrOprosa=0; oprosKB=1;}
		oprosStart=0;
	}

	if (oprosKB){		// Когда опросили надо +1 счётчику нажатой кнопки, если нажата та же


		if (numKey1!=najKey1) {najKey1=numKey1; countPresKey1=0; printLong=0; key1NormPress=0; key1LongPress=0; //(numKey1)&&
			//numKey1=0;
		}
		/*if (numKey2) {
			if(numKey2==najKey2){;}	//countPresKey2++
			else				{najKey2=numKey2; countPresKey2=0; printLong=0; key2NormPress=0; key2LongPress=0;}
			numKey2=0;
		}*/
		if(numKey2!=najKey2) {najKey2=numKey2; countPresKey2=0; printLong=0; key2NormPress=0; key2LongPress=0;
			//numKey2=0;
		}
		numKey1=0; numKey2=0; //key1=0; key2=0; key3=0;
		oprosKB=0;
	}


	//фиксация длительного нажатия
	if (!key1LongPress && najKey1 && (countPresKey1>=VREM_FIX_PRKEY)){
		printLong=1; countPovtorPrint=0;
		key1LongPress=1;
	}
	if (!key2LongPress && najKey2 && (countPresKey2>=VREM_FIX_PRKEY)){
		printLong=1; countPovtorPrint=0;
		key2LongPress=1;
	}

	//фиксация обычного нажатия
	if (!key1NormPress && najKey1 && (countPresKey1>=VREM_FIX_KEY)){ //
		key1NormPress=1;
		printNorm=1;
	}
	if (!key2NormPress && najKey2 && (countPresKey2>=VREM_FIX_KEY)){
		key2NormPress=1;
		printNorm=1;
	}

	//вывод(печать)
	if (kolNaj == 0xfe){
		sizeString=sprintf(uartBuf, "Eror: Over 2 key/ryad!\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t*)uartBuf, sizeString,100);
		HAL_Delay(1000);
		kolNaj=0;
	}
	if (kolNaj == 0xff){
		sizeString=sprintf(uartBuf, "Eror: Over 2 key/KB!\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t*)uartBuf, sizeString,100);
		HAL_Delay(1000);
		kolNaj=0;
	}
	if (printNorm){
		sizeString=sprintf(uartBuf, "Key1: % 3d\t\tKey2: % 3d\r\n", najKey1, najKey2);
		HAL_UART_Transmit(&huart1, (uint8_t*)uartBuf, sizeString,30);
		printNorm=0;
	}
	if ((printLong)&&(countPovtorPrint>=VREM_POVTOR_KEY)) {	//countLongKey
		sizeString=sprintf(uartBuf, "LongKey1: % 3d\t\tLongKey2: % 3d\r\n", najKey1, najKey2);
		HAL_UART_Transmit(&huart1, (uint8_t*)uartBuf, sizeString,30);
		countPovtorPrint=0;
	}




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

