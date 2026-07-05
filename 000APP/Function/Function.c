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
/* �����ж� */
#define IS_LEAP(y) ((((y) % 4 == 0) && ((y) % 100 != 0)) || ((y) % 400 == 0))

/* ÿ����������ƽ��/���꣩ */
static const uint8_t days_in_mon[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},   /* ƽ�� */
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}    /* ���� */
};

#define GROUP_SIZE 10
#define WRITE_BCD(val)  ((val / 10) << 4) + (val % 10)
#define READ_BCD(val) ((val >> 4) * 10 + (val & 0x0F))

/************************ �������� ************************/

 volatile uint32_t sys_tick_ms = 0 ;   // ϵͳ���������
uint8_t my_id;

rtc_parameter_struct rtc_time;
bool auto_report_enabled = false;      // �Ƿ������Զ��ϱ�
uint16_t report_interval_ms = 1000;    // �ϱ���������룩��Ĭ�� 1s
uint32_t last_report_tick = 0;         // �ϴη���ʱ��


/************************ �������� ************************/
void nvic_config(void);
void write_file(void);
void RTC_init();
void timer3_init(void);
void deepsleep_with_rtc_alarm(uint32_t seconds);
void utc_seconds_to_rtc(uint32_t sec, rtc_parameter_struct *rtc);
uint32_t rtc_to_utc_seconds(const rtc_parameter_struct *rtc);
void send_auto_report_frame(void);
void systick_config_1ms(void);
uint32_t get_current_tick(void);
void write_float_big_endian(float value, uint8_t *buf);
float read_float_big_endian(const uint8_t *buf);
void check_and_handle_alarm(uint8_t channel, float value);
/************************************************************ 
 * Function :       System_Init
************************************************************/

void System_Init(void)
{ nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x11000);// ��ӳ���ж��������� App ������ʼ��ַ
	SystemInit();
	systick_config();     // ʱ������
	
	usart_init();	my_dma_init();		// DMA��ʼ��
	nvic_config();
	LED_Init();//led��ʼ��
  timer3_init();
	OLED_Init();OLED_Clear();//oled��ʼ��
//	uint16_t k = 5;
//	DSTATUS stat = 0;
	nvic_config();		//�����жϿ�����
	
	param_init();//������ʼ��or����
  
  RingBuffer_Init(&recv_485_rb, BUFFER_SIZE);
	usart_485_init(param_get_baudrate());
	//485��ʼ��
	
//	
//  spi_flash_init();
//  flash_id = spi_flash_read_id();
//	printf("\n\rThe Flash_ID:0x%X\n\r",flash_id);
//	//flash��ʼ
//	
//	do
//	{
//		stat = disk_initialize(0); 			//��ʼ��SD�����豸��0��,�������������,ÿ����������������ȣ�ͨ����������һ��Ӳ�̡�U ��Ψһ�ı�š�
//	}while((stat != 0) && (--k));			//�����ʼ��ʧ�ܣ��������k�Ρ�
//    
//    printf("SD Card disk_initialize:%d\r\n",stat);
//    f_mount(0, &fs_sd);						 //����SD�����ļ�ϵͳ���豸��0����
//    printf("SD Card f_mount:%d\r\n",stat);
//		//sdcard��ʼ��
//		
	  spi3_init();

    ad3344_init(AD3344_CONFIG);
   	AD3344_reg_Config(AD3344_SINGLE_END, 0);
	  delay_1ms(2);
		timer2_init_for_adc();
   //�ⲿadc��ʼ��
		
		ADC_Init();ADC0_Init();
		//adc (ch0)& adc(ch1) & dac��ʼ��
		
	  RTC_init();
		rtc_register_sync_wait();
		rtc_current_time_get(&rtc_time);
//    printf("%04d-%02d-%02d %02d:%02d:%02d\r\n",
//						 READ_BCD(rtc_time.year) + 2000, 
//						 READ_BCD(rtc_time.month), 
//						 READ_BCD(rtc_time.date),
//						 READ_BCD(rtc_time.hour),
//						 READ_BCD(rtc_time.minute),
//						 READ_BCD(rtc_time.second));
	 //RTC ��ʼ��	
		systick_config_1ms();
		LED4_ON();
		__enable_irq();
		char buf[64];

}

