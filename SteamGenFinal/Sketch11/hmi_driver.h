 

#ifndef _HMI_DRIVER_
#define _HMI_DRIVER_

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;
typedef unsigned char uchar;
typedef signed int int16;

#define FIRMWARE_VER 921
// #define CRC16_ENABLE 0
#define CMD_MAX_SIZE 64
#define QUEUE_MAX_SIZE 512

#define SD_FILE_EN 0


/*!
* \brief Check whether the data meets CRC16 check
* \param buffer Data to be verified, CRC16 is stored at the end
* \param n data length, including CRC16
* \return Return 1 if the check is passed, otherwise 0
*/
uint16 CheckCRC16(uint8 *buffer,uint16 n);

/*!
* \brief delay
* \param n Delay time (in milliseconds)
*/
void DelayMS(unsigned int n);

/*!
* \brief Lock the device configuration, after locking, you need to unlock it to modify the baud rate, touch screen, and buzzer working mode
*/
void LockDeviceConfig(void);

/*!
* \brief unlock device configuration
*/
void UnlockDeviceConfig(void);

/*!
* \brief modify the baud rate of the serial screen
* \details Baud rate option range [0~14], corresponding to actual baud rate
{1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
* \param option Baud rate option
*/
void SetCommBps(uint8 option);

/*!
* \brief send handshake command
*/
void SetHandShake(void);

/*!
* \brief set the foreground color
* \param color foreground color
*/
void SetFcolor(uint16 color);

/*!
* \brief set background color
* \param color background color
*/
void SetBcolor(uint16 color);

/*!
* \brief clear screen
*/
void GUI_CleanScreen();

/*!
* \brief set text interval
* \param x_w horizontal interval
* \param y_w vertical interval
*/
void SetTextSpace(uint8 x_w, uint8 y_w);

/*!
* \brief set text display limit
* \param enable Whether to enable restriction
* \param width width
* \param height height
*/
void SetFont_Region(uint8 enable,uint16 width,uint16 height );

/*!
* \brief set filter color
* \param fillcolor_dwon color lower bound
* \param fillcolor_up color upper bound
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up);

/*!
* \brief set filter color
* \param x position X coordinate
* \param y position Y coordinate
* \param back color upper bound
* \param font font
* \param strings string content
*/
void DisText(uint16 x, uint16 y,uint8 back,uint8 font,uchar *strings );

/*!
* \brief show cursor
* \param enable whether to display
* \param x position X coordinate
* \param y position Y coordinate
* \param width width
* \param height height
*/
void DisCursor(uint8 enable,uint16 x, uint16 y,uint8 width,uint8 height );

/*!
* \brief display full screen picture
* \param image_id image index
* \param masken whether to enable transparent mask
*/
void DisFull_Image(uint16 image_id,uint8 masken);

/*!
* \brief Display the picture at the specified location
* \param x position X coordinate
* \param y position Y coordinate
* \param image_id image index
* \param masken whether to enable transparent mask
*/
void DisArea_Image(uint16 x,uint16 y,uint16 image_id,uint8 masken);

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
void DisCut_Image(uint16 x,uint16 y,uint16 image_id,uint16 image_x,uint16 image_y,
				  uint16 image_l, uint16 image_w,uint8 masken);

/*!
* \brief display GIF animation
* \param x position X coordinate
* \param y position Y coordinate
* \param flashimage_id image index
* \param enable whether to display
* \param playnum play count
*/
void DisFlashImage(uint16 x,uint16 y,uint16 flashimage_id,uint8 enable,uint8 playnum);

/*!
* \brief draw points
* \param x position X coordinate
* \param y position Y coordinate
*/
void GUI_Dot(uint16 x,uint16 y);

/*!
* \brief draw a line
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1);

/*!
* \brief draw a polyline
* \param mode mode
* \param dot data point
* \param dot_cnt points
*/
void GUI_ConDots(uint8 mode,uint16 *dot,uint16 dot_cnt);

/*!
* \brief draw a hollow circle
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
*/
void GUI_Circle(uint16 x0, uint16 y0, uint16 r);

/*!
* \brief draw a solid circle
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
*/
void GUI_CircleFill(uint16 x0, uint16 y0, uint16 r);

/*!
* \brief draw arc
* \param x0 X coordinate of center position
* \param y0 Y coordinate of center position
* \param r radius
* \param sa starting angle
* \param ea end angle
*/
void GUI_Arc(uint16 x,uint16 y, uint16 r,uint16 sa, uint16 ea);

