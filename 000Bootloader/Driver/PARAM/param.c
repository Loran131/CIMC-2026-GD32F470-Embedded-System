#include "param.h"
#include "gd32f4xx.h"      // 包含 fmc_xxx 函数声明
#include <string.h>
#include <stdbool.h>

/* 全局缓存 */
ParameterBlock g_param;

/* ==================== 默认参数 ==================== */
static const ParameterBlock g_default_param = {
    .magic            = PARAM_MAGIC,
    .device_id        = 0x0007,
    .baudrate         = 19200,
    .ch0_ratio        = 1.0f,
    .ch1_ratio        = 1.0f,
    .ch0_threshold    = 10.0f,
    .ch1_threshold    = 10.0f,
    .alarm_push_en    = 1,
    .alarm_write_index = 0,
    .param_version    = 1,
    .crc32            = 0,   // 由初始化时计算
};

/* ==================== CRC 计算（软件 CRC32） ==================== */
static uint32_t calc_crc32(const uint8_t *data, uint32_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

uint32_t param_calculate_crc(const ParameterBlock *block) {
    return calc_crc32((const uint8_t*)block, offsetof(ParameterBlock, crc32));
}

/* ==================== 校验参数有效性 ==================== */
static bool param_is_valid(const ParameterBlock *block) {
    if (block->magic != PARAM_MAGIC) return false;
    uint32_t expected = param_calculate_crc(block);
    return (block->crc32 == expected);
}

/* ==================== 从 Flash 读取参数到 RAM ==================== */
static void param_read_from_flash(void) {
    const ParameterBlock *flash_ptr = (const ParameterBlock *)PARAM_FLASH_ADDR;
    memcpy(&g_param, flash_ptr, sizeof(ParameterBlock));
}

/* ==================== 将 RAM 参数写入 Flash（写后自动上锁） ==================== */
void param_save(void) {
    /* 1. 计算并填入 CRC */
    g_param.crc32 = param_calculate_crc(&g_param);

    /* 2. 解锁 Flash */
    fmc_unlock();

    /* 3. 禁止中断（Flash 编程期间不可响应中断） */
    __disable_irq();

    /* 4. 擦除参数区所有页 */
    for (uint32_t i = 0; i < PARAM_FLASH_PAGE_COUNT; i++) {
        uint32_t page_addr = PARAM_FLASH_ADDR + i * FLASH_PAGE_SIZE;
        fmc_page_erase(page_addr);
        /* 可在此检查擦除状态，简化代码略 */
    }

    /* 5. 按字写入整个结构体 */
    const uint32_t *src = (const uint32_t *)&g_param;
    uint32_t word_cnt = sizeof(ParameterBlock) / 4;
    for (uint32_t i = 0; i < word_cnt; i++) {
        if (fmc_word_program(PARAM_FLASH_ADDR + i * 4, src[i]) != FMC_READY) {
            /* 写入失败处理（打印错误或重试），原型中忽略 */
        }
    }

    /* 6. 重新使能中断 */
    __enable_irq();

    /* 7. 锁定 Flash） */
    fmc_lock();
}

/* ==================== 初始化 ==================== */
void param_init(void) {
    param_read_from_flash();
    if (!param_is_valid(&g_param)) {
        /* 无效：加载默认并写入 Flash */
        memcpy(&g_param, &g_default_param, sizeof(ParameterBlock));
        param_save();   // 写入后自动上锁
    }
}

/* ==================== Getter/Setter ==================== */
uint16_t param_get_device_id(void) {
    return g_param.device_id;
}

void param_set_device_id(uint16_t id) {
    if (g_param.device_id != id) {
        g_param.device_id = id;
        param_save();   // 修改后立即持久化并上锁
    }
}

uint32_t param_get_baudrate(void) {
    return g_param.baudrate;
}

void param_set_baudrate(uint32_t baud) {
    if (g_param.baudrate != baud) {
        g_param.baudrate = baud;
        param_save();
    }
}

float param_get_ch0_ratio(void) {
    return g_param.ch0_ratio;
}

void param_set_ch0_ratio(float ratio) {
    g_param.ch0_ratio = ratio;
    param_save();
}

float    param_get_ch1_ratio(void){
    return g_param.ch1_ratio;
}
void     param_set_ch1_ratio(float ratio){
		g_param.ch1_ratio = ratio;
    param_save();
	
}

float    param_get_ch0_threshold(void){
    return g_param.ch0_threshold;
}
void     param_set_ch0_threshold(float th){
    g_param.ch0_threshold = th;
    param_save();
}

float    param_get_ch1_threshold(void){
    return g_param.ch1_threshold;
}

void     param_set_ch1_threshold(float th){
   g_param.ch1_threshold = th;
    param_save();
}
float param_get_ch2_threshold(void) {
    return g_param.ch2_threshold;
}

void param_set_ch2_threshold(float th) {
    if (g_param.ch2_threshold != th) {
        g_param.ch2_threshold = th;
        param_save(); 
    }
}
uint8_t  param_get_alarm_push_en(void){
    return g_param.alarm_push_en;
}
void     param_set_alarm_push_en(uint8_t en){
    g_param.alarm_push_en = en;
    param_save();
}

void param_add_alarm(const char *record) {
    uint8_t idx = g_param.alarm_write_index;
    strncpy(g_param.alarm_records[idx], record, sizeof(g_param.alarm_records[idx]));
    g_param.alarm_records[idx][sizeof(g_param.alarm_records[idx]) - 1] = '\0';
    g_param.alarm_write_index = (idx + 1) % ALARM_RECORD_COUNT;
    param_save();
}

void param_clear_alarm(void) {
    memset(g_param.alarm_records, 0, sizeof(g_param.alarm_records));
    g_param.alarm_write_index = 0;
    param_save();
}
uint8_t param_get_upgrade_flag(){
	return g_param.upgrade_requested;
}