/************************* ͷ�ļ� *************************/
#include "Function.h"
#include "..\Driver\LED\LED.h"
#include "RTC.h"
#include "USART.h"
#include "SPI_FLASH.h"
#include "ff.h"
#include "diskio.h"
#include "sdcard.h"
#include "SDIO\sdio_sdcard.h"
#include "Function.h"
#include "..\Driver\Fatfs\diskio.h"
#include "USART\usart.h"
#include "LED\LED.h"
#include "KEY/key.h"
#include <stdio.h>
#include "string.h"
#include <stdlib.h> 
#include <stdbool.h>
#include "math.h"
#include "systick.h"
#include "DMA/dma.h"
#include "ADC/adc.h"
#include <time.h>
#include "OLED/oled.h"
#include "SDIO/sdio_sdcard.h"
#include "SPI_FLASH\SPI_FLASH.h"
#include "SDIO\sd_conf.h"
#include "SPI/spi.h"
#include "GD30AD3344_standard_peripheral\Include\gd30ad3344.h"
#include "dac.h"
#include "485.h"
#include "ModbusCRC.h"
#include "flash_partition.h"
#include "param.h"
/************************* �궨�� *************************/
#define FIRMWARE_VERSION_MAJOR 2
#define FIRMWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_PATCH 1
#define FIRMWARE_VERSION_BUILD 0
//�汾��

static volatile uint8_t g_upgrade_phase = 0; // 0:�ȴ�0x0502, 1:�ȴ�0x0503

/* �������� */
static void process_command_in_bootloader(const ParsedFrame *frame);
static void receive_firmware_slices(void);
static void execute_upgrade(void);
static void jump_to_app(void);
static void send_error_response(uint16_t cmd_word);
static int parse_raw_frame(uint8_t *raw, uint32_t len, ParsedFrame *frame);
/************************ �������� ************************/

 volatile uint32_t sys_tick_ms = 0 ;   // ϵͳ���������

uint32_t flash_id = 0;
uint8_t  tx_buffer[TX_BUFFER_SIZE];
uint8_t  rx_buffer[TX_BUFFER_SIZE];
uint16_t i = 0, count, result = 0;
uint8_t  is_successful = 0;

uint8_t my_id;
volatile uint16_t g_recv_len = 0;
rtc_parameter_struct rtc_time;

FIL fdst;
FATFS fs_sd;FATFS fs_flash;
UINT br, bw;
BYTE buffer[128];
BYTE filebuffer[128];

bool auto_report_enabled = false;      // �Ƿ������Զ��ϱ�
uint16_t report_interval_ms = 1000;    // �ϱ���������룩��Ĭ�� 1s
uint32_t last_report_tick = 0;         // �ϴη���ʱ��


/************************ �������� ************************/
void systick_config_1ms(void);
void nvic_config(void);
void timer3_init(void);
uint32_t get_current_tick(void);
int recv_frame_nonblocking(ParsedFrame *frame);
int recv_bytes(uint32_t len, uint8_t *buffer, uint32_t timeout_ms);
/************************************************************ 
 * Function :       System_Init
************************************************************/

void System_Init(void)
{ 
	SystemInit();
	systick_config();     // ʱ������
  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	LED_Init();//led��ʼ��
  timer3_init();
	OLED_Init();OLED_Clear();
	OLED_Refresh();//oled��ʼ��
	
	nvic_config();		//�����жϿ�����
	
  param_init();//������ʼ��or����
 
  RingBuffer_Init(&recv_485_rb, BUFFER_SIZE); 
	usart_485_init(param_get_baudrate());
	//485��ʼ��
		param_init();//������ʼ��or����
		systick_config_1ms();
    LED4_ON();

}

