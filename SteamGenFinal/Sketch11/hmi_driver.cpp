
#include <arduino.h>
#include "hmi_driver.h"

extern void  SendChar(uchar t);

#define TX_8(P1) SEND_DATA((P1)&0xFF)
#define TX_8N(P,N) SendNU8((uint8 *)P,N)
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)
#define TX_16N(P,N) SendNU16((uint16 *)P,N)
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)



#if(CRC16_ENABLE)

static uint16 _crc16 = 0xffff;
/*!
* \brief Check whether the data meets CRC16 check
* \param buffer the data to be verified
* \param n data length, including CRC16
* \param pcrc check code
*/
static void AddCRC16(uint8 *buffer,uint16 n,uint16 *pcrc)
{
    uint16 i,j,carry_flag,a;

    for (i=0; i<n; i++)
    {
        *pcrc=*pcrc^buffer[i];
        for (j=0; j<8; j++)
        {
            a=*pcrc;
            carry_flag=a&0x0001;
            *pcrc=*pcrc>>1;
            if (carry_flag==1)
                *pcrc=*pcrc^0xa001;
        }
    }
} 
/*!
* \brief Check whether the data meets CRC16 check
* \param buffer Data to be verified, CRC16 is stored at the end
* \param n data length, including CRC16
* \return Return 1 if the check is passed, otherwise 0
*/
uint16 CheckCRC16(uint8 *buffer,uint16 n)
{
    uint16 crc0 = 0x0;
    uint16 crc1 = 0xffff;

    if(n>=2)
    {
        crc0 = ((buffer[n-2]<<8)|buffer[n-1]);
        AddCRC16(buffer,n-2,&crc1);
    }

    return (crc0==crc1);
}



/*!
* \brief sends a byte
* \param c
*/
void SEND_DATA(uint8 c)
{
    AddCRC16(&c,1,&_crc16);
    SendChar(c);
}
/*!
* \brief frame header
*/
void BEGIN_CMD()
{
    TX_8(0XEE);
    _crc16 = 0XFFFF; //Start calculating CRC16
}
/*!
* \brief end of frame
*/
void END_CMD()
{
    uint16 crc16 = _crc16;
    TX_16(crc16); //Send CRC16
    TX_32(0XFFFCFFFF);
}

#else//NO CRC16

#define SEND_DATA(P) SendChar(P) //Send a byte
#define BEGIN_CMD() TX_8(0XEE) //Frame header
#define END_CMD() TX_32(0XFFFCFFFF) //End of frame

#endif
/*!
* \brief delay
* \param n Delay time (in milliseconds)
*/
void DelayMS(unsigned int n) 
{
    int i,j;  
    for(i = n;i>0;i--)
        for(j=1000;j>0;j--) ; 
}
/*!
* \brief Serial port to send string
* \param string
*/
void SendStrings(uchar *str)
{
    while(*str)
    {
        TX_8(*str);
        str++;
    }
}



/*!
* \brief Serial port sends N bytes
* \param number
*/
void SendNU8(uint8 *pData,uint16 nDataLen)
{
    uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_8(pData[i]);
    }
}
/*!
* \brief Serial port sends N 16-bit data
* \param number
*/
void SendNU16(uint16 *pData,uint16 nDataLen)
{
    uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_16(pData[i]);
    }
}
/*!
* \brief send handshake command
*/
void SetHandShake()
{
    BEGIN_CMD();
    TX_8(0x04);
    END_CMD();
}

/*!
* \brief set the foreground color
* \param color foreground color
*/
void SetFcolor(uint16 color)
{
    BEGIN_CMD();
    TX_8(0x41);
    TX_16(color);
    END_CMD();
}
/*!
* \brief set background color
* \param color background color
*/
void SetBcolor(uint16 color)
{
    BEGIN_CMD();
    TX_8(0x42);
    TX_16(color);
    END_CMD();
}
/*!
* \brief get
* \param color background color
*/
void ColorPicker(uint8 mode, uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0xA3);
    TX_8(mode);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
