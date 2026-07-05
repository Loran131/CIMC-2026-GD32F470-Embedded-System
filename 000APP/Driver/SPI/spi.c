 
#include "SPI/spi.h"


/**
 * @brief       SPI初始化代码
 * @note        主机模式,8位数据,禁止硬件片选
 * @param       无
 * @retval      无
 */
void spi0_init(void)
{
	  spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(SPI0_SCK_GPIO_CLK);        /* 使能SPI0_SCK IO口时钟 */
    rcu_periph_clock_enable(SPI0_MISO_GPIO_CLK);       /* 使能SPI0_MISO IO口时钟 */
    rcu_periph_clock_enable(SPI0_MOSI_GPIO_CLK);       /* 使能SPI0_MOSI IO口时钟 */
    rcu_periph_clock_enable(SPI0_SPI_CLK);             /* 使能SPI0时钟 */  

    /* 配置使用的SPI0引脚:
             SPI0_SCK->PB3
             SPI0_MISO->PB4
             SPI0_MOSI->PB5 */

    /* 配置SPI0引脚的复用功能 */
    gpio_af_set(SPI0_SCK_GPIO_PORT, SPI0_SCK_GPIO_AF, SPI0_SCK_GPIO_PIN);  
    gpio_af_set(SPI0_MISO_GPIO_PORT, SPI0_MISO_GPIO_AF, SPI0_MISO_GPIO_PIN);
    gpio_af_set(SPI0_MOSI_GPIO_PORT, SPI0_MOSI_GPIO_AF, SPI0_MOSI_GPIO_PIN);

    /* SPI0_SCK引脚模式设置 复用推挽输出 */
    gpio_mode_set(SPI0_SCK_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI0_SCK_GPIO_PIN);
    gpio_output_options_set(SPI0_SCK_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_SCK_GPIO_PIN);

    /* SPI0_MISO引脚模式设置 复用推挽输出 */
    gpio_mode_set(SPI0_MISO_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI0_MISO_GPIO_PIN);
    gpio_output_options_set(SPI0_MISO_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_MISO_GPIO_PIN);

    /* SPI0_MOSI引脚模式设置 复用推挽输出 */
    gpio_mode_set(SPI0_MOSI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI0_MOSI_GPIO_PIN);
    gpio_output_options_set(SPI0_MOSI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_MOSI_GPIO_PIN);
        /* 配置 CS 引脚为推挽输出并拉高(不选中) */
    rcu_periph_clock_enable(SPI0_CS_GPIO_CLK);
    gpio_mode_set(SPI0_CS_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI0_CS_GPIO_PIN);
    gpio_output_options_set(SPI0_CS_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_CS_GPIO_PIN);
    gpio_bit_set(SPI0_CS_GPIO_PORT, SPI0_CS_GPIO_PIN);

    spi_i2s_deinit(SPI0_SPI);                                          /* 复位SPI0 */
    spi_struct_para_init(&spi_init_struct);                            /* 初始化SPI结构体中所有参数为默认值 */
    
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;   /* 传输模式配置:设置为全双工模式 */
    spi_init_struct.device_mode          = SPI_MASTER;                 /* 配置为主机模式 */
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;         /* 数据帧格式配置;8位数据帧格式 */ 
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;     /* 空闲状态下,CLK保持低电平,在第一个时钟跳变沿采集第一个数据 */
    spi_init_struct.nss                  = SPI_NSS_SOFT;               /* NSS软件模式,NSS电平取决于SWNSS位 */
    spi_init_struct.prescale             = SPI_PSC_8;                  /* 预分频器配置:使用更快的分频 */
    spi_init_struct.endian               = SPI_ENDIAN_MSB;             /* 大端或小端模式配置;先发送最高有效位 */
    spi_init(SPI0_SPI, &spi_init_struct);                              /* 初始化SPI0 */
	  
    spi_enable(SPI0_SPI);	                                             /* 使能SPI0 */	
}

/**
 * @brief       SPI0速度设置函数
 * @note        SPI0时钟选择来自APB2, 即PCLK2, 为120Mhz
 *              SPI速度 = PCLK2 / 2^(speed + 1)
 * @param       speed   : SPI时钟分频系数
 * @retval      无
 */
void spi0_set_speed(uint8_t speed)
{
	  speed &= 0X07;                      /* 限制范围 */
	  spi_disable(SPI0_SPI);	            /* SPI失能 */
	  SPI_CTL0(SPI0_SPI) &= ~(7 << 3);    /* 先清零 */
	  SPI_CTL0(SPI0_SPI) |= speed << 3;   /* 设置分频系数 */
	  spi_enable(SPI0_SPI);	              /* SPI使能 */
}