/************************************************************ 
 * Function :       UsrFunction
************************************************************/
void UsrFunction(void)
{ OLED_ShowString(0, 1, (unsigned char *)"CIMC 2026", 12);
	OLED_ShowString(0, 14, (unsigned char *)"Bootloader", 12);
	OLED_Refresh();//oled��ʾ
	my_id = param_get_device_id();

	uint8_t upgrade_requested = param_get_upgrade_flag(); // 0x01 ����������
   if (upgrade_requested) {
        // ==== ����ģʽ��10�뵹��ʱ�ȴ� 0x0502 ====
        int32_t countdown = 10;
        g_upgrade_phase = 0;

        usart_485_send_str((uint8_t*)"using command to interrupt start Application\r\n",47);
        while (countdown > 0) {
            char countdown_msg[64];
            sprintf(countdown_msg, "wait for start Application(%ds)...\r\n", countdown);
            usart_485_send_str((uint8_t*)countdown_msg, strlen(countdown_msg));

            // ÿһ���ڳ�����ѯ���ڣ�����Ƿ��յ� 0x0502
            for (int i = 0; i < 1000; i++) {
                delay_1ms(1);

                ParsedFrame rx_frame;
                if (recv_frame_nonblocking(&rx_frame)) {
                    process_command_in_bootloader(&rx_frame);
                    // ��� g_upgrade_phase ��Ϊ 2��˵��������ȫ����ɲ���ת App
                    if (g_upgrade_phase == 2) {
                        // ��ʱ����ת�� App������ص�����
                    }
                }

                // ����Ѿ���ʼ���չ̼������� 0x0502 ������������ʱ�Ͳ��ټ���
                if (g_upgrade_phase == 1) {
                    break;
                }
            }
            if (g_upgrade_phase == 1) break; // ��������ʱѭ��������̼���������
            countdown--;
        }

        // ����ʱ�������ѿ�ʼ���չ̼���������ת App������̼����ղ�������ɻ��Լ���ת��
        jump_to_app();
    } else {
        // ==== ��������ģʽ����ʱ 5 �����ת App ====
        delay_1ms(5000);
        jump_to_app();
    }
		
		
		
	}

void nvic_config(void)
{
//    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);	// �����ж����ȼ�����
    nvic_irq_enable(SDIO_IRQn, 0, 0);					// ʹ��SDIO�жϣ����ȼ�Ϊ0
}

void timer3_init(void)
{
    /* ʹ��ʱ�� */
    rcu_periph_clock_enable(RCU_TIMER3);                     /* TIMER3ʱ�� */
    rcu_periph_clock_enable(RCU_GPIOA);                      /* GPIOAʱ�� (����LED2) */

    /* ---------- ���� TIMER3 ---------- */
    timer_parameter_struct timer_initpara;
    timer_deinit(TIMER3);
    timer_struct_para_init(&timer_initpara);

/*
     * ��ʱ��ʱ�� = 120 MHz (APB1 ��ʱ��)
     * Ԥ��Ƶ: 12000 - 1 �� ������ʱ�� = 120 MHz / 12000 = 10 kHz
     * �Զ���װ: 5000 - 1 �� �������� = 5000 / 10 kHz = 0.5 s
     */
    timer_initpara.prescaler         = 12000 - 1;      /* 10 kHz */
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 5000 - 1;       /* 0.5 s */
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3, &timer_initpara);


    /* ��������жϱ�־����ʹ�ܸ����ж� */
    timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER3, TIMER_INT_UP);

    /* ����NVIC�ж����ȼ� (�ɸ�����Ҫ����) */
    nvic_irq_enable(TIMER3_IRQn, 1, 0);     /* ��ռ���ȼ�1�������ȼ�0 */

    /* ������ʱ�� */
    timer_enable(TIMER3);
}
void TIMER3_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP) == SET)
    {
        /* ��������жϱ�־ */
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);

        /* ��תLED2״̬��ʵ��������˸ */
        static uint8_t led_toggle = 0;
        led_toggle ^= 1;
        if (led_toggle)
        {
            LED2_ON();
        }
        else
        {
            LED2_OFF();
        }
    }
}



