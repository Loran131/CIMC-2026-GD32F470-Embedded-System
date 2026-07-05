
#include "gd32f4xx.h"
#include "GD30AD3344_standard_peripheral\Include\gd30ad3344.h"

uint16_t ADC_Config[2]={0}; 
uint16_t AD3344_CONFIG;
int16_t adc_16bit;
uint16_t register_data;
float ADC_Value;
float pga;
uint32_t sample_count = 10;
uint32_t number_of_sample = 1;
/*!  adc暂无中断引脚
    \brief      exti-line enable (PA6)
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void ad3344_Exit_enable(void)
//{
//    rcu_periph_clock_enable(RCU_GPIOA);
//	rcu_periph_clock_enable(RCU_SYSCFG);
////    rcu_periph_clock_enable(RCU_AF);
//		 /* 使能GPIOA时钟 */
//		rcu_periph_clock_enable(RCU_GPIOA);

//		/* 配置GPIO模式为输入模式，无上拉/下拉（即浮空输入） */
//		gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_6);

////    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
////    /* connect key wakeup EXTI line to key GPIO pin */
////    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_6);
//	  /* 配置EXTI线6的GPIO引脚源为PA6 */
//    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN6);

//    /* configure key wakeup EXTI line */
//    exti_init(EXTI_6, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
//    exti_interrupt_flag_clear(EXTI_6);
//    
//    nvic_irq_enable(EXTI5_9_IRQn, 2U, 0U);
//}

/*!
    \brief      exti-line disable
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ad3344_Exit_disable(void)
{
    nvic_irq_disable(EXTI5_9_IRQn);
    exti_interrupt_flag_clear(EXTI_6);
    exti_interrupt_disable(EXTI_6);
    
    rcu_periph_clock_enable(RCU_SPI3);
	  rcu_periph_clock_enable(RCU_GPIOE);
//    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, 
                  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_9);  // 修改点2：改为PE引脚
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, 
                            GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_9);  // 修改点3：改为PE引脚

    gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_12);   // SCK
    gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_13);   // MISO
    gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_9);    // MOSI
	  gpio_mode_set(SPI3_CS_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI3_CS_GPIO_PIN);
    gpio_output_options_set(SPI3_CS_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI3_CS_GPIO_PIN);
    SPI3_CS_HIGH();
}

/*!
    \brief      GD30AD3344 transmit data
    \param[in]  config_d: register value
    \param[out] none
    \retval     the read value of register
*/
uint16_t AD3344_Send_Data(uint16_t config_d)
{
    uint16_t Data;

    Data = ad3344_spi_txrx16bit(config_d);
    
    return (Data);
}

/*!
    \brief      GD30AD3344 Config Register(32bit trans)
    \param[in]  config_d: the data need to be tramit
    \param[in]  *config: Register readback value
    \param[out] none
    \retval     the read value of register
*/
uint16_t ad3344_read_data32(uint16_t config_d, uint16_t *config)
{
    uint16_t data;
    
    data = AD3344_Send_Data(config_d);
    *config = AD3344_Send_Data(0);
    
    return (data);
}

/*!
    \brief      GD30AD3344 Config Register(16bit trans)
    \param[in]  config_d: the data need to be tramit
    \param[out] none
    \retval     the read value of register
*/
uint16_t ad3344_read_data16(uint16_t config_d)
{
    uint16_t data;
    
    SPI_CLR_CS();
    delay_1ms(1);
    
    data = AD3344_Send_Data(config_d);
    
    SPI_SET_CS();
    delay_1ms(1);
    
    SPI_CLR_CS();
    
    return (data);
}