/**
 * @brief       SPI0读写一个字节数据
 * @param       txdata  : 要发送的数据(1字节)
 * @retval      接收到的数据(1字节)
 */
uint8_t spi0_read_write_byte(uint8_t txdata)
{   
	  while(RESET == spi_i2s_flag_get(SPI0_SPI, SPI_FLAG_TBE));    /* 等待发送缓冲区空 */
	
	  spi_i2s_data_transmit(SPI0_SPI, txdata);                     /* 发送一个字节 */
	
	  while(RESET == spi_i2s_flag_get(SPI0_SPI, SPI_FLAG_RBNE));   /* 等待接收缓冲区非空 */
	
	  return spi_i2s_data_receive(SPI0_SPI);                       /* 返回收到的数据 */
}



/**
 * @brief       SPI3 初始化代码
 * @note        主机模式,8位数据,软件片选
 * @param       无
 * @retval      无
 */
void spi3_init(void)
{
    spi_parameter_struct spi_init_struct;

    /* 使能 SPI3 相关时钟 */
    rcu_periph_clock_enable(SPI3_SCK_GPIO_CLK);
    rcu_periph_clock_enable(SPI3_MISO_GPIO_CLK);
    rcu_periph_clock_enable(SPI3_MOSI_GPIO_CLK);
    rcu_periph_clock_enable(SPI3_CS_GPIO_CLK);
    rcu_periph_clock_enable(SPI3_SPI_CLK);

    /* 配置 SPI3 引脚复用功能 */
    gpio_af_set(SPI3_SCK_GPIO_PORT, SPI3_SCK_GPIO_AF, SPI3_SCK_GPIO_PIN);
    gpio_af_set(SPI3_MISO_GPIO_PORT, SPI3_MISO_GPIO_AF, SPI3_MISO_GPIO_PIN);
    gpio_af_set(SPI3_MOSI_GPIO_PORT, SPI3_MOSI_GPIO_AF, SPI3_MOSI_GPIO_PIN);

    /* SPI3_SCK (PE12) 复用推挽输出 */
    gpio_mode_set(SPI3_SCK_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI3_SCK_GPIO_PIN);
    gpio_output_options_set(SPI3_SCK_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI3_SCK_GPIO_PIN);

    /* SPI3_MISO (PE13) 复用推挽输出 */
    gpio_mode_set(SPI3_MISO_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI3_MISO_GPIO_PIN);
    gpio_output_options_set(SPI3_MISO_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI3_MISO_GPIO_PIN);

    /* SPI3_MOSI (PE14) 复用推挽输出 */
    gpio_mode_set(SPI3_MOSI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI3_MOSI_GPIO_PIN);
    gpio_output_options_set(SPI3_MOSI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI3_MOSI_GPIO_PIN);

    /* 配置 CS 引脚为推挽输出并拉高（不选中） */
    gpio_mode_set(SPI3_CS_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI3_CS_GPIO_PIN);
    gpio_output_options_set(SPI3_CS_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI3_CS_GPIO_PIN);
    gpio_bit_set(SPI3_CS_GPIO_PORT, SPI3_CS_GPIO_PIN);

    /* 复位 SPI3 */
    spi_i2s_deinit(SPI3_SPI);
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;  /* 根据 ADC 手册修改 */
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;               /* 初始分频，后续可调整 */
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI3_SPI, &spi_init_struct);

    /* 使能 SPI3 */
    spi_enable(SPI3_SPI);
}


/**
 * @brief       SPI3 速度设置函数
 * @note        SPI3 时钟来自 APB1(PCLK1)，设为 60MHz
 *              SPI 速度 = PCLK1 / 2^(speed + 1)
 * @param       speed   : SPI 时钟分频系数 (0~7)
 * @retval      无
 */
void spi3_set_speed(uint8_t speed)
{
    speed &= 0x07;
    spi_disable(SPI3_SPI);
    SPI_CTL0(SPI3_SPI) &= ~(7 << 3);
    SPI_CTL0(SPI3_SPI) |= speed << 3;
    spi_enable(SPI3_SPI);
}
/**
 * @brief       SPI3 读写一个字节数据
 * @param       txdata  : 要发送的数据（1字节）
 * @retval      接收到的数据（1字节）
 */
uint8_t spi3_read_write_byte(uint8_t txdata)
{
    while (RESET == spi_i2s_flag_get(SPI3_SPI, SPI_FLAG_TBE));  /* 等待发送缓冲区空 */
    spi_i2s_data_transmit(SPI3_SPI, txdata);                     /* 发送一个字节 */
    while (RESET == spi_i2s_flag_get(SPI3_SPI, SPI_FLAG_RBNE)); /* 等待接收缓冲区非空 */
    return spi_i2s_data_receive(SPI3_SPI);                       /* 返回收到的数据 */
}