* \brief clear screen
*/
void GUI_CleanScreen()
{
    BEGIN_CMD();
    TX_8(0x01);
    END_CMD();
}
/*!
* \brief set text interval
* \param x_w horizontal interval
* \param y_w vertical interval
*/
void SetTextSpace(uint8 x_w, uint8 y_w)
{
    BEGIN_CMD();
    TX_8(0x43);
    TX_8(x_w);
    TX_8(y_w);
    END_CMD();
}
/*!
* \brief set text display limit
* \param enable Whether to enable restriction
* \param width width
* \param height height
*/
void SetFont_Region(uint8 enable,uint16 width,uint16 height)
{
    BEGIN_CMD();
    TX_8(0x45);
    TX_8(enable);
    TX_16(width);
    TX_16(height);
    END_CMD();
}
/*!
* \brief set filter color
* \param fillcolor_dwon color lower bound
* \param fillcolor_up color upper bound
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up)
{
    BEGIN_CMD();
    TX_8(0x44);
    TX_16(fillcolor_dwon);
    TX_16(fillcolor_up);
    END_CMD();
}

/*!
* \brief set filter color
* \param x position X coordinate
* \param y position Y coordinate
* \param back color upper bound
* \param font font
* \param strings string content
*/
void DisText(uint16 x, uint16 y,uint8 back,uint8 font,uchar *strings )
{
    BEGIN_CMD();
    TX_8(0x20);
    TX_16(x);
    TX_16(y);
    TX_8(back);
    TX_8(font);
    SendStrings(strings);
    END_CMD();
}
/*!
* \brief show cursor
* \param enable whether to display
* \param x position X coordinate
* \param y position Y coordinate
* \param width width
* \param height height
*/
void DisCursor(uint8 enable,uint16 x, uint16 y,uint8 width,uint8 height )
{
    BEGIN_CMD();
    TX_8(0x21);
    TX_8(enable);
    TX_16(x);
    TX_16(y);
    TX_8(width);
    TX_8(height);
    END_CMD();
}
/*!
* \brief display full screen picture
* \param image_id image index
* \param masken whether to enable transparent mask
*/
void DisFull_Image(uint16 image_id,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x31);
    TX_16(image_id);
    TX_8(masken);
    END_CMD();
}
/*!
* \brief Display the picture at the specified location
* \param x position X coordinate
* \param y position Y coordinate
* \param image_id image index
* \param masken whether to enable transparent mask
*/
void DisArea_Image(uint16 x,uint16 y,uint16 image_id,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x32);
    TX_16(x);
    TX_16(y);
    TX_16(image_id);
    TX_8(masken);
    END_CMD();
}
/*!
* \brief display cropped picture
* \param x position X coordinate
* \param y position Y coordinate
* \param image_id image index
* \param image_x Image cropping position X coordinate
* \param image_y Y coordinate of image cropping position
* \param image_l Image cropping length
* \param image_w Image cropping height
* \param masken whether to enable transparent mask
*/
void DisCut_Image(uint16 x,uint16 y,uint16 image_id,uint16 image_x,uint16 image_y,uint16 image_l, uint16 image_w,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x33);
    TX_16(x);
    TX_16(y);
    TX_16(image_id);
    TX_16(image_x);
    TX_16(image_y);
    TX_16(image_l);
    TX_16(image_w);
    TX_8(masken);
    END_CMD();
}

/*!
* \brief display GIF animation
* \param x position X coordinate
* \param y position Y coordinate
* \param flashimage_id image index
* \param enable whether to display
* \param playnum play count
*/
void DisFlashImage(uint16 x,uint16 y,uint16 flashimage_id,uint8 enable,uint8 playnum)
{
    BEGIN_CMD();
    TX_8(0x80);
    TX_16(x);
    TX_16(y);
    TX_16(flashimage_id);
    TX_8(enable);
    TX_8(playnum);
    END_CMD();
}
/*!
* \brief draw points
* \param x position X coordinate
* \param y position Y coordinate
*/
void GUI_Dot(uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0x50);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
* \brief draw a line
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{
    BEGIN_CMD();
    TX_8(0x51);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}

/*!
* \brief draw a polyline
* \param mode mode
* \param dot data point
* \param dot_cnt points
*/
void GUI_ConDots(uint8 mode,uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x63);
    TX_8(mode);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}