/************************************************************ 
 * Function :       UsrFunction
************************************************************/
void UsrFunction(void)
{ OLED_ShowString(0, 1, (unsigned char *)"CIMC 2026", 12);
	OLED_ShowString(0, 14, (unsigned char *)"APP", 12);
	OLED_Refresh();//oled��ʾ
	my_id = param_get_device_id();
	
	//������ȡ
	while (1) {
	ParsedFrame rx_frame;
  int ret = parse_ascii_frame_from_rb(&recv_485_rb, &rx_frame);

	if (auto_report_enabled && (rx_frame.cmd_word != 0x0303))
{
    // �����Զ��ϱ������յ��Ĳ���ֹͣ���� �� ֱ�Ӻ��ԣ����ظ��κζ���
    continue; // ��������ѭ������������������
}
	if (auto_report_enabled)
    {
        uint32_t now = get_current_tick(); // ��ȡ��ǰ ms ʱ��
        if (now - last_report_tick >= report_interval_ms)
        {
            send_auto_report_frame();
            last_report_tick = now;
        }
    }
	
  if (ret == 1) {
            // ֡�����ɹ�������������ִ�ж�Ӧ����
        if (rx_frame.dev_id != 0xFFFF && rx_frame.dev_id != my_id)
                continue; // ID ��ƥ�䣬�������㲥 0xFFFF ���⣩

            // ��������֡ 
        if (rx_frame.frame_type == 0x01) {//ϵͳ������
                switch (rx_frame.cmd_word) {
                    case 0x0101: // ����
                        send_response(my_id, 0x02, 0x0101, (unsigned char[]){0xFF}, 1);//ok֡
                        delay_1ms(10);
                        NVIC_SystemReset();
                        break;
										case 0x0102://�ָ��������� (Ԥ��)
											break;
										case 0x0103://��ѯ�豸��Ϣ (Ԥ��)
											break;
										case 0x0104:{//��ѯ�̼��汾
												 unsigned char version_content[4];
													version_content[0] = FIRMWARE_VERSION_MAJOR;  
													version_content[1] = FIRMWARE_VERSION_MINOR;  
													version_content[2] = FIRMWARE_VERSION_PATCH;  
													version_content[3] = FIRMWARE_VERSION_BUILD;  //�汾��ƴ��
                          send_response(my_id, 0x02, 0x0104, version_content, 4);

											break;}
										case 0x0105://�����豸ʱ��
											send_response(my_id, 0x02, 0x0105, (unsigned char[]){0xFF}, 1);//ok֡
											unsigned int utc_sec = (rx_frame.content[0] << 24) | (rx_frame.content[1] << 16) |
                           (rx_frame.content[2] << 8)  | rx_frame.content[3];
											utc_seconds_to_rtc(utc_sec,&rtc_time);
											pmu_backup_write_enable();

											if (RCU_BDCTL & RCU_RTCSRC_LXTAL) {
													while (!(RCU_BDCTL & RCU_FLAG_LXTALSTB)) {
															delay_1ms(10);
													}
											} else if (RCU_BDCTL & RCU_RTCSRC_IRC32K) {
												 
													delay_1ms(10);
											} 
										
											rcu_periph_clock_enable(RCU_RTC);
											rtc_init_mode_enter();
											rtc_parameter_struct rtc_initpara;
											memset(&rtc_initpara, 0, sizeof(rtc_initpara)); 
											rtc_initpara.factor_asyn = 127;                                 
											rtc_initpara.factor_syn  = 255;  
											rtc_initpara.year = WRITE_BCD(rtc_time.month);
											rtc_initpara.month = WRITE_BCD(rtc_time.month);
											rtc_initpara.date = WRITE_BCD(rtc_time.date);
											rtc_initpara.hour = WRITE_BCD(rtc_time.hour);
											rtc_initpara.minute = WRITE_BCD(rtc_time.minute);
											rtc_initpara.second = WRITE_BCD(rtc_time.second);
											rtc_init_mode_exit();
											rcu_periph_clock_enable(RCU_RTC);
											pmu_backup_write_enable();
											break;
										case 0x0106://��ѯ�豸ʱ��
											rtc_register_sync_wait();
		                	rtc_current_time_get(&rtc_time);
											uint8_t unix = rtc_to_utc_seconds(&rtc_time);
											send_response(my_id, 0x02, 0x0106, &unix, 4);
											break;
										case 0x01A1:{//�����豸 ID
											uint16_t new_id = (rx_frame.content[0] << 8) | rx_frame.content[1];
											param_set_device_id(new_id);
											// ���µ�ǰȫ�ֱ��� my_id
											my_id = new_id;
											send_response(my_id, 0x02, 0x01A1, (unsigned char[]){0xFF}, 1);//ok֡
											break;}
										case 0x01A2:{//���ò�����
											uint8_t baud_rate_code = rx_frame.content[0];
										  param_set_baudrate(baud_rate_code);
                      NVIC_SystemReset();
											break;}
										case 0x0111://��ѯ�豸 ID
											send_response(my_id, 0x02, 0x0111, &my_id, 4);
											break;
										case 0x0112:{//��ѯ������
											uint8_t baud_rate_code=param_get_baudrate();
											send_response(my_id, 0x02, 0x0112, &baud_rate_code, 4);
                      break;}
        }
        }else if ((rx_frame.frame_type == 0x02)){//������
					switch (rx_frame.cmd_word) {
                    case 0x0201: //��ѯ CH0 ����
										{float ch0_ratio = param_get_ch0_ratio();
										float origin = ADC_Read_Register();
											float report_value = origin * ch0_ratio;
											unsigned char content[4];
											union {
													float f;
													unsigned char bytes[4];
											} converter;
											
											converter.f = report_value;
											
											// �����
											content[0] = converter.bytes[3];  // ���λ
											content[1] = converter.bytes[2];
											content[2] = converter.bytes[1];
											content[3] = converter.bytes[0];  // ���λ
											send_response(my_id, 0x02, 0x0201, content, 4);
                        break;}
										case 0x0202://��ѯ CH1 ����
											{float ch1_ratio = param_get_ch1_ratio();
										  float origin = ADC0_Read();
											float report_value = origin * ch1_ratio;
											unsigned char content[4];
											union {
													float f;
													unsigned char bytes[4];
											} converter;
											
											converter.f = report_value;
											
											// �����
											content[0] = converter.bytes[3];  // ���λ
											content[1] = converter.bytes[2];
											content[2] = converter.bytes[1];
											content[3] = converter.bytes[0];  // ���λ
											send_response(my_id, 0x02, 0x0202, content, 4);
                        break;}
										case 0x0221://��ѯ�ⲿADC (PT100) 
										{uint16_t temperature = ad3344_read_regs();
											union {
												float f;
												uint8_t bytes[4];
										} converter;
										converter.f = temperature;
										uint8_t content[4] = {converter.bytes[3], converter.bytes[2],
																					converter.bytes[1], converter.bytes[0]};
										send_response(my_id, 0x02, 0x0221, content, 4);
										}
											break;
										case 0x0241://���� CH0 ���
										{send_response(my_id, 0x02, 0x0241, (unsigned char[]){0xFF}, 1);//ok֡
											
											// ���������������
											if (rx_frame.content_len != 4)
											{
													// ���ش���֡
													send_response(my_id, 0xFF, 0x0241, NULL, 0);
													break;
											}
                    uint32_t raw = ((uint32_t)rx_frame.content[0] << 24) |  // ����ֽ��Ƶ����λ
                   ((uint32_t)rx_frame.content[1] << 16) |  // �θ��ֽ�
                   ((uint32_t)rx_frame.content[2] <<  8) |  // �ε��ֽ�
                   ((uint32_t)rx_frame.content[3]);          // ����ֽ�

										float ch0_ratio = *(float *)&raw; 
										param_set_ch0_ratio(ch0_ratio);
											break;}
										case 0x0242://���� CH1 ���
										{send_response(my_id, 0x02, 0x0242, (unsigned char[]){0xFF}, 1);//ok֡
											
											// ���������������
											if (rx_frame.content_len != 4)
											{
													// ���ش���֡
													send_response(my_id, 0xFF, 0x0242, NULL, 0);
													break;
											}
                       uint32_t raw = ((uint32_t)rx_frame.content[0] << 24) |  // ����ֽ��Ƶ����λ
                      ((uint32_t)rx_frame.content[1] << 16) |  // �θ��ֽ�
                      ((uint32_t)rx_frame.content[2] <<  8) |  // �ε��ֽ�
                      ((uint32_t)rx_frame.content[3]);          // ����ֽ�

										float ch1_ratio = *(float *)&raw; 
										param_set_ch1_ratio(ch1_ratio);
											break;}
										case 0x0261:  // ���������ϱ�ʱ����
										{
												// 1. У�����ݳ��ȣ�ӦΪ 1 �ֽ�
												if (rx_frame.content_len != 1)
												{
														// ���ȴ��󣬻ظ�����֡
														send_response(my_id, 0xFF, 0x0261, NULL, 0);
														break;
												}

												// 2. ��ȡʱ����ӳ��ֵ
												unsigned char interval_code = rx_frame.content[0];

												// 3. ����ӳ��ֵ����Ϊʵ�������������浽ȫ�ֱ���
												switch (interval_code)
												{
														case 0x01:
																report_interval_ms = 1000;   // 1 ��
																break;
														case 0x02:
																report_interval_ms = 3000;   // 3 ��
																break;
														case 0x03:
																report_interval_ms = 5000;   // 5 ��
																break;
														default:
																// �Ƿ����룬�ظ�����֡
																send_response(my_id, 0xFF, 0x0261, NULL, 0);
																break;
												}
														unsigned char ok = 0xFF;
														send_response(my_id, 0x02, 0x0261, &ok, 1);
											
												break;
										}

        }
				}else if ((rx_frame.frame_type == 0x03)){//������
					switch (rx_frame.cmd_word) {
                    case 0x0301: //���� DAC �����ѹ
										{uint16_t dac_value = ((uint16_t)rx_frame.content[0] << 8) | rx_frame.content[1];

										// ��鷶Χ��0~4095��0x0000 ~ 0x0FFF��
										if (dac_value > 4095) {
												// ������Χ���ظ�����֡
												send_response(my_id, 0xFF, 0x0301, NULL, 0);
												break;
										}
										/* set DAC output data */
										dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, dac_value);
										/* enable DAC software trigger */
										dac_software_trigger_enable(DAC0, DAC_OUT0);
                      break;}
										case 0x0302://��ʱ�Զ��ϱ����ݿ�ʼ
											send_auto_report_frame();

											// �����Զ��ϱ���־
											auto_report_enabled = true;
											last_report_tick = get_current_tick(); // ��¼��ǰʱ��
											
											break;
										case 0x0303://��ʱ�Զ��ϱ�����ֹͣ
										{ unsigned char ok = 0xFF;
											send_response(my_id, 0x02, 0x0303, &ok, 1);
										  auto_report_enabled = false;
											break;}
										case 0x03AA://����˯��ģʽ
										send_response(my_id, 0x02, 0x03AA, (unsigned char[]){0xFF}, 1);//ok֡
										deepsleep_with_rtc_alarm(10);
											break;
				}
				}else if ((rx_frame.frame_type == 0x04)){//����������
						switch (rx_frame.cmd_word) {
								case 0x0400: // ��ȡ��ֵ������������CH0 + CH1��
								{
										uint8_t resp_data[8];
										float ch0_th = param_get_ch0_threshold();
										float ch1_th = param_get_ch1_threshold();
										write_float_big_endian(ch0_th, &resp_data[0]);
										write_float_big_endian(ch1_th, &resp_data[4]);
										send_response(my_id,0x02, 0x0400, resp_data, 8);
										break;
								}

								case 0x0401: // ��ȡ CH0 ��ֵ����
								{
										uint8_t resp_data[4];
										write_float_big_endian(param_get_ch0_threshold(), resp_data);
										send_response(my_id,0x02, 0x0401, resp_data, 4);
										break;
								}

								case 0x0402: // ��ȡ CH1 ��ֵ����
								{
										uint8_t resp_data[4];
										write_float_big_endian(param_get_ch1_threshold(), resp_data);
										send_response(my_id,0x02, 0x0402, resp_data, 4);
										break;
								}

								case 0x0403: // ��ȡ CH2 ��ֵ����
								{
										uint8_t resp_data[4];
										write_float_big_endian(param_get_ch2_threshold(), resp_data);
										send_response(my_id,0x02, 0x0403, resp_data, 4);
										break;
								}

								case 0x0411: // д�� CH0 ��ֵ����
								{
										// �� rx_data ȡ 4 �ֽ� IEEE 754 ��˸�����
										float new_th = read_float_big_endian(rx_frame.content);
										param_set_ch0_threshold(new_th);  
										uint8_t ok = 0xFF;               // OK �ظ�
										send_response(my_id,0x02, 0x0411, &ok, 1);
										break;
								}

								case 0x0412: // д�� CH1 ��ֵ����
								{
										float new_th = read_float_big_endian(rx_frame.content);
										param_set_ch1_threshold(new_th);  
										uint8_t ok = 0xFF;
										send_response(my_id,0x02, 0x0412, &ok, 1);
										break;
								}

								case 0x0413: // д�� CH2 ��ֵ����
								{
										float new_th = read_float_big_endian(rx_frame.content);
										param_set_ch2_threshold(new_th);  
										uint8_t ok = 0xFF;
										send_response(my_id,0x02, 0x0413, &ok, 1);
										break;
								}
				}	
				}else if ((rx_frame.frame_type == 0x06)){//�澯����־��
											switch (rx_frame.cmd_word) {
											case 0x0601://�����Ƿ������ϱ��澯
												{
												//  0x01 ����, 0x02 ������
												uint8_t enable = rx_frame.content[0];
												if (enable == 0x01 || enable == 0x02) {
														param_set_alarm_push_en(enable); 
														uint8_t ok = 0xFF;
														send_response(my_id,0x02, 0x0601, &ok, 1);
												} else {
														// �Ƿ��������ɻظ�����֡
														send_response(0xFFFF, 0xFF, 0xEEEE, NULL, 0);
												}
												break;
												}
											case 0x0602://��ѯ�澯��¼
											{
													char output[1024] = {0};
													uint8_t idx = g_param.alarm_write_index; // ��ǰд��λ��
													int has_record = 0;

													// �����µ����ɵ���
													for (int i = 0; i < ALARM_RECORD_COUNT; i++) {
															// �������� (����)
															int rec_idx = (idx - 1 - i + ALARM_RECORD_COUNT) % ALARM_RECORD_COUNT;
															if (strlen(g_param.alarm_records[rec_idx]) > 0) {
																	strcat(output, g_param.alarm_records[rec_idx]);
																	strcat(output, "\n"); // ÿ��ĩβ����
																	has_record = 1;
															}
													}

													// ֱ�ӷ��� ASCII �ַ���
													if (has_record) {
															usart_485_send_str((uint8_t*)output,4);
													} else {
															usart_485_send_str((uint8_t*)"empty",4);
													}
													break;
											}

											case 0x0603://����澯
												{
													param_clear_alarm(); 
													uint8_t ok = 0xFF;
													send_response(my_id,0x02, 0x0603, &ok, 1);
													break;
											}
											case 0x0604://��ѯ������־ (Ԥ��)
												break;
											case 0x0605://���������־ (Ԥ��)
												break;
				}	
				}else if (rx_frame.frame_type == 0x05 && rx_frame.cmd_word == 0xFFFF) {
             // �㲥Ѱַ���ظ�����
            send_response(my_id, 0x05, 0x8888, NULL, 0);
        
       } else if (ret == -1) {
            // CRC/֡��ʽ���󣬷��ʹ���Ӧ��֡
            send_response(0xFFFF, 0xFF, 0xEEEE, NULL, 0);
        } else if ((rx_frame.frame_type == 0x05)){//ϵͳ������
					switch (rx_frame.cmd_word) {
										case 0x0501://��������
											send_response(my_id, 0x02, 0x0501, (unsigned char[]){0xFF}, 1);//ok֡
											g_param.upgrade_requested = 1;
											param_save();
											NVIC_SystemReset();
												break;
											case 0x0502://׼������̼����ݰ�
												break;
											case 0x0503://ִ����������
												send_response(my_id, 0x02, 0x0503, (unsigned char[]){0xFF}, 1);//ok֡
												break;
										}}
	
	if (auto_report_enabled) {
													uint32_t now = get_current_tick();
													if (now - last_report_tick >= report_interval_ms) {
															send_auto_report_frame();     
															last_report_tick = now;}
														}
}
}}

void nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);	// �����ж����ȼ�����
    nvic_irq_enable(SDIO_IRQn, 0, 0);					// ʹ��SDIO�жϣ����ȼ�Ϊ0
}

void RTC_init(){
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
		uint32_t timeout = 1000000;
    rcu_osci_on(RCU_LXTAL);
    while(SUCCESS != rcu_osci_stab_wait(RCU_LXTAL)&& timeout--){delay_1ms(1);}; 
		if(timeout == 0) {
				printf("RTC sync timeout!\n");}
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    rcu_periph_clock_enable(RCU_RTC);
		timeout=1000000;
		while(SUCCESS != rtc_register_sync_wait() && timeout--) {
				delay_1ms(1);
		}if(timeout == 0) {
				printf("RTC sync timeout!\n");}
		
    #define BKP_VALUE 0x5A5A 
    if((BKP_VALUE != RTC_BKP0) || (0x00 == GET_BITS(RCU_BDCTL, 8, 9))) {
        rtc_parameter_struct rtc_initpara = {0};
       
   
        rtc_initpara.year = WRITE_BCD(26);        
        rtc_initpara.month = WRITE_BCD(6);       
        rtc_initpara.date = WRITE_BCD(6);        
        rtc_initpara.day_of_week = WRITE_BCD(6);  
        rtc_initpara.hour = WRITE_BCD(10);        
        rtc_initpara.minute = WRITE_BCD(30);      
        rtc_initpara.second = WRITE_BCD(0);      
				
        rtc_initpara.display_format = RTC_24HOUR; 
        rtc_initpara.am_pm = RTC_AM;
        rtc_initpara.factor_asyn = 0x7F;   // = 127
        rtc_initpara.factor_syn = 0xFF;    // = 255
        // (32768 / (127+1)) / (255+1) = 1 Hz
				
        RTC_BKP0 = BKP_VALUE;
    } 
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
void deepsleep_with_rtc_alarm(uint32_t seconds)
{
    //  ��ȡ��ǰʱ�䣬��������ʱ��
    rtc_parameter_struct current_time;
    rtc_parameter_struct alarm_time;

    rtc_current_time_get(&current_time);

    uint8_t sec  = READ_BCD(current_time.second);
    uint8_t min  = READ_BCD(current_time.minute);
    uint8_t hour = READ_BCD(current_time.hour);
    uint8_t date = READ_BCD(current_time.date);
    uint8_t mon  = READ_BCD(current_time.month);
    uint16_t year = READ_BCD(current_time.year) + 2000; 
    sec += (uint8_t)seconds;
    while (sec >= 60) { sec -= 60; min++; }
    while (min >= 60) { min -= 60; hour++; }
    while (hour >= 24) { hour -= 24; date++; }

//  ���� RTC ���� 0
    rtc_alarm_struct alarm_config;
		
    alarm_config.alarm_mask     = RTC_ALARM_ALL_MASK;   // �Ƚ������ֶΣ���/��/��/ʱ/��/�룩
    alarm_config.weekday_or_date = RTC_ALARM_DATE_SELECTED; // ʹ������
    alarm_config.alarm_day      = WRITE_BCD(current_time.date);        // BCD ��ʽ
    alarm_config.alarm_hour     = WRITE_BCD(current_time.hour);
    alarm_config.alarm_minute   = WRITE_BCD(current_time.minute);
    alarm_config.alarm_second   = WRITE_BCD(current_time.second);
    alarm_config.am_pm          = RTC_AM;               // 24 Сʱ�ƿ���Ϊ RTC_AM (0)

    rtc_alarm_config(RTC_ALARM0, &alarm_config);
    rtc_alarm_enable(RTC_ALARM0);     
    rtc_interrupt_enable(RTC_INT_ALARM0);
    rtc_interrupt_enable(RTC_INT_ALARM0);   // ʹ������ 0 �ж�
    exti_interrupt_flag_clear(EXTI_17);                 // ��������־
    exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING); // �����ش���
    exti_interrupt_enable(EXTI_17);                     // ʹ�� EXTI �ж�
    nvic_irq_enable(RTC_Alarm_IRQn, 0, 0);   
    rtc_flag_clear(RTC_FLAG_ALRM0);
    exti_interrupt_flag_clear(EXTI_17);
    pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, PMU_LOWDRIVER_ENABLE,WFI_CMD);

    rcu_osci_on(RCU_HXTAL);
    while(!rcu_osci_stab_wait(RCU_HXTAL));

    rcu_pll_config(RCU_PLLSRC_HXTAL, 8, 336, 2, 8);  
    rcu_osci_on(RCU_PLL_CK);
    while(!rcu_osci_stab_wait(RCU_PLL_CK));

    rcu_system_clock_source_config(RCU_CKSYSSRC_PLLP);
    while(rcu_system_clock_source_get() != RCU_CKSYSSRC_PLLP);
		
}
uint32_t rtc_to_utc_seconds(const rtc_parameter_struct *rtc)
{   rtc_register_sync_wait();
		rtc_current_time_get(&rtc_time);
    uint32_t days = 0;
    int i;

    /* 1. �� BCD ת��Ϊʮ���� */
    uint16_t year_full = 2000 + READ_BCD(rtc->year);          /* ������� */
    uint8_t  mon       = READ_BCD(rtc->month);                /* 1~12 */
    uint8_t  day       = READ_BCD(rtc->date);                 /* 1~31 */
    uint8_t  hour      = READ_BCD(rtc->hour);                 /* 0~23 */
    uint8_t  minute    = READ_BCD(rtc->minute);               /* 0~59 */
    uint8_t  second    = READ_BCD(rtc->second);               /* 0~59 */

    /* 2. ����� 1970 �굽 (year_full-1) �������� */
    for (i = 1970; i < year_full; i++) {
        days += IS_LEAP(i) ? 366 : 365;
    }

    /* 3. ���ϱ���ǰ (mon-1) ���µ������� */
    int leap = IS_LEAP(year_full) ? 1 : 0;
    for (i = 0; i < mon - 1; i++) {
        days += days_in_mon[leap][i];
    }

    /* 4. ���ϵ����ѹ������������ڴ� 1 ��ʼ������Ҫ -1�� */
    days += (day - 1);

    /* 5. ��� = ���� * 86400 + �������� */
    return (uint32_t)(days * 86400UL + hour * 3600UL + minute * 60UL + second);
}
void utc_seconds_to_rtc(uint32_t sec, rtc_parameter_struct *rtc)
{
    uint32_t days = sec / 86400UL;          /* ������ */
    uint32_t sec_today = sec % 86400UL;     /* �������� */

    uint16_t y;          /* ��ǰ��ѡ��� */
    int m;               /* �·ݣ���ʱ0~11�� */

    /* 1. �ҵ���Ӧ����ݣ���1970���𲻶ϼ�ȥ���������� */
    y = 1970;
    while (1) {
        uint16_t days_this_year = IS_LEAP(y) ? 366 : 365;
        if (days < days_this_year) {
            break;
        }
        days -= days_this_year;
        y++;
    }

    /* 2. ȷ���·ݣ��� 1 �������¼��� */
    int leap = IS_LEAP(y) ? 1 : 0;
    for (m = 0; m < 12; m++) {
        if (days < days_in_mon[leap][m]) {
            break;
        }
        days -= days_in_mon[leap][m];
    }
    /* ��ʱ days Ϊ���µĵڼ��죨0?~?��������-1�� */

    /* 3. ��ʮ������ֵתΪ BCD ������ṹ�� */
    /* ע�� year ��Ҫת��Ϊ����λ���� 2024 �� 0x24 */
    uint16_t year_tail = y % 100;
    rtc->year   = WRITE_BCD(year_tail);
    rtc->month  = WRITE_BCD(m + 1);            /* m Ϊ 0?11��ת��Ϊ 1?12 */
    rtc->date   = WRITE_BCD(days + 1);          /* ���ڴ� 1 ��ʼ */
    rtc->hour   = WRITE_BCD(sec_today / 3600UL);
    rtc->minute = WRITE_BCD((sec_today % 3600UL) / 60UL);
    rtc->second = WRITE_BCD(sec_today % 60UL);

    /* �����ֶΰ��������� 0 ����ԭֵ */
		    rtc->display_format = RTC_24HOUR; 
        rtc->am_pm = RTC_AM;
        rtc->factor_asyn = 0x7F;   // = 127
        rtc->factor_syn = 0xFF;    // = 255
        // (32768 / (127+1)) / (255+1) = 1 Hz
	}
