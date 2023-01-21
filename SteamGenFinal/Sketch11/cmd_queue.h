

#ifndef _CMD_QUEUE
#define _CMD_QUEUE

typedef unsigned char qdata;
typedef unsigned short qsize;

void queue_reset(void);
void queue_push(qdata _data);
qsize queue_find_cmd(qdata *cmd,qsize buf_len);

#endif