/*!
* \brief x coordinate is equidistant using foreground color connection
* \param x abscissa
* \param x_space distance
* \param dot_y a set of vertical axis coordinates
* \param dot_cnt number of ordinates
*/
void GUI_ConSpaceDots(uint16 x,uint16 x_space,uint16 *dot_y,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x59);
    TX_16(x);
    TX_16(x_space);
    TX_16N(dot_y,dot_cnt);
    END_CMD();
}
/*!
* \brief Use the foreground color to connect according to the coordinate offset
* \param x abscissa
* \param y vertical distance
* \param dot_offset offset
* \param dot_cnt offset number
*/
void GUI_FcolorConOffsetDots(uint16 x,uint16 y,uint16 *dot_offset,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x75);
    TX_16(x);
    TX_16(y);
    TX_16N(dot_offset,dot_cnt);
    END_CMD();
}
/*!
* \brief uses the background color to connect according to the coordinate offset
* \param x abscissa
* \param y vertical distance
* \param dot_offset offset
* \param dot_cnt offset number
*/
void GUI_BcolorConOffsetDots(uint16 x,uint16 y,uint8 *dot_offset,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x76);
    TX_16(x);
    TX_16(y);
    TX_16N(dot_offset,dot_cnt);
    END_CMD();
}
/*!
* \brief automatically adjust the backlight brightness
* \param enable
* \param bl_off_level standby brightness
* \param bl_on_level activate brightness
* \param bl_on_time offset number
*/
void SetPowerSaving(uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8  bl_on_time)
{
    BEGIN_CMD();
    TX_8(0x77);
    TX_8(enable);
    TX_8(bl_off_level);
    TX_8(bl_on_level);
    TX_8(bl_on_time);
    END_CMD();
}
/*!
* \brief connects multiple coordinate points with foreground color
* \param dot coordinate point
* \param dot_cnt offset number
*/
void GUI_FcolorConDots(uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x68);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}
/*!
* \brief connects multiple coordinate points with background color
* \param dot coordinate point
* \param dot_cnt offset number
*/
void GUI_BcolorConDots(uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x69);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}
/*!
* \brief draw a hollow circle
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
*/
void GUI_Circle(uint16 x, uint16 y, uint16 r)
{
    BEGIN_CMD();
    TX_8(0x52);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    END_CMD();
}
/*!
* \brief draw a solid circle
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
*/
void GUI_CircleFill(uint16 x, uint16 y, uint16 r)
{
    BEGIN_CMD();
    TX_8(0x53);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    END_CMD();
}
/*!
* \brief draw arc
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
* \param sa starting angle
* \param ea end angle
*/
void GUI_Arc(uint16 x,uint16 y, uint16 r,uint16 sa, uint16 ea)
{
    BEGIN_CMD();
    TX_8(0x67);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    TX_16(sa);
    TX_16(ea);
    END_CMD();
}
/*!
* \brief draw a hollow rectangle
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x54);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
* \brief draw a solid rectangle
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x55);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
* \brief draw a hollow ellipse
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Ellipse(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x56);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
* \brief draw a solid ellipse
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_EllipseFill(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x57);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
* \brief draw a line
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void SetBackLight(uint8 light_level)
{
    BEGIN_CMD();
    TX_8(0x60);
    TX_8(light_level);
    END_CMD();
}

void SetBuzzer(uint8 time)
{
    BEGIN_CMD();
    TX_8(0x61);
    TX_8(time);
    END_CMD();
}

void GUI_AreaInycolor(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x65);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}

void SetTouchPaneOption(uint8 enbale,uint8 beep_on,uint8 work_mode,uint8 press_calibration)
{
    uint8 options = 0;

    if(enbale)
        options |= 0x01;
    if(beep_on)
        options |= 0x02;
    if(work_mode)
        options |= (work_mode<<2);
    if(press_calibration)
        options |= (press_calibration<<5);

    BEGIN_CMD();
    TX_8(0x70);
    TX_8(options);
    END_CMD();
}

void CalibrateTouchPane()
{
    BEGIN_CMD();
    TX_8(0x72);
    END_CMD();
}

void TestTouchPane()
{
    BEGIN_CMD();
    TX_8(0x73);
    END_CMD();
}


void LockDeviceConfig(void)
{
    BEGIN_CMD();
    TX_8(0x09);
    TX_8(0xDE);
    TX_8(0xED);
    TX_8(0x13);
    TX_8(0x31);
    END_CMD();
}


void UnlockDeviceConfig(void)
{
    BEGIN_CMD();
    TX_8(0x08);
    TX_8(0xA5);
    TX_8(0x5A);
    TX_8(0x5F);
    TX_8(0xF5);
    END_CMD();
}
void SetCommBps(uint8 option)
{
    BEGIN_CMD();
    TX_8(0xA0);
    TX_8(option);
    END_CMD();
}

/*!
* \brief set the current write layer
* \details is generally used to achieve double buffering effect (to avoid flickering when drawing):
* \details uint8 layer = 0;
* \details WriteLayer(layer); Set the write layer
* \details ClearLayer(layer); make the layer transparent
* \details add a series of drawing commands
* \details DisText(100,100,0,4,"hello hmi!!!");
* \details DisplyLayer(layer); switch display layer
* \details layer = (layer+1)%2; double buffer switching
* \see DisplyLayer
* \see ClearLayer
* \param layer layer number
*/
void WriteLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0xA1);
    TX_8(layer);
    END_CMD();
}
/*!
* \brief Set the current display layer
* \param layer layer number
*/
void DisplyLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0xA2);
    TX_8(layer);
    END_CMD();
}
/*!
* \brief copy layer
* \param src_layer original layer
* \param dest_layer target layer
*/
void CopyLayer(uint8 src_layer,uint8 dest_layer)
{
    BEGIN_CMD();
    TX_8(0xA4);
    TX_8(src_layer);
    TX_8(dest_layer);
    END_CMD();
}
/*!
* \brief clear the layer and make the layer transparent
* \param layer layer number
*/
void ClearLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0x05);
    TX_8(layer);
    END_CMD();
}