void send_auto_report_frame(void)
{
    uint8_t content[12];
     rtc_register_sync_wait();
		 rtc_current_time_get(&rtc_time);
    // --- 1. ��ȡ UTC �뼶ʱ�����4 �ֽڣ������---
    uint32_t utc_sec = rtc_to_utc_seconds(&rtc_time); // ����Ҫ�� RTC ��ȡ��ת��
    content[0] = (utc_sec >> 24) & 0xFF;
    content[1] = (utc_sec >> 16) & 0xFF;
    content[2] = (utc_sec >> 8)  & 0xFF;
    content[3] =  utc_sec        & 0xFF;

    // --- 2. CH0 ���ݣ���λ������ֵ �� ��ǰ��ȣ�---
    float ch0_ratio = param_get_ch0_ratio();
										float origin = ADC_Read_Register();
										float report_value = origin * ch0_ratio;
    uint32_t ch0_raw;
    memcpy(&ch0_raw, &report_value, 4); // С���� uint32
    // תΪ������ֽ�
    content[4] = (ch0_raw >> 24) & 0xFF;
    content[5] = (ch0_raw >> 16) & 0xFF;
    content[6] = (ch0_raw >> 8)  & 0xFF;
    content[7] =  ch0_raw        & 0xFF;

    // --- 3. CH1 ���ݣ�DAC �ض� �� ��ǰ��ȣ�---
    float ch1_ratio = param_get_ch1_ratio();
										  origin = ADC0_Read();
										  report_value = origin * ch1_ratio;
    uint32_t ch1_raw;
    memcpy(&ch1_raw, &report_value, 4);
    content[8]  = (ch1_raw >> 24) & 0xFF;
    content[9]  = (ch1_raw >> 16) & 0xFF;
    content[10] = (ch1_raw >> 8)  & 0xFF;
    content[11] =  ch1_raw        & 0xFF;

    // ����Ӧ��֡��֡���� 0x02�������� 0x0302������ 12 �ֽڣ�
    send_response(my_id, 0x02, 0x0302, content, 12);
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
    NVIC_SetPriority(SysTick_IRQn, 0x0FUL);
}
uint32_t get_current_tick(void)
{
    return sys_tick_ms;
}