/*!
    \brief      GD30AD3344 Read Register
    \param[in]  addr
      \arg      0x01: Config Register
    \param[out] none
    \retval     the read value of register
*/
													uint16_t ad3344_read_regs()
													{ float voltage, resistance, temperature;
														uint16_t data;
														SPI_CLR_CS();
														delay_1ms(1);                     // 保证CS建立时间
														data = ad3344_spi_txrx16bit(0);  // 发送16个SCLK，DOUT输出转换结果
														SPI_SET_CS();                     // 保持CS高脉冲宽度
														delay_1ms(1);
														voltage = (float)data * 125e-6;   // 如果每LSB=125μV，但手册表3是差分FSR，单端需调整
																// 假设采用分压电路，需根据实际电路换算。

																// 5. 转换为电阻（示例：1mA恒流源，V = I*R -> R = V / 0.001）
																resistance = voltage / 0.001f;

																// 6. 计算温度（简单线性近似，实际需高精度查表）
																temperature = (resistance - 100.0f) / 0.385f;
														
														return data;
													}

void ad3344_process(void)
{
    uint16_t addr,val;
    uint16_t tx_data;
    
    addr = 0x10 + 0x02;
    val = 0xACCA;
    
    SPI_CLR_CS();
    delay_1ms(1);
    
    tx_data = 0x8100;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = addr;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = val;
    ad3344_spi_txrx16bit(tx_data);
    delay_1ms(1);
    
    SPI_SET_CS();
    delay_1ms(1);
}

void ad3344_ExtRef(void)
{
    uint16_t addr,val,rdval;
    uint16_t tx_data;
    
    addr = 0x10 + 0x4;
    
    SPI_CLR_CS();
    delay_1ms(1);
    
    tx_data = 0x8106;
    ad3344_spi_txrx16bit(tx_data);
    delay_1ms(1);
    
    tx_data = addr;
    ad3344_spi_txrx16bit(tx_data);
    delay_1ms(1);
    
    rdval = ad3344_spi_txrx16bit(0x00);
    delay_1ms(1);
    
    SPI_SET_CS();
    delay_1ms(1);
    
    val = rdval | 0x40;
    
    SPI_CLR_CS();
    delay_1ms(1);
    
    tx_data = 0x8100;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = addr;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = val;
    ad3344_spi_txrx16bit(tx_data);
    delay_1ms(1);
    
    SPI_SET_CS();
    delay_1ms(1);
}