void GUI_DispRTC(uint8 enable,uint8 mode,uint8 font,uint16 color,uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0x85);
    TX_8(enable);
    TX_8(mode);
    TX_8(font);
    TX_16(color);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
* \brief Write data to the user storage area of ??the serial screen
* \param startAddress starting address
* \param length bytes
* \param _data The data to be written
*/
void WriteUserFlash(uint32 startAddress,uint16 length,uint8 *_data)
{
    BEGIN_CMD();
    TX_8(0x87);
    TX_32(startAddress);
    TX_8N(_data,length);
    END_CMD();
}
/*!
* \brief read data from the user storage area of ??the serial screen
* \param startAddress starting address
* \param length bytes
*/
void ReadUserFlash(uint32 startAddress,uint16 length)
{
    BEGIN_CMD();
    TX_8(0x88);
    TX_32(startAddress);
    TX_16(length);
    END_CMD();
}
/*!
* \brief Get the current picture
*/
void GetScreen(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x01);
    END_CMD();
}
/*!
* \brief set the current screen
* \param screen_id screen ID
*/
void SetScreen(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x00);
    TX_16(screen_id);
    END_CMD();
}
/*!
* \brief disable\enable screen update
* \details disable\enable general use in pairs to avoid flicker and improve refresh speed
* \details usage:
* \details SetScreenUpdateEnable(0);//Update is prohibited
* \details A series of commands to update the screen
* \details SetScreenUpdateEnable(1);//Update immediately
* \param enable 0 is disabled, 1 is enabled
*/
void SetScreenUpdateEnable(uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB3);
    TX_8(enable);
    END_CMD();
}
/*!
* \brief set control input focus
* \param screen_id screen ID
* \param control_id control ID
* Whether \param focus has input focus
*/
void SetControlFocus(uint16 screen_id,uint16 control_id,uint8 focus)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x02);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(focus);
    END_CMD();
}
/*!
* \brief show\hide controls
* \param screen_id screen ID
* \param control_id control ID
* \param visible whether to display
*/
void SetControlVisiable(uint16 screen_id,uint16 control_id,uint8 visible)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x03);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(visible);
    END_CMD();
}
/*!
* \brief set touch control enable
* \param screen_id screen ID
* \param control_id control ID
* \param enable Whether the control is enabled
*/
void SetControlEnable(uint16 screen_id,uint16 control_id,uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x04);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(enable);
    END_CMD();
}
/*!
* \brief set button state
* \param screen_id screen ID
* \param control_id control ID
* \param value button state
*/
void SetButtonValue(uint16 screen_id,uint16 control_id,uchar state)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(state);
    END_CMD();
}
/*!
* \brief set text value
* \param screen_id screen ID
* \param control_id control ID
* \param str text value
*/
void SetTextValue(uint16 screen_id,uint16 control_id,uchar *str)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    SendStrings(str);
    END_CMD();
}