void write_float_big_endian(float value, uint8_t *buf)
{
    uint32_t tmp;
    
    // ����memcpy��ȫ�ؽ�float��λģʽ���Ƶ�uint32_t��
    // ������ֱ��ָ������ת�������������ϸ����Υ�棩
    memcpy(&tmp, &value, sizeof(tmp));
    
    // ������򣨸��ֽ���ǰ�����4���ֽ�
    buf[0] = (uint8_t)(tmp >> 24);
    buf[1] = (uint8_t)(tmp >> 16);
    buf[2] = (uint8_t)(tmp >> 8);
    buf[3] = (uint8_t)(tmp);
}
float read_float_big_endian(const uint8_t *buf) {
    uint32_t tmp;
    float result;

    // �������4�ֽ�ƴ��uint32_t
    tmp = ((uint32_t)buf[0] << 24) |
          ((uint32_t)buf[1] << 16) |
          ((uint32_t)buf[2] << 8)  |
          ((uint32_t)buf[3]);

    // ����memcpy��uint32_t��λģʽ����Ϊfloat����������˫�أ�
    memcpy(&result, &tmp, sizeof(result));
    return result;
}
// �ڲ������Զ��ϱ������е���
void check_and_handle_alarm(uint8_t channel, float value) {
    float threshold;
    if (channel == 0) threshold = param_get_ch0_threshold();
    if (channel == 1) threshold = param_get_ch1_threshold();
    else return;

    // ֻ�г�����ֵ�Ŵ����澯
    if (value > threshold) {
        // ����澯�ַ��� (��ʽ: ʱ��|ͨ��|��ֵ|ʵ��ֵ)
        char record[64];
        rtc_register_sync_wait();
		    rtc_current_time_get(&rtc_time);
        snprintf(record, sizeof(record),
                 "%04d-%02d-%02d %02d:%02d:%02d | CH%d | %.2f | %.2f\r\n",READ_BCD(rtc_time.year)+2000,READ_BCD(rtc_time.month), 
						 READ_BCD(rtc_time.date),
						 READ_BCD(rtc_time.hour),
						 READ_BCD(rtc_time.minute),
						 READ_BCD(rtc_time.second),
                 channel, threshold, value);
        param_add_alarm(record); 

        // ����������ϱ�ģʽ�������ظ� ASCII �ַ���
        if (param_get_alarm_push_en() == 1) {
            usart_485_send_str((uint8_t*)record,2);
        }
    }
}

/****************************End*****************************/

