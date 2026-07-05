/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：timer.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/1/4     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "timer.h"
#include "LED.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/
uint8_t led_state_flag = 0;

/************************ 函数定义 ************************/
/************************************************************
 * Function :       my_timer_init
 * Comment  :       初始化基本定时器 TIM6
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-01-04 V0.1 original
************************************************************/
void my_timer_init(void)
{

	//初始化LED
	LED_Init();

	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	nvic_irq_enable(TIMER6_IRQn , 2 , 0);

	rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

	timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable(RCU_TIMER6);

	timer_deinit(TIMER6);

	timer_initpara.prescaler = 2400 - 1;			//分频2400
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;	//边沿对齐模式
	timer_initpara.counterdirection = TIMER_COUNTER_UP;	//向上计数模式
	timer_initpara.period = 50000 - 1;			//自动重装载值50000
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;	//时钟分频1
	timer_initpara.repetitioncounter = 0;		//重复计数器0
	timer_init(TIMER6 , &timer_initpara);			//初始化定时器6

	timer_interrupt_enable(TIMER6 , TIMER_INT_UP);
	timer_interrupt_flag_clear(TIMER6 , TIMER_INT_UP);

	timer_enable(TIMER6);

}

/************************************************************
 * Function :       TIMER6_IRQHandler
 * Comment  :       定时器6中断服务函数
 * Parameter:       null
 * Return   :       null
 * Author   :       Jialei Zhao
 * Date     :       2026-01-04 V0.1 original
************************************************************/
void TIMER6_IRQHandler(void)
{
	//功能
	//每触发一次定时器更新中断，切换一下LED3的状态
	if (timer_interrupt_flag_get(TIMER6 , TIMER_INT_UP) == SET)
	{
		timer_interrupt_flag_clear(TIMER6 , TIMER_INT_UP);
		if (!led_state_flag)
		{
			LED3_ON();
			led_state_flag = 1;
		}
		else
		{
			LED3_OFF();
			led_state_flag = 0;
		}

	}
}

/****************************End*****************************/
