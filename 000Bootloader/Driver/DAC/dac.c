/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：adc.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/31     V0.01    original
************************************************************/


/************************* 头文件 *************************/
#include "dac.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 函数定义 ************************/

/************************************************************
 * Function :       my_dac_init
 * Comment  :       用于初始化DAC
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-31 V0.01 original
************************************************************/
void my_dac_init(void) {
    /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable DAC clock */
    rcu_periph_clock_enable(RCU_DAC);

    /* configure PA4 as DAC output */
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);

    /* initialize DAC */
    dac_deinit(DAC0);
    /* DAC trigger config */
    dac_trigger_source_config(DAC0, DAC_OUT0, DAC_TRIGGER_SOFTWARE);
    /* DAC trigger enable */
    dac_trigger_enable(DAC0, DAC_OUT0);
    /* DAC wave mode config */
    dac_wave_mode_config(DAC0, DAC_OUT0, DAC_WAVE_DISABLE);
    /* DAC output buffer config */
    dac_output_buffer_enable(DAC0, DAC_OUT0);

    /* DAC enable */
    dac_enable(DAC0, DAC_OUT0);
}

/****************************End*****************************/