#if FIRMWARE_VER>=908
/*!
* \brief sets the text to an integer value, requiring FIRMWARE_VER>=908
* \param screen_id screen ID
* \param control_id control ID
* \param value text value
* \param sign 0-unsigned, 1-signed
* \param fill_zero Number of digits, if not enough
*/
void SetTextInt32(uint16 screen_id,uint16 control_id,uint32 value,uint8 sign,uint8 fill_zero)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x07);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(sign?0X01:0X00);
    TX_8((fill_zero&0x0f)|0x80);
    TX_32(value);
    END_CMD();
}
/*!
* \brief sets the text single-precision floating point value, requires FIRMWARE_VER>=908
* \param screen_id screen ID
* \param control_id control ID
* \param value text value
* \param precision decimal places
* When \param show_zeros is 1, the end 0 is displayed
*/
void SetTextFloat(uint16 screen_id,uint16 control_id,float value,uint8 precision,uint8 show_zeros)
{
	uint8 i = 0;

	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x07);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(0x02);
	TX_8((precision&0x0f)|(show_zeros?0x80:0x00));
	
	for (i=0;i<4;++i)
	{
	 //Need to distinguish between big and small
#if(0)
		TX_8(((uint8 *)&value)[i]);
#else
		TX_8(((uint8 *)&value)[3-i]);
#endif
	}
	END_CMD();
}
#endif
/*!
* \brief set progress value
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/
void SetProgressValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
* \brief set meter value
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/
void SetMeterValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
* \brief set meter value
* \param screen_id screen ID
* \param control_id Image control ID
* \param value
*/
void Set_picMeterValue(uint16 screen_id,uint16 control_id,uint16 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);
    END_CMD();
}
/*!
* \brief set the slider
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/

void SetSliderValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
* \brief set selection control
* \param screen_id screen ID
* \param control_id control ID
* \param item current option
*/
void SetSelectorValue(uint16 screen_id,uint16 control_id,uint8 item)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(item);
    END_CMD();
}

/*!
* \brief Get control value
* \param screen_id screen ID
* \param control_id control ID
*/
void GetControlValue(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x11);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}

/*!
* \brief start to play animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationStart(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x20);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}

/*!
* \brief stop the animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationStop(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x21);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief Pause the animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPause(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x22);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief play the specified frame
* \param screen_id screen ID
* \param control_id control ID
* \param frame_id frame ID
*/
void AnimationPlayFrame(uint16 screen_id,uint16 control_id,uint8 frame_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x23);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(frame_id);
    END_CMD();
}
/*!
* \brief play the previous frame
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPlayPrev(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x24);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief play the next frame
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPlayNext(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x25);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief curve control-add channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param color color
*/
void GraphChannelAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x30);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_16(color);
    END_CMD();
}
/*!
* \brief curve control-delete channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
*/
void GraphChannelDel(uint16 screen_id,uint16 control_id,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x31);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    END_CMD();
}
/*!
* \brief curve control-add data
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param pData Curve data
* \param nDataLen Number of data
*/
void GraphChannelDataAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint8 *pData,uint16 nDataLen)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x32);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_16(nDataLen);
    TX_8N(pData,nDataLen);
    END_CMD();
}
/*!
* \brief curve control-clear data
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
*/
void GraphChannelDataClear(uint16 screen_id,uint16 control_id,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x33);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    END_CMD();
}
/*!
* \brief curve control-set view window
* \param screen_id screen ID
* \param control_id control ID
* \param x_offset horizontal offset
* \param x_mul horizontal zoom factor
* \param y_offset vertical offset
* \param y_mul vertical zoom factor
*/
void GraphSetViewport(uint16 screen_id,uint16 control_id,int16 x_offset,uint16 x_mul,int16 y_offset,uint16 y_mul)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x34);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(x_offset);
    TX_16(x_mul);
    TX_16(y_offset);
    TX_16(y_mul);
    END_CMD();
}
/*!
* \brief start batch update
* \param screen_id screen ID
*/
void BatchBegin(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x12);
    TX_16(screen_id);
}
/*!
* \brief Batch update button controls
* \param control_id control ID
* \param value
*/
void BatchSetButtonValue(uint16 control_id,uint8 state)
{
    TX_16(control_id);
    TX_16(1);
    TX_8(state);
}
/*!
* \brief batch update progress bar controls
* \param control_id control ID
* \param value
*/
void BatchSetProgressValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}

