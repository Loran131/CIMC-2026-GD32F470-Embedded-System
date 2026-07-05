#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>

typedef struct {
    unsigned char *buffer;
    int size;
    int head;
    int tail;
    int count;
} RingBuffer;

/* 初始化：分配size字节的内存，成功返回0，失败返回-1 */
int RingBuffer_Init(RingBuffer *rb, int size);

/* 释放缓冲区内存 */
void RingBuffer_Destroy(RingBuffer *rb);

/*
 * 写入len字节数据，返回实际写入的字节数。
 * 如果可用空间不足，会尽可能写入（部分写入），返回值可能小于len。
 * 写入失败（如参数错误）返回 -1。
 */
int RingBuffer_Write(RingBuffer *rb, const unsigned char *data, int len);

/*
 * 读取len字节数据存入buf，返回实际读取的字节数。
 * 如果有效数据不足，只读取现有数据，返回值可能小于len。
 * 读取失败返回 -1。
 */
int RingBuffer_Read(RingBuffer *rb, unsigned char *buf, int len);

/* 判断缓冲区是否为空（count == 0） */
int RingBuffer_IsEmpty(RingBuffer *rb);

/* 判断缓冲区是否为满（count == size） */
int RingBuffer_IsFull(RingBuffer *rb);

/* 返回缓冲区中有效数据字节数 */
int RingBuffer_Used(RingBuffer *rb);

/* 返回缓冲区中剩余可用字节数 */
int RingBuffer_Available(RingBuffer *rb);

/* 重置缓冲区：不释放内存，只重置指针和计数 */
void RingBuffer_Reset(RingBuffer *rb);

/*
 * 窥视（Peek）：不移动读指针，复制最多len字节到buf。
 * 返回实际复制的字节数（不超过有效数据）。
 */
int RingBuffer_Peek(RingBuffer *rb, unsigned char *buf, int len);

#endif