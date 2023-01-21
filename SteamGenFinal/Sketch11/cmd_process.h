/*! 
*  \file hmi_driver.h
*  \brief ָ����ദ��
*  \version 1.0
*  \date 2012-2018
*  \copyright ���ݴ�ʹ��Ƽ����޹�˾
*/


#ifndef _CMD_PROCESS_H
#define _CMD_PROCESS_H

// #include "hmi_driver.h"

#define CMD_MAX_SIZE 64

#define NOTIFY_TOUCH_PRESS         0X01  //����������֪ͨ
#define NOTIFY_TOUCH_RELEASE       0X03  //�������ɿ�֪ͨ
#define NOTIFY_WRITE_FLASH_OK      0X0C  //дFLASH�ɹ�
#define NOTIFY_WRITE_FLASH_FAILD   0X0D  //дFLASHʧ��
#define NOTIFY_READ_FLASH_OK       0X0B  //��FLASH�ɹ�
#define NOTIFY_READ_FLASH_FAILD    0X0F  //��FLASHʧ��
#define NOTIFY_MENU                0X14  //�˵��¼�֪ͨ
#define NOTIFY_TIMER               0X43  //��ʱ����ʱ֪ͨ
#define NOTIFY_CONTROL             0XB1  //�ؼ�����֪ͨ
#define NOTIFY_READ_RTC            0XF7  //��ȡRTCʱ��
#define MSG_GET_CURRENT_SCREEN     0X01  //����ID�仯֪ͨ
#define MSG_GET_DATA               0X11  //�ؼ�����֪ͨ
#define NOTIFY_HandShake           0X55  //����֪ͨ

#define PTR2U16(PTR) ((((unsigned char *)(PTR))[0]<<8)|((unsigned char *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((unsigned char *)(PTR))[0]<<24)|(((unsigned char *)(PTR))[1]<<16)|(((unsigned char *)(PTR))[2]<<8)|((unsigned char *)(PTR))[3])  //�ӻ�����ȡ32λ����


enum CtrlType
{
    kCtrlUnknown=0x0,
    kCtrlButton=0x10,
    kCtrlText,
    kCtrlProgress,
    kCtrlSlider,
    kCtrlMeter,
    kCtrlDropList,
    kCtrlAnimation,
    kCtrlRTC,
    kCtrlGraph,
    kCtrlTable,
    kCtrlMenu,
    kCtrlSelector,
    kCtrlQRCode,
};

#pragma pack(push)
#pragma pack(1)

typedef struct
{
    unsigned char    cmd_head;

    unsigned char    cmd_type;
    unsigned char    ctrl_msg;
    unsigned int   screen_id;
    unsigned int   control_id;
    unsigned char    control_type;

    unsigned char    param[256];

    unsigned char  cmd_tail[4];
}CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)

/*! 
*  \brief  ����֪ͨ
*/
void NOTIFYHandShake();

/*! 
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, unsigned int size );
/*! 
*  \brief  �����л�֪ͨ
*  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
*  \param screen_id ��ǰ����ID
*/
void NotifyScreen(unsigned int screen_id);
/*! 
*  \brief  ���������¼���Ӧ
*  \param press 1���´�������3�ɿ�������
*  \param x x����
*  \param y y����
*/
void NotifyTouchXY(unsigned char press,unsigned int x,unsigned int y);

/*! 
*  \brief  ��ť�ؼ�֪ͨ
*  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param state ��ť״̬��0����1����
*/
void NotifyButton(unsigned int screen_id, unsigned int control_id, unsigned char state);
/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param str �ı��ؼ�����
*/
void NotifyText(unsigned int screen_id, unsigned int control_id, unsigned char *str);
/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ����GetControlValueʱ��ִ�д˺���                                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ                                                              
*/   
void NotifyProgress(unsigned int screen_id, unsigned int control_id, unsigned long value);
/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ                                                              
*/    
void NotifySlider(unsigned int screen_id, unsigned int control_id, unsigned long value);
/*! 
*  \brief  �Ǳ�ؼ�֪ͨ
*  \details  ����GetControlValueʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param value ֵ
*/
void NotifyMeter(unsigned int screen_id, unsigned int control_id, unsigned long value);
/*! 
*  \brief  �˵��ؼ�֪ͨ
*  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item �˵�������
*  \param state ��ť״̬��0�ɿ���1����
*/
void NotifyMenu(unsigned int screen_id, unsigned int control_id, unsigned char  item, unsigned char  state);

/*! 
*  \brief  ѡ��ؼ�֪ͨ
*  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item ��ǰѡ��
*/
void NotifySelector(unsigned int screen_id, unsigned int control_id, unsigned char  item);
/*! 
*  \brief  ��ʱ����ʱ֪ͨ����
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*/
void NotifyTimer(unsigned int screen_id, unsigned int control_id);
/*! 
*  \brief  ��ȡ�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*  \param _data ��������
*  \param length ���ݳ���
*/
void NotifyReadFlash(unsigned char status,unsigned char *_data,unsigned int length);

/*! 
*  \brief  д�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*/
void NotifyWriteFlash(unsigned char status);
/*! 
*  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
*  \param year �꣨BCD��
*  \param month �£�BCD��
*  \param week ���ڣ�BCD��
*  \param day �գ�BCD��
*  \param hour ʱ��BCD��
*  \param minute �֣�BCD��
*  \param second �루BCD��
*/
void NotifyReadRTC(unsigned char year,unsigned char month,unsigned char week,unsigned char day,unsigned char hour,unsigned char minute,unsigned char second);

#endif