/*!
* \brief draw a hollow rectangle
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1,uint16 y1 );

/*!
* \brief draw a solid rectangle
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1,uint16 y1 );

/*!
* \brief draw a hollow ellipse
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_Ellipse (uint16 x0, uint16 y0, uint16 x1,uint16 y1 );

/*!
* \brief draw a solid ellipse
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void GUI_EllipseFill (uint16 x0, uint16 y0, uint16 x1,uint16 y1 );

/*!
* \brief draw a line
* \param x0 starting position X coordinate
* \param y0 Y coordinate of starting position
* \param x1 End position X coordinate
* \param y1 Y coordinate of end position
*/
void SetBackLight(uint8 light_level);

/*!
* \brief Buzzer settings
* \time time duration (in milliseconds)
*/
void SetBuzzer(uint8 time);

/*!
* \brief touch screen settings
* \param enable touch enable
* \param beep_on touch buzzer
* \param work_mode Touch working mode: 0 press to upload, 1 release to upload, 2 keep uploading coordinate values, 3 press and release to upload data
* \param press_calibration Continuously click the touch screen 20 times to calibrate the touch screen: 0 disable, 1 enable
*/
void SetTouchPaneOption(uint8 enbale,uint8 beep_on,uint8 work_mode,uint8 press_calibration);

/*!
* \brief Calibrate the touch screen
*/
void	CalibrateTouchPane();

/*!
* \brief touch screen test
*/
void TestTouchPane();

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
void WriteLayer(uint8 layer);

/*!
* \brief Set the current display layer
* \param layer layer number
*/
void DisplyLayer(uint8 layer);

/*!
* \brief clear the layer and make the layer transparent
* \param layer layer number
*/
void ClearLayer(uint8 layer);

/*!
* \brief Write data to the user storage area of ??the serial screen
* \param startAddress starting address
* \param length bytes
* \param _data The data to be written
*/
void WriteUserFlash(uint32 startAddress,uint16 length,uint8 *_data);

/*!
* \brief read data from the user storage area of ??the serial screen
* \param startAddress starting address
* \param length bytes
*/
void ReadUserFlash(uint32 startAddress,uint16 length);

/*!
* \brief copy layer
* \param src_layer original layer
* \param dest_layer target layer
*/
void CopyLayer(uint8 src_layer,uint8 dest_layer);

/*!
* \brief set the current screen
* \param screen_id screen ID
*/
void SetScreen(uint16 screen_id);

/*!
* \brief Get the current picture
*/
void GetScreen(uint16 screen_id);

/*!
* \brief disable\enable screen update
* \details disable\enable general use in pairs to avoid flicker and improve refresh speed
* \details usage:
* \details SetScreenUpdateEnable(0);//Update is prohibited
* \details A series of commands to update the screen
* \details SetScreenUpdateEnable(1);//Update immediately
* \param enable 0 is disabled, 1 is enabled
*/
void SetScreenUpdateEnable(uint8 enable);

/*!
* \brief set control input focus
* \param screen_id screen ID
* \param control_id control ID
* Whether \param focus has input focus
*/
void SetControlFocus(uint16 screen_id,uint16 control_id,uint8 focus);

/*!
* \brief show\hide controls
* \param screen_id screen ID
* \param control_id control ID
* \param visible whether to display
*/
void SetControlVisiable(uint16 screen_id,uint16 control_id,uint8 visible);

/*!
* \brief set touch control enable
* \param screen_id screen ID
* \param control_id control ID
* \param enable Whether the control is enabled
*/
void SetControlEnable(uint16 screen_id,uint16 control_id,uint8 enable);

/*!
* \brief Get control value
* \param screen_id screen ID
* \param control_id control ID
*/
void GetControlValue(uint16 screen_id,uint16 control_id);

/*!
* \brief set button state
* \param screen_id screen ID
* \param control_id control ID
* \param value button state
*/
void SetButtonValue(uint16 screen_id,uint16 control_id,uchar value);

/*!
* \brief set text value
* \param screen_id screen ID
* \param control_id control ID
* \param str text value
*/
void SetTextValue(uint16 screen_id,uint16 control_id,uchar *str);

#if FIRMWARE_VER>=908

