#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>

int RingBuffer_Init(RingBuffer *rb, int size)
{
    if (rb == NULL || size <= 0)
        return -1;

    rb->buffer = (unsigned char *)malloc(size);
    if (rb->buffer == NULL)
        return -1;

    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return 0;
}

void RingBuffer_Destroy(RingBuffer *rb)
{
    if (rb != NULL) {
        free(rb->buffer);
        rb->buffer = NULL;
        rb->size = 0;
        rb->head = rb->tail = rb->count = 0;
    }
}

int RingBuffer_Write(RingBuffer *rb, const unsigned char *data, int len)
{
    if (rb == NULL || rb->buffer == NULL || data == NULL || len <= 0)
        return -1;

    /* 计算实际可写入长度（不能超过可用空间） */
    int available = rb->size - rb->count;
    int write_len = (len < available) ? len : available;
    if (write_len == 0)
        return 0;

    /* 分两段拷贝：从 tail 到缓冲区末尾，再绕回到头部 */
    int first_part = rb->size - rb->tail;
    if (first_part >= write_len) {
        /* 一次拷贝即可 */
        memcpy(rb->buffer + rb->tail, data, write_len);
        rb->tail += write_len;
    } else {
        /* 先拷贝第一段（到末尾） */
        memcpy(rb->buffer + rb->tail, data, first_part);
        /* 再拷贝第二段（从开头） */
        memcpy(rb->buffer, data + first_part, write_len - first_part);
        rb->tail = write_len - first_part;    /* 回绕后的新 tail */
    }

    rb->count += write_len;
    return write_len;
}

int RingBuffer_Read(RingBuffer *rb, unsigned char *buf, int len)
{
    if (rb == NULL || rb->buffer == NULL || buf == NULL || len <= 0)
        return -1;

    if (rb->count == 0)
        return 0;

    int read_len = (len < rb->count) ? len : rb->count;   /* 不能超过有效数据 */

    /* 分两段读取：从 head 到缓冲区末尾，再绕回到头部 */
    int first_part = rb->size - rb->head;
    if (first_part >= read_len) {
        memcpy(buf, rb->buffer + rb->head, read_len);
        rb->head += read_len;
    } else {
        memcpy(buf, rb->buffer + rb->head, first_part);
        memcpy(buf + first_part, rb->buffer, read_len - first_part);
        rb->head = read_len - first_part;       /* 回绕后的新 head */
    }

    rb->count -= read_len;
    return read_len;
}

int RingBuffer_IsEmpty(RingBuffer *rb)
{
    return (rb == NULL || rb->count == 0);
}

int RingBuffer_IsFull(RingBuffer *rb)
{
    return (rb != NULL && rb->count == rb->size);
}

int RingBuffer_Used(RingBuffer *rb)
{
    return (rb == NULL) ? -1 : rb->count;
}

int RingBuffer_Available(RingBuffer *rb)
{
    return (rb == NULL) ? -1 : (rb->size - rb->count);
}

void RingBuffer_Reset(RingBuffer *rb)
{
    if (rb != NULL) {
        rb->head = 0;
        rb->tail = 0;
        rb->count = 0;
    }
}

int RingBuffer_Peek(RingBuffer *rb, unsigned char *buf, int len)
{
    if (rb == NULL || rb->buffer == NULL || buf == NULL || len <= 0)
        return -1;

    if (rb->count == 0)
        return 0;

    int peek_len = (len < rb->count) ? len : rb->count;

    /* 只复制数据，不移动 head */
    int first_part = rb->size - rb->head;
    if (first_part >= peek_len) {
        memcpy(buf, rb->buffer + rb->head, peek_len);
    } else {
        memcpy(buf, rb->buffer + rb->head, first_part);
        memcpy(buf + first_part, rb->buffer, peek_len - first_part);
    }
    return peek_len;
}