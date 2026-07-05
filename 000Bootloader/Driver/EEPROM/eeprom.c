/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：usart.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/2/9     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "eeprom.h"
#include "I2C/i2c.h"

/************************* 宏定义 *************************/
#define I2C_PAGE_SIZE 16

/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

/************************************************************
 * Function :       my_eeprom_init
 * Comment  :       用于初始化EEPROM
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void my_eeprom_init(void)
{
   my_I2C_Init();
}

/************************************************************
 * Function :       EEPROM_WritePage
 * Comment  :       用于写入EEPROM一页数据
 * Parameter:       addr  要写入的地址
 * Comment  :       用于写入EEPROM一页数据
 * Parameter:       addr  EEPROM地址
 * Parameter:       data  要写入的数据
 * Parameter:       size  要写入的数据大小
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void EEPROM_WritePage(uint8_t addr, uint8_t *data, uint16_t size)
{
    I2C_Start();
    //! 发送地址 + 写标识
    my_I2C_Send_Byte(0xA0);
    my_I2C_Send_Byte(addr);
    for (uint8_t i = 0; i < size; i++)
    {
        my_I2C_Send_Byte(data[i]);
    }
    I2C_Stop();
    delay_1ms(10);
}

/************************************************************
 * Function :       EEPROM_WriteStr
 * Comment  :       用于写入EEPROM字符串
 * Parameter:       addr  要写入的地址
 * Comment  :       用于写入EEPROM字符串
 * Parameter:       addr  EEPROM地址
 * Parameter:       data  要写入的数据
 * Parameter:       len   要写入的数据大小
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void EEPROM_WriteStr(uint8_t addr, uint8_t *data, uint16_t len)
{

    uint8_t pageRemain = I2C_PAGE_SIZE - (addr % I2C_PAGE_SIZE);
    if (pageRemain >= len)
    {
        EEPROM_WritePage(addr, data, len);
    }
    else
    {
        EEPROM_WritePage(addr, data, pageRemain);
        EEPROM_WriteStr(addr + pageRemain, data + pageRemain, len - pageRemain);
    }
}

/************************************************************
 * Function :       EEPROM_ReadStr
 * Comment  :       用于读取EEPROM字符串
 * Parameter:       addr  要读取的地址
 * Comment  :       用于读取EEPROM字符串
 * Parameter:       addr  EEPROM地址
 * Parameter:       data  要读取的数据
 * Parameter:       len   要读取的数据大小
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-02-10 V0.1 original
************************************************************/
void EEPROM_ReadStr(uint8_t addr, uint8_t *data, uint16_t len)
{

    I2C_Start();

    my_I2C_Send_Byte(0xA0);

    my_I2C_Send_Byte(addr);

    I2C_Start();
    my_I2C_Send_Byte(0xA1);

    for (int8_t i = 0; i < len; i++)
    {
        data[i] = my_I2C_Read_Byte();
        if (i == len - 1)
        {
            I2C_Respond(1);
        }
        else
        {
            I2C_Respond(0);
        }
    }

    I2C_Stop();
}

/****************************End*****************************/