/*!
* \brief batch update slider controls
* \param control_id control ID
* \param value
*/
void BatchSetSliderValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}
/*!
* \brief batch update instrument controls
* \param control_id control ID
* \param value
*/
void BatchSetMeterValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}
/*!
* \brief calculate string length
*/
uint32 GetStringLen(uchar *str)
{
    uchar *p = str;
    while(*str)
    {
        str++;
    }

    return (str-p);
}
/*!
* \brief batch update text controls
* \param control_id control ID
* \param strings
*/
void BatchSetText(uint16 control_id,uchar *strings)
{
    TX_16(control_id);
    TX_16(GetStringLen(strings));
    SendStrings(strings);
}
/*!
* \brief batch update animation\icon controls
* \param control_id control ID
* \param frame_id frame ID
*/
void BatchSetFrame(uint16 control_id,uint16 frame_id)
{
    TX_16(control_id);
    TX_16(2);
    TX_16(frame_id);
}

#if FIRMWARE_VER>=908

/*!
* \brief Batch setting controls are visible
* \param control_id control ID
* \param visible frame ID
*/
void BatchSetVisible(uint16 control_id,uint8 visible)
{
    TX_16(control_id);
    TX_8(1);
    TX_8(visible);
}
/*!
* \brief batch setting control enable
* \param control_id control ID
* \param enable frame ID
*/
void BatchSetEnable(uint16 control_id,uint8 enable)
{
    TX_16(control_id);
    TX_8(2);
    TX_8(enable);
}

#endif
/*!
* \brief end batch update
*/
void BatchEnd()
{
    END_CMD();
}
/*!
* \brief set countdown control
* \param screen_id screen ID
* \param control_id control ID
* \param timeout countdown (seconds)
*/
void SeTimer(uint16 screen_id,uint16 control_id,uint32 timeout)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x40);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(timeout);
    END_CMD();
}
/*!
* \brief turn on the countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void StartTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x41);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief stop countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void StopTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x42);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief Pause countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void PauseTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x44);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief set the background color of the control
* \details supports controls: progress bar, text
* \param screen_id screen ID
* \param control_id control ID
* \param color background color
*/
void SetControlBackColor(uint16 screen_id,uint16 control_id,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x18);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(color);
    END_CMD();
}
/*!
* \brief set the foreground color of the control
* \details Support control: progress bar
* \param screen_id screen ID
* \param control_id control ID
* \param color foreground color
*/
void SetControlForeColor(uint16 screen_id,uint16 control_id,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x19);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(color);
    END_CMD();
}
/*!
* \brief show\hide popup menu controls
* \param screen_id screen ID
* \param control_id control ID
* \param show whether to display, focus_control_id is invalid when it is 0
* \param focus_control_id associated text control (the content of the menu control is output to the text control)
*/
void ShowPopupMenu(uint16 screen_id,uint16 control_id,uint8 show,uint16 focus_control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x13);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(show);
    TX_16(focus_control_id);
    END_CMD();
}
/*!
* \brief show\hide system keyboard
* \param show 0 hides, 1 shows
* \param x keyboard display position X coordinate
* \param y Y coordinate of keyboard display position
* \param type 0 small keyboard, 1 full keyboard
* \param option 0 normal characters, 1 password, 2 time setting
* \param max_len keyboard input character length limit
*/
void ShowKeyboard(uint8 show,uint16 x,uint16 y,uint8 type,uint8 option,uint8 max_len)
{
    BEGIN_CMD();
    TX_8(0x86);
    TX_8(show);
    TX_16(x);
    TX_16(y);
    TX_8(type);
    TX_8(option);
    TX_8(max_len);
    END_CMD();
}