void systick_config_1ms(void)
{
    /* ��������ֵ��ϵͳ HCLK Ƶ�� / 1000 - 1 */
    /* GD32F4xx Ĭ�� HCLK = 200 MHz������ʹ������Ƶ�����޸� */
    uint32_t reload_val = SystemCoreClock / 1000U;
    
    /* ���㵱ǰֵ */
    SysTick->VAL  = 0UL;
    
    /* ���� SysTick������ֵ��ʹ���ں�ʱ�ӡ�ʹ���ж� */
    SysTick->LOAD = reload_val - 1UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |   // ʹ���ں�ʱ�ӣ�HCLK��
                    SysTick_CTRL_TICKINT_Msk   |   // ʹ���ж�
                    SysTick_CTRL_ENABLE_Msk;       // ������ʱ��
    
    /* ���� SysTick �ж����ȼ�Ϊ��ͣ���ֵԽ�����ȼ�Խ�ͣ� */
    NVIC_SetPriority(SysTick_IRQn, 0x0FUL);
}
uint32_t get_current_tick(void)
{
    return sys_tick_ms;
}


static void process_command_in_bootloader(const ParsedFrame *frame)
{
    // �豸 ID ƥ�䴦��������豸 ID ���Ǳ��� ID Ҳ���ǹ㲥 0xFFFF��ֱ�Ӷ���
    if (frame->dev_id != param_get_device_id() && frame->dev_id != 0xFFFF) {
        return; // ���ظ�
    }

    // ֡����������������ж�
    switch (frame->cmd_word) {
			  case 0x0501:   // ��ѯBootloader״̬
  {
      uint8_t ok = 0xFF;
      send_response(my_id, 0x02, 0x0501, &ok, 1);
      break;
  }
        case 0x0502:   // ׼������̼����ݰ�
        {
            if (g_upgrade_phase != 0) {
                // ״̬���ԣ��ɺ��Ի��ʹ���
                break;
            }
            // 1. �ظ� OK
            uint8_t ok = 0xFF;
            send_response(my_id, 0x02, 0x0502, &ok, 1);

            // 2. ����ѽ���̼�����״̬
            g_upgrade_phase = 1;

            // 3. ִ�й̼���Ƭ���գ�������
            receive_firmware_slices();
            break;
        }

        case 0x0503:   // ִ����������
        {
            // �����Ѿ��� g_upgrade_phase == 1���Ѿ�������̼���ʱ�Ŵ���
            if (g_upgrade_phase != 1) {
               send_error_response(frame->cmd_word);
                break;
            }
            // 1. �ظ� OK
            uint8_t ok = 0xFF;
            send_response(my_id,0x02, 0x0503, &ok, 1);

            // 2. ִ�й̼����ˣ�������
            execute_upgrade();

            // 3. ������ɲ������־��ֱ����ת App�����ٷ�����ѭ����
            // ��Ϊ�˰�ȫ�����ǿ����� execute_upgrade �ڲ������ת
            // Ȼ������ѭ����ͨ�� g_upgrade_phase = 2 ��ʾ�����
            // �˴����� execute_upgrade �ڲ������� jump_to_app
            break;
        }

        default:
        {
            send_error_response(frame->cmd_word);
            break;
        }
    }
}