/*!
* \brief sets the text to an integer value, requiring FIRMWARE_VER>=908
* \param screen_id screen ID
* \param control_id control ID
* \param value text value
* \param sign 0-unsigned, 1-signed
* \param fill_zero Number of digits, if not enough
*/
void SetTextInt32(uint16 screen_id,uint16 control_id,uint32 value,uint8 sign,uint8 fill_zero);

/*!
* \brief Set the text single-precision floating point value, requiring FIRMWARE_VER>=908
* \param screen_id screen ID
* \param control_id control ID
* \param value text value
* \param precision decimal places
* When \param show_zeros is 1, the end 0 is displayed
*/
void SetTextFloat(uint16 screen_id,uint16 control_id,float value,uint8 precision,uint8 show_zeros);

#endif

/*!
* \brief set progress value
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/
void SetProgressValue(uint16 screen_id,uint16 control_id,uint32 value);

/*!
* \brief set meter value
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/
void SetMeterValue(uint16 screen_id,uint16 control_id,uint32 value);

/*!
* \brief set meter value
* \param screen_id screen ID
* \param control_id Image control ID
* \param value
*/
void Set_picMeterValue(uint16 screen_id,uint16 control_id,uint16 value);

/*!
* \brief set the slider
* \param screen_id screen ID
* \param control_id control ID
* \param value
*/

void SetSliderValue(uint16 screen_id,uint16 control_id,uint32 value);

/*!
* \brief set selection control
* \param screen_id screen ID
* \param control_id control ID
* \param item current option
*/
void SetSelectorValue(uint16 screen_id,uint16 control_id,uint8 item);

/*!
* \brief start to play animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationStart(uint16 screen_id,uint16 control_id);

/*!
* \brief stop the animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationStop(uint16 screen_id,uint16 control_id);

/*!
* \brief Pause the animation
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPause(uint16 screen_id,uint16 control_id);

/*!
* \brief play the specified frame
* \param screen_id screen ID
* \param control_id control ID
* \param frame_id frame ID
*/
void AnimationPlayFrame(uint16 screen_id,uint16 control_id,uint8 frame_id);

/*!
* \brief play the previous frame
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPlayPrev(uint16 screen_id,uint16 control_id);

/*!
* \brief play the next frame
* \param screen_id screen ID
* \param control_id control ID
*/
void AnimationPlayNext(uint16 screen_id,uint16 control_id);

/*!
* \brief curve control-add channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param color color
*/
void GraphChannelAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint16 color);

/*!
* \brief curve control-delete channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
*/
void GraphChannelDel(uint16 screen_id,uint16 control_id,uint8 channel);

/*!
* \brief curve control-add data
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param pData Curve data
* \param nDataLen Number of data
*/
void GraphChannelDataAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint8 *pData,uint16 nDataLen);

/*!
* \brief curve control-clear data
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
*/
void GraphChannelDataClear(uint16 screen_id,uint16 control_id,uint8 channel);

/*!
* \brief curve control-set view window
* \param screen_id screen ID
* \param control_id control ID
* \param x_offset horizontal offset
* \param x_mul horizontal zoom factor
* \param y_offset vertical offset
* \param y_mul vertical zoom factor
*/
void GraphSetViewport(uint16 screen_id,uint16 control_id,int16 x_offset,uint16 x_mul,int16 y_offset,uint16 y_mul);

/*!
* \brief start batch update
* \param screen_id screen ID
*/
void BatchBegin(uint16 screen_id);

/*!
* \brief Batch update button controls
* \param control_id control ID
* \param value
*/
void BatchSetButtonValue(uint16 control_id,uint8 state);

/*!
* \brief batch update progress bar controls
* \param control_id control ID
* \param value
*/
void BatchSetProgressValue(uint16 control_id,uint32 value);

/*!
* \brief batch update slider controls
* \param control_id control ID
* \param value
*/
void BatchSetSliderValue(uint16 control_id,uint32 value);

/*!
* \brief batch update instrument controls
* \param control_id control ID
* \param value
*/
void BatchSetMeterValue(uint16 control_id,uint32 value);

/*!
* \brief calculate string length
*/
uint32 GetStringLen(uchar *str);

/*!
* \brief batch update text controls
* \param control_id control ID
* \param strings
*/
void BatchSetText(uint16 control_id,uchar *strings);

/*!
* \brief batch update animation\icon controls
* \param control_id control ID
* \param frame_id frame ID
*/
void BatchSetFrame(uint16 control_id,uint16 frame_id);

#if FIRMWARE_VER>=921