#if FIRMWARE_VER>=921
/*!
* \brief multi-language settings
* \param ui_lang User interface language 0~9
* \param sys_lang System keyboard language-0 Chinese, 1 English
*/
void SetLanguage(uint8 ui_lang,uint8 sys_lang)
{
    uint8 lang = ui_lang;
    if(sys_lang)    lang |= 0x80;

    BEGIN_CMD();
    TX_8(0xC1);
    TX_8(lang);
    TX_8(0xC1+lang);//Check to prevent accidental modification of language
    END_CMD();
}
#endif


#if FIRMWARE_VER>=921
/*!
* \brief starts to save the control value to FLASH
* \param version Data version number, can be arbitrarily specified, the high 16 bits are the main version number, and the low 16 bits are the minor version number
* \param address The storage address of the data in the user storage area, pay attention to avoid address overlap and conflict
*/
void FlashBeginSaveControl(uint32 version,uint32 address)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0xAA);
    TX_32(version);
    TX_32(address);
}

/*!
* \brief save the value of a control to FLASH
* \param screen_id screen ID
* \param control_id control ID
*/
void FlashSaveControl(uint16 screen_id,uint16 control_id)
{
    TX_16(screen_id);
    TX_16(control_id);
}
/*!
* \brief save the value of a control to FLASH
* \param screen_id screen ID
* \param control_id control ID
*/
void FlashEndSaveControl()
{
    END_CMD();
}
/*!
* \brief restore control data from FLASH
* \param version Data version number, the main version number must be the same as when stored, otherwise it will fail to load
* \param address The storage address of the data in the user storage area
*/
void FlashRestoreControl(uint32 version,uint32 address)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0xAB);
    TX_32(version);
    TX_32(address);
    END_CMD();
}

#endif

#if FIRMWARE_VER>=921
/*!
* \brief Set historical curve sampling data value (single byte, uint8 or int8)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt8(uint16 screen_id,uint16 control_id,uint8 *value,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8N(value,channel);
    END_CMD();
}
/*!
* \brief Set historical curve sampling data value (double byte, uint16 or int16)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt16(uint16 screen_id,uint16 control_id,uint16 *value,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16N(value,channel);
    END_CMD();
}
/*!
* \brief Set historical curve sampling data value (four bytes, uint32 or int32)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt32(uint16 screen_id,uint16 control_id,uint32 *value,uint8 channel)
{
    uint8 i = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);

    for (;i<channel;++i)
    {
        TX_32(value[i]);
    }

    END_CMD();
}
/*!
* \brief Set historical curve sampling data value (single precision floating point number)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueFloat(uint16 screen_id,uint16 control_id,float *value,uint8 channel)
{
    uint8 i = 0;
    uint32 tmp = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);

    for (;i<channel;++i)
    {
        tmp = *(uint32 *)(value+i);
        TX_32(tmp);
    }

    END_CMD();
}
/*!
* \brief allows or prohibits historical curve sampling
* \param screen_id screen ID
* \param control_id control ID
* \param enable 0-prohibit, 1-allow
*/
void HistoryGraph_EnableSampling(uint16 screen_id,uint16 control_id,uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x61);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(enable);
    END_CMD();
}
/*!
* \brief show or hide historical curve channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param show 0-hide, 1-show
*/
void HistoryGraph_ShowChannel(uint16 screen_id,uint16 control_id,uint8 channel,uint8 show)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x62);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_8(show);
    END_CMD();
}
/*!
* \brief Set the time length of the historical curve (ie the number of sampling points)
* \param screen_id screen ID
* \param control_id control ID
* \param sample_count The number of sample points displayed on one screen
*/
void HistoryGraph_SetTimeLength(uint16 screen_id,uint16 control_id,uint16 sample_count)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x00);
    TX_16(sample_count);
    END_CMD();
}

