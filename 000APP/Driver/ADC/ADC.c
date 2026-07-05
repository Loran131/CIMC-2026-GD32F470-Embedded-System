/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：adc.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/31     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "adc.h"
#include "dac.h"
//! 此时只跑了一个ADC,不需要扫描

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/
static void __ADC0_Init_GPIO(void);
static void __ADC0_Init(void);

void __ADC_Init_GPIO(void);
void __ADC_Init(void);
/************************************************************
 * Function :       ADC_Init
 * Comment  :       用于初始化ADC
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-31 V0.01 original
************************************************************/
void ADC_Init(void)
{
    __ADC_Init_GPIO();
    __ADC_Init();
    my_dac_init();
}

/************************************************************
 * Function :       __ADC_Init_GPIO
 * Comment  :       用于初始化ADC GPIO
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-31 V0.01 original
************************************************************/
void __ADC_Init_GPIO(void)
{
    // 开启ADC_GPIO时钟
    rcu_periph_clock_enable(ADC_GPIO_RCU);
    // 开启ADC时钟
    rcu_periph_clock_enable(ADC_RCU);
    // 设置ADC GPIO引脚为模拟输入模式
    gpio_mode_set(ADC_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, ADC_GPIO_PIN);

}
/************************************************************
 * Function :       __ADC_Init
 * Comment  :       用于初始化ADC
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-31 V0.01 original
************************************************************/
void __ADC_Init(void)
{
    // 复位ADC
    adc_deinit();
    // 配置ADC时钟   8分频  240/8 = 30MHz 采集频率 30MHZ ？？？这么快？？
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    // ADC模式配置   独立模式下
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    // ADC不使用扫描模式
    adc_special_function_config(ADCX, ADC_SCAN_MODE, DISABLE);
    // ADC启用连续模式
//    adc_special_function_config(ADCX, ADC_CONTINUOUS_MODE, ENABLE);
    // ADC数据对齐配置(右对齐)
    adc_data_alignment_config(ADCX, ADC_DATAALIGN_RIGHT);
    // ADC 通道长度配置(使用常规)
    adc_channel_length_config(ADCX, ADC_ROUTINE_CHANNEL, 1);
    // ADC常规通道配置
    adc_routine_channel_config(ADCX, 0, ADC_CHANNEL, ADC_SAMPLETIME_56);
    // ADC 触发配置 //!先试用手动触发 采集电压的不需要自动触发
    //! 下面是常规触发
    // adc_external_trigger_source_config(ADCX, ADC_ROUTINE_CHANNEL, ADC_EXTTRIG_ROUTINE_NONE);
    adc_external_trigger_config(ADCX, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    // 开启adc接口
    adc_enable(ADCX);
    // 等待1ms
    delay_1ms(1);
    // ADC校准和复位ADC校准
    adc_calibration_enable(ADCX);
}
/************************************************************
 * Function :       ADC_Read_Register
 * Comment  :       用于读取ADC寄存器值
 * Parameter:       null
 * Return   :       uint16_t  ADC寄存器值
 * Author   :       Jialei Zhao
 * Date     :       2025-12-31 V0.01 original
************************************************************/
uint16_t ADC_Read_Register(void)
{
    // 启用转化
    adc_software_trigger_enable(ADCX, ADC_ROUTINE_CHANNEL);
    // 等待转化完成
    while (!adc_flag_get(ADCX, ADC_FLAG_EOC))
        ;
    // 清除EOC标志
    adc_flag_clear(ADCX, ADC_FLAG_EOC);
    uint16_t adc_value = adc_routine_data_read(ADCX);

	  // 转换为DAC输出值
//    /* set DAC output data */
//    dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, adc_value);
//    /* enable DAC software trigger */
//    dac_software_trigger_enable(DAC0, DAC_OUT0);
    // 返回转换结果
    return adc_value;
}
void ADC0_Init(void)
{
    __ADC0_Init_GPIO();
    __ADC0_Init();
}

/*---------------------------------------------------------------------------*/
/* GPIO 和时钟配置                                                           */
/*---------------------------------------------------------------------------*/
static void __ADC0_Init_GPIO(void)
{
    // 开启ADC0对应GPIO时钟
    rcu_periph_clock_enable(ADC0_GPIO_RCU);
    // 开启ADC0时钟
    rcu_periph_clock_enable(ADC0_RCU);
    // 设置引脚为模拟输入模式
    gpio_mode_set(ADC0_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, ADC0_GPIO_PIN);
}

static void __ADC0_Init(void)
{
    // 复位ADC0
    adc_deinit();
    // ADC时钟配置，8分频（PCLK2/8）
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    // 独立模式（ADC0不与AD1/2同步）
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    // 关闭扫描模式（仅单个通道）
    adc_special_function_config(ADC0X, ADC_SCAN_MODE, DISABLE);
    // 启用连续转换模式（使能后ADC持续转换，适合实时读取）
//	adc_special_function_config(ADC0X, ADC_CONTINUOUS_MODE, ENABLE);
    // 数据右对齐（常规12位结果存放于寄存器低16位）
    adc_data_alignment_config(ADC0X, ADC_DATAALIGN_RIGHT);
    // 规则通道组长度 = 1（只转换一个规则通道）
    adc_channel_length_config(ADC0X, ADC_ROUTINE_CHANNEL, 1);
    // 配置规则通道：序号0, 通道号为之前宏定义, 采样时间56个周期
    adc_routine_channel_config(ADC0X, 0, ADC0_CHANNEL, ADC_SAMPLETIME_56);
    // 禁能外部触发（使用软件触发）
    adc_external_trigger_config(ADC0X, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    // 使能ADC0
    adc_enable(ADC0X);
    // 等待ADC稳定（参考原代码延迟1ms）
    delay_1ms(1);
    // 执行校准
    adc_calibration_enable(ADC0X);
}

/*---------------------------------------------------------------------------*/
/* ADC0 采样函数：返回规则通道转换值（16位）                                  */
/* 每次调用启动一次软件触发，等待转换完成后读取结果                             */
/*---------------------------------------------------------------------------*/
uint16_t ADC0_Read(void)
{
    // 软件触发规则通道转换
    adc_software_trigger_enable(ADC0X, ADC_ROUTINE_CHANNEL);
    // 等待规则通道软件触发转换开始位清0（表示转换完成）
    while(adc_routine_software_startconv_flag_get(ADC0X) != RESET);
    // 读取并返回规则数据寄存器
    return adc_routine_data_read(ADC0X);
}
/****************************End*****************************/
