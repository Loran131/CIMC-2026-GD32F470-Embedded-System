


#ifndef __MODBUSCRC_H
#define __MODBUSCRC_H



#include "HeaderFiles.h"
#include <stdint.h>
#include "485.h"

#define CRC16_POLY 0xA001
// 状态机：寻找帧头、读取帧、校验、提取
#define FRAME_MAX_LEN   256   // 根据最大内容长度调整


// 解析结果结构体
typedef struct {
    unsigned short dev_id;    // 设备ID
    unsigned char  frame_type;//功能模块
    unsigned short cmd_word;  // 指令
    unsigned char  version;
    unsigned char  content_len;
    unsigned char  content[FRAME_MAX_LEN];
} ParsedFrame;

extern const uint16_t crc16_table[256];
extern const unsigned char CRCHi_table[256];
extern const unsigned char CRCLo_table[256];
unsigned short ModbusCRC(const unsigned char *data, unsigned short len);
int parse_ascii_frame_from_rb(RingBuffer *rb, ParsedFrame *frame);
void reset_ascii_parser(void);
void send_response(unsigned short dev_id, unsigned char frame_type,
                   unsigned short cmd_word,
                   unsigned char *content, unsigned char content_len);

#endif