/*!
* \brief zoom history curve to full screen
* \param screen_id screen ID
* \param control_id control ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x01);
    END_CMD();
}
/*!
* \brief set the historical curve scaling factor
* \param screen_id screen ID
* \param control_id control ID
* \param zoom zoom percentage (horizontal zoom when zoom>100%, zoom in anyway)
* \param max_zoom zoom limit, the maximum number of sampling points displayed on a screen
* \param min_zoom zoom limit, the minimum number of sampling points displayed on a screen
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id,uint16 control_id,uint16 zoom,uint16 max_zoom,uint16 min_zoom)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x02);
    TX_16(zoom);
    TX_16(max_zoom);
    TX_16(min_zoom);
    END_CMD();
}

#endif

#if SD_FILE_EN
/*!
* \brief detects whether the SD card is inserted
*/
void SD_IsInsert(void)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x01);
    END_CMD();
}
/*!
* \brief open or create file
* \param filename file name (ASCII encoding only)
* \param mode mode, optional combination mode is as above FA_XXXX
*/
void SD_CreateFile(uint8 *filename,uint8 mode)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x05);
    TX_8(mode);
    SendStrings(filename);
    END_CMD();
}
/*!
* \brief creates a file with the current time, for example: 20161015083000.txt
* \param ext file suffix, such as txt
*/
void SD_CreateFileByTime(uint8 *ext)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x02);
    SendStrings(ext);
    END_CMD();
}
/*!
* \brief write data at the end of the current file
* \param buffer data
* \param dlc data length
*/
void SD_WriteFile(uint8 *buffer,uint16 dlc)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x03);
    TX_16(dlc);
    TX_8N(buffer,dlc);
    END_CMD();
}
/*!
* \brief read the current file
* \param offset file position offset
* \param dlc data length
*/
void SD_ReadFile(uint32 offset,uint16 dlc)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x07);
    TX_32(offset);
    TX_16(dlc);
    END_CMD();
}
/*!
* \brief Get the current file length
*/
void SD_GetFileSize()
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x06);
    END_CMD();
}
/*!
* \brief close the current file
*/
void SD_CloseFile()
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x04);
    END_CMD();
}

#endif//SD_FILE_EN
/*!
* \brief record control-trigger warning
* \param screen_id screen ID
* \param control_id control ID
* \param value alarm value
* \param time The time when the alarm is generated, the internal time of the screen is used when it is 0
*/
void Record_SetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time)
{
    uint8 i  = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x50);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);

    if(time)
    {
        for(i=0;i<7;++i)
            TX_8(time[i]);
    }

    END_CMD();
}
/*!
* \brief record control-remove warning
* \param screen_id screen ID
* \param control_id control ID
* \param value alarm value
* \param time The time when the alarm is cancelled, the internal time of the screen is used when it is 0
*/
void Record_ResetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time)
{
    uint8 i  = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x51);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);

    if(time)
    {
        for(i=0;i<7;++i)
            TX_8(time[i]);
    }

    END_CMD();
}
/*!
* \brief record control-add regular records
* \param screen_id screen ID
* \param control_id control ID
* \param record A record (string), the sub-items are separated by semicolons, for example: the first item; the second item; the third item;
*/
void Record_Add(uint16 screen_id,uint16 control_id,uint8 *record)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x52);
    TX_16(screen_id);
    TX_16(control_id);

    SendStrings(record);

    END_CMD();
}
/*!
* \brief record control-clear record data
* \param screen_id screen ID
* \param control_id control ID
*/
void Record_Clear(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x53);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief record control-set record display offset
* \param screen_id screen ID
* \param control_id control ID
* \param offset display offset, scroll bar position
*/
void Record_SetOffset(uint16 screen_id,uint16 control_id,uint16 offset)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x54);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(offset);
    END_CMD();
}
/*!
* \brief record control-get the current record number
* \param screen_id screen ID
* \param control_id control ID
*/
void Record_GetCount(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x55);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
* \brief read screen RTC time
*/
void ReadRTC(void)
{
    BEGIN_CMD();
    TX_8(0x82);
    END_CMD();
}

/*!
* \brief play music
* \param buffer hexadecimal music path and name
*/
void PlayMusic(uint8 *buffer)
{
    uint8 i  = 0;

    BEGIN_CMD();
    if(buffer)
    {
        for(i=0;i<19;++i)
            TX_8(buffer[i]);
    }
    END_CMD();
}
