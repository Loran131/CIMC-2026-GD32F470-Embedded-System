/************************************************************
* 版权：2025CIMC Copyright。
* 文件：usart.c
* 作者: Jialei Zhao
* 平台: 2025CIMC IHD-V04
* 版本: Jialei Zhao     2026/2/9     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "i2c.h"

/************************* 宏定义 *************************/
#define RCU_GPIO_I2C_SCL    RCU_GPIOB
#define RCU_GPIO_I2C_SDA    RCU_GPIOB

#define I2C_SCL_PORT        GPIOB
#define I2C_SDA_PORT        GPIOB
#define I2C_SCL_PIN         GPIO_PIN_8
#define I2C_SDA_PIN         GPIO_PIN_9

// #define ADDR 0xA0

//!拉高/拉低 数据线
#define IIC_SDA_HIGH (gpio_bit_set(I2C_SDA_PORT,I2C_SDA_PIN))
#define IIC_SDA_LOW (gpio_bit_reset(I2C_SDA_PORT,I2C_SDA_PIN))

//!拉高/拉低 时钟线
#define IIC_SCL_HIGH (gpio_bit_set(I2C_SCL_PORT,I2C_SCL_PIN))
#define IIC_SCL_LOW (gpio_bit_reset(I2C_SCL_PORT,I2C_SCL_PIN))

//!读取SDA
#define IIC_SDA_READ (gpio_input_bit_get(I2C_SDA_PORT,I2C_SDA_PIN))
/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

/************************************************************
 * Function :       IIC_delay
 * Comment  :       用户程序功能: 延时
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void IIC_delay(void)
{
	__IO uint16_t t =100;
	while (t--);
}

/************************************************************
 * Function :       I2C_Init
 * Comment  :       用户程序功能: 初始化I2C
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_I2C_Init(void)
{

	rcu_periph_clock_enable(RCU_GPIO_I2C_SCL);
	rcu_periph_clock_enable(RCU_GPIO_I2C_SDA);

	gpio_mode_set(I2C_SCL_PORT , GPIO_MODE_OUTPUT , GPIO_PUPD_PULLUP , I2C_SCL_PIN);
	gpio_output_options_set(I2C_SCL_PORT , GPIO_OTYPE_PP , GPIO_OSPEED_50MHZ , I2C_SCL_PIN);

	gpio_mode_set(I2C_SDA_PORT , GPIO_MODE_OUTPUT , GPIO_PUPD_PULLUP , I2C_SDA_PIN);
	gpio_output_options_set(I2C_SDA_PORT , GPIO_OTYPE_OD , GPIO_OSPEED_50MHZ , I2C_SDA_PIN);

	IIC_SCL_HIGH;
	IIC_SDA_HIGH;
	IIC_delay();
}

/************************************************************
 * Function :       I2C_Start
 * Comment  :       用户程序功能: 发送I2C启动信号
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void I2C_Start(void)
{

	IIC_SDA_HIGH;
	IIC_SCL_HIGH;
	IIC_delay();
	IIC_SDA_LOW;
	IIC_delay();
	IIC_SCL_LOW;
	IIC_delay();
}

/************************************************************
 * Function :       I2C_Stop
 * Comment  :       用户程序功能: 发送I2C停止信号
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void I2C_Stop(void)
{
	IIC_SCL_LOW;

	// IIC_SDA_LOW;

	IIC_SDA_LOW;
	IIC_delay();
	IIC_SCL_HIGH;
	IIC_delay();
	IIC_SDA_HIGH;
	for (uint8_t i = 0; i < 100; i++)
	{
		IIC_delay();
	}
}

/************************************************************
 * Function :       my_I2C_Send_Byte
 * Comment  :       用户程序功能: 发送I2C字节
 * Parameter:       dataToSend - 要发送的字节数据
 * Return   :       发送成功返回0，失败返回1
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
uint8_t my_I2C_Send_Byte(uint8_t dataToSend)
{
	// IIC_SDA_LOW;

	for (uint8_t i = 0; i < 8; i++)
	{
		IIC_SCL_LOW;
		IIC_delay();
		if ((dataToSend >> 7) & 0x01)
		{
			IIC_SDA_HIGH;
		}
		else
		{
			IIC_SDA_LOW;
		}
		IIC_delay();
		IIC_SCL_HIGH;
		IIC_delay();
		dataToSend <<= 1;
	}
	//! 获取应答
	IIC_SCL_LOW;
	IIC_delay();

	IIC_SDA_HIGH;

	IIC_delay();
	IIC_SCL_HIGH;
	IIC_delay();
	uint8_t i = 250;
	while (i--)
	{
		if (!IIC_SDA_READ)
		{
			IIC_SCL_LOW;
			IIC_delay();
			return 0;
		}
	}
	IIC_SCL_LOW;
	IIC_delay();
	return 1;
}

/************************************************************
 * Function :       I2C_Respond
 * Comment  :       用户程序功能: 发送I2C应答
 * Parameter:       ACKSignal - 应答信号，1为ACK，0为NACK
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void I2C_Respond(unsigned char ACKSignal)
{

	// IIC_SDA_LOW;

	IIC_SDA_LOW;
	IIC_SCL_LOW;

	if (ACKSignal)
	{
		IIC_SDA_HIGH;
	}
	else
	{
		IIC_SDA_LOW;
	}
	IIC_delay();
	IIC_SCL_HIGH;
	IIC_delay();
	IIC_SCL_LOW;
	IIC_delay();
}

/************************************************************
 * Function :       my_I2C_Read_Byte
 * Comment  :       用户程序功能: 读取I2C字节
 * Parameter:       null
 * Return   :       读取到的字节值
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
uint8_t my_I2C_Read_Byte(void)
{

	uint8_t buf = 0;
	IIC_SDA_HIGH;
	IIC_SCL_LOW;
	for (uint8_t i = 0; i < 8; i++)
	{
		IIC_delay();
		IIC_SCL_HIGH;
		buf = (buf << 1) | IIC_SDA_READ;
		IIC_delay();
		IIC_SCL_LOW;
		IIC_delay();
	}
	return buf;

}

/************************************************************
 * Function :       test_send
 * Comment  :       测试函数：发送0xAA并验证
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
// 测试函数：发送0xAA并验证
// void test_send(void)
// {
// 	I2C_Start();                          // 启动I2C
// 	my_I2C_Send_Byte(0xF0);               // 发送0xF0
// 	my_I2C_Send_Byte(0xAA);               // 发送0xAA
// 	I2C_Stop();                           // 停止I2C
// }

/****************************End*****************************/