static void receive_firmware_slices(void)
{
    const uint32_t fw_size = APP_AREA_SIZE;          // 128KB
    const uint32_t slice_len = 256;                  // ÿƬ 256 �ֽ�
    const uint32_t slice_num = fw_size / slice_len;  // 512 Ƭ

    uint32_t addr = FW_STAGING_AREA_BASE_ADDR;

    // �����̼��ݴ�����0x08051000 ~ 0x08070FFF��
    fmc_unlock();
    __disable_irq();
    for (uint32_t off = 0; off < FW_STAGING_AREA_SIZE; off += FLASH_PAGE_SIZE) {
        fmc_page_erase(FW_STAGING_AREA_BASE_ADDR + off);
    }
    __enable_irq();
    fmc_lock();

    // ѭ������ÿһƬ
    fmc_unlock();
    usart_interrupt_disable(USART_485_, USART_INT_IDLE);
    for (uint32_t i = 0; i < slice_num; i++) {
        uint8_t slice[256];
        // ���� 256 �ֽ������ݣ���ʱ 5 ��
        if (recv_bytes(256, slice, 5000) != 0) {
            // ��ʱ�����ʹ���֡
            __enable_irq();
            fmc_lock();
            send_error_response(0x0502);
            return;
        }
        // �� 256 �ֽڣ�64 ���֣�д�� Flash
        for (uint32_t j = 0; j < 64; j++) {
            fmc_word_program(addr, ((uint32_t*)slice)[j]);
            addr += 4;
        }
        // �����ӽ���ָʾ��ȴ� 5ms��ͨ����λ����ȴ���
        delay_1ms(5);
    }
    usart_interrupt_enable(USART_485_, USART_INT_IDLE);
    fmc_lock();

    // ��������������� ASCII ����������ֹ�����ƹ̼����ݸ���
    RingBuffer_Reset(&recv_485_rb);
    reset_ascii_parser();

    // ������ɣ�У��ħ���֣��������ڹ̼�ǰ 4 �ֽڣ����� 0xABCD0001��
    uint32_t magic_read = *(volatile uint32_t*)FW_STAGING_AREA_BASE_ADDR;
    if (magic_read == g_param.magic) {  
        uint8_t ok = 0xFF;
        send_response(my_id,0x02, 0x0502, &ok, 1);
    } else {
        send_error_response(0x0502);
    }
}

static void execute_upgrade(void)
{
    // 1. ���� App ����0x08011000 ~ 0x08030FFF��
    fmc_unlock();
    __disable_irq();
    for (uint32_t addr = APP_AREA_BASE_ADDR; addr < APP_AREA_BASE_ADDR + APP_AREA_SIZE; addr += FLASH_PAGE_SIZE) {
        fmc_page_erase(addr);
    }

    // 2. ���ݴ������ֿ����� App ��
    uint32_t *src = (uint32_t*)FW_STAGING_AREA_BASE_ADDR;
    uint32_t *dst = (uint32_t*)APP_AREA_BASE_ADDR;
    for (uint32_t i = 0; i < APP_AREA_SIZE / 4; i++) {
        fmc_word_program((uint32_t)&dst[i], src[i]);
    }
    __enable_irq();
    fmc_lock();

    // 3. ���������־�������� upgrade_requested = 0��
    fmc_unlock();
    // ֱ��д���������ֽڵ�λ�ã��Ƽ�ֻ��һ���ֽڣ�����ע�� Flash ���ԣ�
    uint32_t flag_addr = PARAM_AREA_BASE_ADDR + offsetof(ParameterBlock, upgrade_requested);
    fmc_word_program(flag_addr, 0);
    fmc_lock();

    // 4. ��ת���� App
    jump_to_app();
}

static void jump_to_app(void)
{
    uint32_t app_sp = *(volatile uint32_t*)APP_AREA_BASE_ADDR;
    uint32_t app_pc = *(volatile uint32_t*)(APP_AREA_BASE_ADDR + 4);

    typedef void (*app_entry_t)(void);

    /* �����Ϸ��Լ�飺MSP �� SRAM��PC �� APP Flash �� */
    if ((app_sp & 0x2FFE0000U) != 0x20000000U) return;
    if (app_pc < APP_AREA_BASE_ADDR || app_pc > APP_AREA_END_ADDR) return;

    __disable_irq();

    /* �ر� SysTick������ Bootloader �� SysTick ״̬���� APP */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* �� NVIC ʹ�ܺ� pending������ Bootloader �����ж�Ӱ�� APP */
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    /* APP ��������ʼ��ַ */
    SCB->VTOR = APP_AREA_BASE_ADDR;

    __set_MSP(app_sp);
    __DSB();
    __ISB();

    /* �ص㣺���ܴ��� PRIMASK=1 ���� APP */
    __enable_irq();

    ((app_entry_t)app_pc)();
}

