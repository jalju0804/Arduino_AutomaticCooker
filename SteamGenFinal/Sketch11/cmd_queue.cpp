
#include <arduino.h>
#include "cmd_queue.h"

// #define USE_32BIT_CPU // STM32, STM8 등
#define USE_8BIT_CPU // 아두이노, ATmega32, ATmega128등

#define CMD_HEAD 0XEE         // 통신 프레임 헤더
#define CMD_TAIL 0XFFFCFFFF   // 프레임 끝
#define QUEUE_MAX_SIZE 256

// #define DEBUG_QUEUE_DATA

typedef struct _QUEUE                                             
{                                                                 
    qsize _head;
    qsize _tail;
    qdata _data[QUEUE_MAX_SIZE];
}QUEUE;                                                           

static QUEUE que = {0,0,0};

#ifdef USE_32BIT_CPU
static unsigned int cmd_state = 0;
#else
static unsigned long cmd_state = 0;
#endif

static qsize cmd_pos = 0;

void queue_reset(void)
{
    que._head = que._tail = 0;
    cmd_pos = cmd_state = 0;
}

void queue_push(qdata _data)
{
    qsize pos = (que._head+1)%QUEUE_MAX_SIZE;
    if(pos!=que._tail)
    {
        que._data[que._head] = _data;
        que._head = pos;

#ifdef DEBUG_QUEUE_DATA  
      Serial.print("_data=");
      Serial.println(_data);
      Serial.print("pos=");
      Serial.println(pos);      
#endif        
    }
}

static void queue_pop(qdata* _data)
{
    if(que._tail!=que._head)
    {
        *_data = que._data[que._tail];
        que._tail = (que._tail+1)%QUEUE_MAX_SIZE;
    }
}

static qsize queue_size()
{
    return ((que._head+QUEUE_MAX_SIZE-que._tail)%QUEUE_MAX_SIZE);
}

qsize queue_find_cmd(qdata *buffer,qsize buf_len)
{
    qsize cmd_size = 0;
    qdata _data = 0;

    while(queue_size()>0)
    {
        
        queue_pop(&_data);

        if(cmd_pos==0&&_data!=CMD_HEAD)
        {
            continue;
        }
        //    LED2_ON;
        if(cmd_pos<buf_len)
            buffer[cmd_pos++] = _data;

#ifdef USE_32BIT_CPU
        cmd_state = ((cmd_state<<8)|_data);        
        if(cmd_state==CMD_TAIL)
#else
        cmd_state = ((cmd_state<<8)|_data);        
        if(cmd_state==CMD_TAIL)
#endif        
        {
            //LED2_ON;
            cmd_size = cmd_pos;
            cmd_state = 0;
            cmd_pos = 0;

#if(CRC16_ENABLE)
            
            if(!CheckCRC16(buffer+1,cmd_size-5))
                return 0;

            cmd_size -= 2;
#endif
            return cmd_size;
        }
    }
    return 0;
}
