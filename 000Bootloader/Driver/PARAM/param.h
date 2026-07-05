#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>      // offsetof

/* 参数区物理基地址（与 Flash 分区表一致） */
#define PARAM_FLASH_ADDR         ((uint32_t)0x08010000UL)

/* Flash 页大小！！！！！！！！！！这个是多少来着 */
#define FLASH_PAGE_SIZE          0x800UL   // 2KB
#define PARAM_FLASH_SIZE         0x1000UL  // 4KB
#define PARAM_FLASH_PAGE_COUNT   (PARAM_FLASH_SIZE / FLASH_PAGE_SIZE)

/* 告警记录最大条目数 */
#define ALARM_RECORD_COUNT       10
#define TIMESTAMP_STR_LEN        24

/* 魔数，用于标识参数区是否已初始化 */
#define PARAM_MAGIC              0xA5A5

/**
 * 参数结构体（所有字段对齐到 4 字节，便于 Flash 按字写入）
 */
typedef struct {
    uint16_t magic;                     // 魔数 (0xA5A5)
    uint16_t device_id;                 // 设备 ID
    uint32_t baudrate;                  // 波特率（如 19200）
    float    ch0_ratio;                 // CH0 变比
    float    ch1_ratio;                 // CH1 变比
    float    ch0_threshold;             // CH0 阈值
    float    ch1_threshold;             // CH1 阈值
	  float    ch2_threshold; 								// CH2 阈值
    uint8_t  alarm_push_en;             // 主动上报告警使能 (1-主动, 2-被动)
    uint8_t  alarm_write_index;         // 告警记录循环写入索引
    uint8_t  padding[2];                // 补齐到 4 字节
	  uint8_t  upgrade_requested;          // 0:无升级请求, 1:有升级请求
    char     alarm_records[ALARM_RECORD_COUNT][TIMESTAMP_STR_LEN + 32]; // 最近 10 条告警
    uint8_t  param_version;             // 参数版本号
    uint8_t  reserved[3];               // 预留
    uint32_t crc32;                     // 整个结构体（不含本字段）的 CRC32
} ParameterBlock;

/* 全局缓存，对外可见 */
extern ParameterBlock g_param;

/* 初始化：从 Flash 读取并校验，若无效则设默认值并写入 */
void param_init(void);

/* 保存 RAM 缓存到 Flash（含解锁→擦除→编程→上锁） */
void param_save(void);

/* 计算（除 crc32 外）的 CRC32 值 */
uint32_t param_calculate_crc(const ParameterBlock *block);

/* ================== Getter/Setter 接口 ================== */
uint16_t param_get_device_id(void);
void     param_set_device_id(uint16_t id);

uint32_t param_get_baudrate(void);
void     param_set_baudrate(uint32_t baud);

float    param_get_ch0_ratio(void);
void     param_set_ch0_ratio(float ratio);

float    param_get_ch1_ratio(void);
void     param_set_ch1_ratio(float ratio);

float    param_get_ch0_threshold(void);
void     param_set_ch0_threshold(float th);

float    param_get_ch1_threshold(void);
void     param_set_ch1_threshold(float th);

void param_set_ch2_threshold(float th);
float param_get_ch2_threshold(void);

uint8_t  param_get_alarm_push_en(void);
void     param_set_alarm_push_en(uint8_t en);

void     param_add_alarm(const char *record);
void     param_clear_alarm(void);

uint8_t param_get_upgrade_flag();
#endif /* __PARAM_H__ */