static void send_error_response(uint16_t cmd_word)
{
    // ����Ӧ��֡��֡���� 0xFF�������� 0xEEEE������Ϊ��
    uint8_t dummy = 0;
    send_response(my_id,0xFF, 0xEEEE, &dummy, 0);
}

/**
 * @brief ��������ȡһ֡������ recv_485_flag �� g_recv_len��
 * @param frame ����ṹ��
 * @return 1�ɹ���0��֡�����ʧ��
 */
int recv_frame_nonblocking(ParsedFrame *frame)
{
    if (!recv_485_flag) {
        return 0;   // ����֡
    }

    // ֱ�Ӵ� RingBuffer ��ȡ������ ASCII ʮ������֡
    int result = parse_ascii_frame_from_rb(&recv_485_rb, frame);
    if (result == 1) {
        // �����ɹ�
        recv_485_flag = 0;
        g_recv_len = 0;
        return 1;
    } else if (result == -1) {
        // CRC ����ʧ��
        recv_485_flag = 0;
        g_recv_len = 0;
        return 0;
    }
    // result == 0: ���ݲ��������ȴ��´���ѯ
    return 0;
}
/**
 * @brief ��������ָ���ֽ�������ʱ���� -1
 * @param len �����ֽ���
 * @param buffer ���ջ�����
 * @param timeout_ms ��ʱ������
 * @return 0�ɹ�, -1��ʱ
 */
int recv_bytes(uint32_t len, uint8_t *buffer, uint32_t timeout_ms)
{
    uint32_t start = get_current_tick();
    uint32_t received = 0;

    while (received < len) {
        uint32_t avail = RingBuffer_Used(&recv_485_rb);

        if (avail >= (len - received)) {
            // һ���Զ�ȡ�����ʣ���ֽ�
            __disable_irq();
            uint32_t cnt = RingBuffer_Read(&recv_485_rb, buffer + received, len - received);
            __enable_irq();
            received += cnt;
            break;   // ���꣬�˳�
        } else if (avail > 0) {
            // �����ݵ����㣬�ȶ�һ����
            __disable_irq();
            uint32_t cnt = RingBuffer_Read(&recv_485_rb, buffer + received, avail);
            __enable_irq();
            received += cnt;
        }

        // ��ʱ�ж�
        if (get_current_tick() - start > timeout_ms) {
            return -1;   // ��ʱ
        }

        // ���ݵȴ�������æ��
        delay_1ms(1);   // �����е� 1ms ��ʱ
    }

    return 0;
}
static int parse_raw_frame(uint8_t *raw, uint32_t len, ParsedFrame *frame)
{
    // ��С���ȣ�֡ͷ2 + �豸ID2 + ֡����1 + ������2 + �汾1 + ���ݳ���1 + ����0 + CRC2 + ֡β2 = 13
    if (len < 13) return -1;

    // ���֡ͷ A5 B6
    if (raw[0] != 0xA5 || raw[1] != 0xB6) return -1;
    // ���֡β B6 A5
    if (raw[len-2] != 0xB6 || raw[len-1] != 0xA5) return -1;

    // ��ȡ�ֶΣ����������
    uint16_t dev_id = (raw[2] << 8) | raw[3];
    uint8_t  frame_type = raw[4];
    uint16_t cmd_word = (raw[5] << 8) | raw[6];
    uint8_t  content_len = raw[7];
    uint8_t  version = raw[8];

    // У�飺���ݳ��� + �̶����ֳ��� + CRC + ֡β �Ƿ�����ܳ�
    uint32_t expected_len = 2 + 2 + 1 + 2 + 1 + 1 + content_len + 2 + 2; // 12 + content_len
    if (len < expected_len) return -1;  // ���ݲ�����

    // ���ݿ���
    frame->dev_id      = dev_id;
    frame->frame_type  = frame_type;
    frame->cmd_word    = cmd_word;
    frame->version     = version;
    frame->content_len = content_len;
    if (content_len > 0) {
        memcpy(frame->content, &raw[9], content_len);
    }

    // ����ѡ��У�� CRC���˴�ʡ��
    return 0;
}
/****************************End*****************************/