/*!
* \brief Batch setting controls are visible
* \param control_id control ID
* \param visible frame ID
*/
void BatchSetVisible(uint16 control_id,uint8 visible);

/*!
* \brief Batch setting control enable
* \param control_id control ID
* \param enable frame ID
*/
void BatchSetEnable(uint16 control_id,uint8 enable);

#endif

/*!
* \brief end batch update
*/
void BatchEnd();

/*!
* \brief set countdown control
* \param screen_id screen ID
* \param control_id control ID
* \param timeout countdown (seconds)
*/
void SeTimer(uint16 screen_id,uint16 control_id,uint32 timeout);

/*!
* \brief turn on the countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void StartTimer(uint16 screen_id,uint16 control_id);

/*!
* \brief stop countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void StopTimer(uint16 screen_id,uint16 control_id);

/*!
* \brief Pause countdown control
* \param screen_id screen ID
* \param control_id control ID
*/
void PauseTimer(uint16 screen_id,uint16 control_id);

/*!
* \brief set the background color of the control
* \details supports controls: progress bar, text
* \param screen_id screen ID
* \param control_id control ID
* \param color background color
*/
void SetControlBackColor(uint16 screen_id,uint16 control_id,uint16 color);

/*!
* \brief set the foreground color of the control
* \details Support control: progress bar
* \param screen_id screen ID
* \param control_id control ID
* \param color foreground color
*/
void SetControlForeColor(uint16 screen_id,uint16 control_id,uint16 color);

/*!
* \brief show\hide pop-up menu controls
* \param screen_id screen ID
* \param control_id control ID
* \param show whether to display, focus_control_id is invalid when it is 0
* \param focus_control_id associated text control (the content of the menu control is output to the text control)
*/
void ShowPopupMenu(uint16 screen_id,uint16 control_id,uint8 show,uint16 focus_control_id);

/*!
* \brief show\hide system keyboard
* \param show 0 hides, 1 shows
* \param x keyboard display position X coordinate
* \param y Y coordinate of keyboard display position
* \param type 0 small keyboard, 1 full keyboard
* \param option 0 normal characters, 1 password, 2 time setting
* \param max_len keyboard input character length limit
*/
void ShowKeyboard(uint8 show,uint16 x,uint16 y,uint8 type,uint8 option,uint8 max_len);

#if FIRMWARE_VER>=914
/*!
* \brief multi-language settings
* \param ui_lang User interface language 0~9
* \param sys_lang System keyboard language-0 Chinese, 1 English
*/
void SetLanguage(uint8 ui_lang,uint8 sys_lang);
#endif

#if FIRMWARE_VER>=917
/*!
* \brief starts to save the control value to FLASH
* \param version Data version number, can be arbitrarily specified, the high 16 bits are the main version number, and the low 16 bits are the minor version number
* \param address The storage address of the data in the user storage area, pay attention to avoid address overlap and conflict
*/
void FlashBeginSaveControl(uint32 version,uint32 address);

/*!
* \brief save the value of a control to FLASH
* \param screen_id screen ID
* \param control_id control ID
*/
void FlashSaveControl(uint16 screen_id,uint16 control_id);

/*!
* \brief end of save
*/
void FlashEndSaveControl();

/*!
* \brief restore control data from FLASH
* \param version Data version number, the main version number must be the same as when stored, otherwise it will fail to load
* \param address The storage address of the data in the user storage area
*/
void FlashRestoreControl(uint32 version,uint32 address);
#endif

#if FIRMWARE_VER>=921
/*!
* \brief Set historical curve sampling data value (single byte, uint8 or int8)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt8(uint16 screen_id,uint16 control_id,uint8 *value,uint8 channel);

/*!
* \brief Set historical curve sampling data value (double byte, uint16 or int16)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt16(uint16 screen_id,uint16 control_id,uint16 *value,uint8 channel);

/*!
* \brief Set historical curve sampling data value (four bytes, uint32 or int32)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueInt32(uint16 screen_id,uint16 control_id,uint32 *value,uint8 channel);

/*!
* \brief Set historical curve sampling data value (single precision floating point number)
* \param screen_id screen ID
* \param control_id control ID
* \param value sampling point data
* \param channel number of channels
*/
void HistoryGraph_SetValueFloat(uint16 screen_id,uint16 control_id,float *value,uint8 channel);

