/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：timer.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2025/12/29      V0.01    original
************************************************************/

/************************* 头文件 *************************/
#include "timer.h"

/************************* 宏定义 *************************/

/************************ 变量定义 ************************/

/************************ 函数定义 ************************/
void timer_gpio_init(void);
void timer_config(void);

/************************************************************
 * Function :       my_timer_init
 * Comment  :       用于初始化定时器1（PWM模式）
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-29 V0.01 original
************************************************************/
void my_timer_init(void)
{

    timer_gpio_init(); // 初始化定时器1的GPIO引脚（PWM模式）

    timer_config();	// 初始化定时器1（PWM模式）
}

/************************************************************
 * Function :       timer_gpio_init
 * Comment  :       用于初始化定时器1的GPIO引脚（PWM模式）
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-29 V0.01 original
************************************************************/
void timer_gpio_init(void) {

    rcu_periph_clock_enable(RCU_GPIOB);

    /*Configure PB3 (TIMER1 CH1) as alternate function*/
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_3);
}

/************************************************************
 * Function :       timer_config
 * Comment  :       用于配置定时器1（PWM模式）
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-29 V0.01 original
************************************************************/
void timer_config(void) {

    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

    timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.prescaler = 240 - 1;
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = TIMER_ARR;
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    //! repetitioncounter 是重复计数器  0 表示不重复 1个周期产生更新事件
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1, &timer_initpara);

    /* CH1,CH2 and CH3 configuration in PWM mode */
    timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
    //! 关闭互补通道  不需要处理H桥问题
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER1, TIMER_CH_1, &timer_ocintpara);

    //!16000微妙一次事件

    /* CH1 configuration in PWM mode1,duty cycle duty */
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1,0);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
    /* auto-reload preload enable */
    timer_enable(TIMER1);
}
/************************************************************
 * Function :       my_timer_set_duty
 * Comment  :       用于设置定时器1的占空比（PWM模式）
 * Parameter:       duty 占空比 0-99%
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2025-12-29 V0.01 original
************************************************************/
void my_timer_set_duty(uint8_t duty) {

    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, (uint32_t)duty * TIMER_ARR / 100);

}

/****************************End*****************************/