/*!
    \brief      GD30AD3344 Init
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ad3344_init(uint16_t config_d)
{
    SPI_CLR_CS();
    delay_1ms(1);
    
    #ifdef BIT32_TRANS_CYCLE
    ad3344_read_data32(config_d, ADC_Config);
    #else
    ad3344_spi_txrx16bit(config_d);
    #endif
    
    delay_1ms(1);
	
    SPI_SET_CS();
    delay_1ms(1);

    SPI_CLR_CS();
    delay_1ms(1);
}

/*!
    \brief      GD30AD3344 stop conversion
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ad3344_stop_conver()
{
    AD3344_CONFIG |= AD3344_REG_CONFIG_MODE_SINGLE;
    
    #ifdef BIT32_TRANS_CYCLE
    ad3344_read_data32(AD3344_CONFIG, ADC_Config);
    #else
    ad3344_spi_txrx16bit(AD3344_CONFIG);
    #endif
}

/*!
    \brief      GD30AD3344 reset
    \param[in]  none
    \param[out] none
    \retval     the result of the conversion
*/
void ad3344_reset()
{
    #ifdef BIT32_TRANS_CYCLE
    ad3344_read_data32(AD3344_CONFIG_DEFAULT, ADC_Config);
    #else
    ad3344_spi_txrx16bit(AD3344_CONFIG_DEFAULT);
    #endif
}
void AD3344_reg_Config(uint8_t InputMUX, uint8_t Channel)
{
    if(InputMUX == AD3344_DUAL_END)
    {
        switch (Channel)
        {
        case (0):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_DIFF_0_1;
          break;
        case (1):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_DIFF_0_3;
          break;
        case (2):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_DIFF_1_3;
          break;
        case (3):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_DIFF_2_3;
          break;
        }
    }else if(InputMUX == AD3344_SINGLE_END)
    {
        switch (Channel)
        {
        case (0):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_SINGLE_0;
          break;
        case (1):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_SINGLE_1;
          break;
        case (2):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_SINGLE_2;
          break;
        case (3):
          AD3344_CONFIG |= AD3344_REG_CONFIG_MUX_SINGLE_3;
          break;
        }
    }
    
    AD3344_CONFIG |= AD3344_REG_CONFIG_DR_1000SPS;
    AD3344_CONFIG |= AD3344_REG_CONFIG_PULL_UP_EN;
    AD3344_CONFIG |= AD3344_REG_CONFIG_NOP_VALID;
    
    AD3344_CONFIG |= AD3344_REG_CONFIG_PGA_4_096V;
    
    if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_6_144V){
        pga = 6.144;
    }else if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_4_096V){
        pga = 4.096;
    }else if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_2_048V){
        pga = 2.048;
    }else if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_1_024V){
        pga = 1.024;
    }else if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_0_512V){
        pga = 0.512;
    }else if((AD3344_CONFIG&AD3344_REG_CONFIG_PGA_MASK) == AD3344_REG_CONFIG_PGA_0_256V){
        pga = 0.256;
    }else{
        pga = 0.064;
    }
    
    #ifdef CONTINUOUS_CONVERSION
        AD3344_CONFIG |= AD3344_REG_CONFIG_MODE_CONTIN;
    #else
        AD3344_CONFIG |= AD3344_REG_CONFIG_MODE_SINGLE;
        /* Set 'start single-conversion' bit */
        AD3344_CONFIG |= AD3344_REG_CONFIG_OS_SINGLE;
    #endif
}
void ad3344_polling_read(void)
{
    /* 读取一次转换结果 */
    adc_16bit = ad3344_read_data16(AD3344_CONFIG);   // 沿用原中断中的读法
    ADC_Value = adcdata_to_volt(adc_16bit);

    printf("%d:  0x%04x  ADC_VALUE=%.4f\n", number_of_sample, adc_16bit, ADC_Value);
    number_of_sample++;

    if (number_of_sample >= sample_count)
    {
        ad3344_stop_conver();          // 停止转换
        usart_interrupt_disable(USART0, USART_INT_RBNE);  // 若需关闭串口中断
        printf("sampling over!\n\r");
    }
}
float adcdata_to_volt(uint16_t bin)
{
    int  _val;
    float adcValue;
    
    if(bin == NEGATIVE_FS){
        adcValue = -(pga*bin/ADC_DATA);
    }else{
        _val     =  bin&NEGATIVE_FS ?  (-((~bin+1)&POSITIVE_FS)):bin;
        adcValue = pga*_val/ADC_DATA;
    }
    
    return adcValue;
}
void timer2_init_for_adc(void)
{
    rcu_periph_clock_enable(RCU_TIMER2);
    timer_parameter_struct timer_par;
    timer_struct_para_init(&timer_par);

    timer_par.prescaler = 120 - 1;          // 假设系统时钟120MHz，120分频后为1MHz
    timer_par.period = 1000 - 1;            // 自动重装载值1000，产生1ms定时
    timer_par.clockdivision = TIMER_CKDIV_DIV1;
    timer_par.counterdirection = TIMER_COUNTER_UP;
    timer_init(TIMER2, &timer_par);

    timer_flag_clear(TIMER2, TIMER_FLAG_UP);
    timer_interrupt_enable(TIMER2, TIMER_INT_UP);
    nvic_irq_enable(TIMER2_IRQn, 1, 0);     // 设置合适的中断优先级

    timer_enable(TIMER2);
}
void TIMER2_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

        if (number_of_sample < sample_count)
        {
            /* 读取ADC数据 */
            adc_16bit = ad3344_read_data16(AD3344_CONFIG);
            ADC_Value = adcdata_to_volt(adc_16bit);

//            printf("%d:  0x%04x  ADC_VALUE=%.4f\n", number_of_sample, adc_16bit, ADC_Value);
            number_of_sample++;
        }
        else
        {
            /* 采样完成，停止转换并关闭定时器中断 */
            ad3344_stop_conver();
            timer_interrupt_disable(TIMER2, TIMER_INT_UP);
            usart_interrupt_disable(USART0, USART_INT_RBNE);
//            printf("sampling over!\n\r");
        }
    }
}