/*!
* \brief allows or prohibits historical curve sampling
* \param screen_id screen ID
* \param control_id control ID
* \param enable 0-prohibit, 1-allow
*/
void HistoryGraph_EnableSampling(uint16 screen_id,uint16 control_id,uint8 enable);

/*!
* \brief show or hide historical curve channel
* \param screen_id screen ID
* \param control_id control ID
* \param channel channel number
* \param show 0-hide, 1-show
*/
void HistoryGraph_ShowChannel(uint16 screen_id,uint16 control_id,uint8 channel,uint8 show);


/*!
* \brief Set the time length of the historical curve (ie the number of sampling points)
* \param screen_id screen ID
* \param control_id control ID
* \param sample_count The number of sample points displayed on one screen
*/
void HistoryGraph_SetTimeLength(uint16 screen_id,uint16 control_id,uint16 sample_count);

/*!
* \brief zoom history curve to full screen
* \param screen_id screen ID
* \param control_id control ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id,uint16 control_id);

/*!
* \brief set the historical curve scaling factor
* \param screen_id screen ID
* \param control_id control ID
* \param zoom zoom percentage (horizontal zoom when zoom>100%, zoom in anyway)
* \param max_zoom zoom limit, the maximum number of sampling points displayed on a screen
* \param min_zoom zoom limit, the minimum number of sampling points displayed on a screen
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id,uint16 control_id,uint16 zoom,uint16 max_zoom,uint16 min_zoom);
#endif

#if SD_FILE_EN
/*!
* \brief detects whether the SD card is inserted
*/
void SD_IsInsert(void);

#define FA_READ 0x01 // Can be read
#define FA_WRITE 0x02 // Can be written
#define FA_CREATE_NEW 0x04 // Create a new file, if the file already exists, return failure
#define FA_CREATE_ALWAYS 0x08 // Create a new file, if the file already exists, it will be overwritten
#define FA_OPEN_EXISTING 0x00 // Open the file, if the file does not exist, return failure
#define FA_OPEN_ALWAYS 0x10 // Open the file, if the file does not exist, create a new file

/*!
* \brief open or create file
* \param filename file name (ASCII encoding only)
* \param mode mode, optional combination mode is as above FA_XXXX
*/
void SD_CreateFile(uint8 *filename,uint8 mode);

/*!
* \brief creates a file with the current time, for example: 20161015083000.txt
* \param ext file suffix, such as txt
*/
void SD_CreateFileByTime(uint8 *ext);

/*!
* \brief write data at the end of the current file
* \param buffer data
* \param dlc data length
*/
void SD_WriteFile(uint8 *buffer,uint16 dlc);

/*!
* \brief read the current file
* \param offset file position offset
* \param dlc data length
*/
void SD_ReadFile(uint32 offset,uint16 dlc);

/*!
* \brief Get the current file length
*/
void SD_GetFileSize();

/*!
* \brief close the current file
*/
void SD_CloseFile();
#endif

/*!
* \brief record control-trigger warning
* \param screen_id screen ID
* \param control_id control ID
* \param value alarm value
* \param time The time when the alarm is generated, the internal time of the screen is used when it is 0
*/
void Record_SetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time);

/*!
* \brief record control-remove warning
* \param screen_id screen ID
* \param control_id control ID
* \param value alarm value
* \param time The time when the alarm is cancelled, the internal time of the screen is used when it is 0
*/
void Record_ResetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time);

/*!
* \brief record control-add regular records
* \param screen_id screen ID
* \param control_id control ID
* \param record A record (string), the sub-items are separated by semicolons, for example: the first item; the second item; the third item;
*/
void Record_Add(uint16 screen_id,uint16 control_id,uint8 *record);

/*!
* \brief record control-clear record data
* \param screen_id screen ID
* \param control_id control ID
*/
void Record_Clear(uint16 screen_id,uint16 control_id);

/*!
* \brief record control-set record display offset
* \param screen_id screen ID
* \param control_id control ID
* \param offset display offset, scroll bar position
*/
void Record_SetOffset(uint16 screen_id,uint16 control_id,uint16 offset);

/*!
* \brief record control-get the current record number
* \param screen_id screen ID
* \param control_id control ID
*/
void Record_GetCount(uint16 screen_id,uint16 control_id);


/*!
* \brief read screen RTC time
*/
void ReadRTC(void);

/*!
* \brief play music
* \param buffer hexadecimal music path and name
*/
void PlayMusic(uint8 *buffer);

void SendChar(uint8 t);

#endif //_HMI_DRIVER_
