/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：adc.h
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/31     V0.01    original
************************************************************/
#ifndef __ADC_H__
#define __ADC_H__

/************************* 头文件 *************************/
#include "HeaderFiles.h"

/************************* 宏定义 *************************/
#define ADC_RCU RCU_ADC1
#define ADC_GPIO_RCU RCU_GPIOC
#define ADC_GPIO_PORT GPIOC
#define ADC_GPIO_PIN GPIO_PIN_0

#define ADC_CHANNEL ADC_CHANNEL_10
#define ADCX ADC1


#define ADC0_RCU            RCU_ADC0
#define ADC0_GPIO_RCU       RCU_GPIOC           
#define ADC0_GPIO_PORT      GPIOA
#define ADC0_GPIO_PIN       GPIO_PIN_1           
#define ADC0_CHANNEL        ADC_CHANNEL_1        
#define ADC0X               ADC0

/************************ 变量定义 ************************/


/************************ 函数定义 ************************/


void ADC_Init(void);

uint16_t ADC_Read_Register(void);

uint16_t ADC0_Read(void);
static void __ADC0_Init(void);
static void __ADC0_Init_GPIO(void);
void ADC0_Init(void);
#endif // !__ADC_H__

/****************************End*****************************/
