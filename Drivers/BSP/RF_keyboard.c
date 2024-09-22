//
// Created by Shin on 2024/4/19.
//

#include "RF_keyboard.h"
#include "main.h"
#include "string.h"
#include "usart.h"
#include "usbd_core.h"
#include "RFK_USB_CODE.h"
#include "usbd_customhid.h"
#include "adc.h"

#define BYTE0(dwTemp) (*((char *)(&dwTemp)))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))

#define TOUCH_BAR_LEFT      PAGE_UP
#define TOUCH_BAR_RIGHT     PAGE_DOWN

#define JOY_LEFT            LEFT_ARROW
#define JOY_RIGHT           RIGHT_ARROW
#define JOY_UP              UP_ARROW
#define JOY_DOWN            DOWN_ARROW

#define JOY_VALUE_LEFT      1500
#define JOY_VALUE_RIGHT     2500
#define JOY_VALUE_UP        1500
#define JOY_VALUE_DOWN      2500

_RF_Keyboard_st RFK;
extern USBD_HandleTypeDef hUsbDeviceFS;

//高八位(BYTE1)代表按mask上的按键在键盘的第几个bit上
//第八位(BYTE0)代表按mask上的按键在键盘的第几个byte上
//举例:报文第40个是数字'2',且绑定在了第5个按键上
//BYTE0(remap[40]) = 0;
//BYTE1(remap[40]) = 0x01 << (5-1);
//RFK.hidBuffer[i] = RFK.hidBuffer[i] | (RFK.mapBuffer[BYTE0(remap[i*8+j])]|BYTE1(remap[i*8+j]));
uint16_t remap1[200] = {0};
uint16_t remap2[200] = {0};
uint16_t remap3[200] = {0};

inline void DelayUs(uint32_t _us)
{
    for (int i = 0; i < _us; i++)
        for (int j = 0; j < 8; j++)  // ToDo: tune this for different chips
            __NOP();
}

uint8_t _switch_map(uint8_t map)
{
    if(map == 1)
    {
        RFK.remapPtr = remap1;
    }
    if(map == 2)
    {
        RFK.remapPtr = remap2;
    }
    if(map == 3)
    {
        RFK.remapPtr = remap3;
    }
}

uint8_t _remap_init(void)
{
    RFK.remapPtr = remap1;

//    memset(remap1, 0xFF, 128);

    BYTE0(remap1[PAD_NUM_1]) = 0;//1
    BYTE0(remap1[PAD_NUM_2]) = 0;//2
    BYTE0(remap1[PAD_NUM_3]) = 0;//3
    BYTE0(remap1[PAD_NUM_4]) = 0;//4
    BYTE0(remap1[PAD_NUM_5]) = 0;//5
    BYTE0(remap1[PAD_NUM_6]) = 0;//6
    BYTE0(remap1[PAD_NUM_7]) = 1;//7
    BYTE0(remap1[PAD_NUM_8]) = 1;//8
    BYTE0(remap1[PAD_NUM_9]) = 1;//9
    BYTE0(remap1[PAD_NUM_0]) = 2;//0

    BYTE1(remap1[PAD_NUM_1]) = 0;//1
    BYTE1(remap1[PAD_NUM_2]) = 4;//2
    BYTE1(remap1[PAD_NUM_3]) = 6;//3
    BYTE1(remap1[PAD_NUM_4]) = 3;//4
    BYTE1(remap1[PAD_NUM_5]) = 2;//5
    BYTE1(remap1[PAD_NUM_6]) = 1;//6
    BYTE1(remap1[PAD_NUM_7]) = 4;//7
    BYTE1(remap1[PAD_NUM_8]) = 5;//8
    BYTE1(remap1[PAD_NUM_9]) = 6;//9
    BYTE1(remap1[PAD_NUM_0]) = 0;//0

    BYTE0(remap1[PAD_DEL]) = 0;//.
    BYTE0(remap1[PAD_NUM_LOCK]) = 1;//Num Lock
    BYTE0(remap1[PAD_PLUS]) = 1;//+
    BYTE0(remap1[PAD_MINUS]) = 1;//-
    BYTE0(remap1[PAD_ASTERISK]) = 1;//*
    BYTE0(remap1[PAD_SLASH]) = 1;///
    BYTE0(remap1[PAD_ENTER]) = 0;//Enter
    BYTE0(remap1[LEFT_GUI]) = 2;//windows

    BYTE1(remap1[PAD_DEL]) = 5;//.
    BYTE1(remap1[PAD_NUM_LOCK]) = 3;//Num Lock
    BYTE1(remap1[PAD_PLUS]) = 7;//+
    BYTE1(remap1[PAD_MINUS]) = 0;//-
    BYTE1(remap1[PAD_ASTERISK]) = 1;//*
    BYTE1(remap1[PAD_SLASH]) = 2;///
    BYTE1(remap1[PAD_ENTER]) = 7;//Enter
    BYTE1(remap1[LEFT_GUI]) = 1;//windows


    BYTE0(remap2[F1]) = 0;//F1
    BYTE0(remap2[F2]) = 0;//F2
    BYTE0(remap2[F3]) = 0;//F3
    BYTE0(remap2[F4]) = 0;//F4
    BYTE0(remap2[F5]) = 0;//F5
    BYTE0(remap2[F6]) = 0;//F6
    BYTE0(remap2[F7]) = 1;//F7
    BYTE0(remap2[F8]) = 1;//F8
    BYTE0(remap2[F9]) = 1;//F9
    BYTE0(remap2[F10]) = 1;//F10
    BYTE0(remap2[F11]) = 1;//F11
    BYTE0(remap2[F12]) = 1;//F12
    BYTE0(remap2[NUM_9]) = 2;// 9/(
    BYTE0(remap2[NUM_0]) = 0;// 0/)
    BYTE0(remap2[LEFT_SHIFT]) = 0;//Shift
    BYTE0(remap2[LEFT_U_BRACE]) = 1;// [/{
    BYTE0(remap2[RIGHT_U_BRACE]) = 1;// ]/}

    BYTE1(remap2[F1]) = 0;//F1
    BYTE1(remap2[F2]) = 4;//F2
    BYTE1(remap2[F3]) = 6;//F3
    BYTE1(remap2[F4]) = 3;//F4
    BYTE1(remap2[F5]) = 2;//F5
    BYTE1(remap2[F6]) = 1;//F6
    BYTE1(remap2[F7]) = 4;//F7
    BYTE1(remap2[F8]) = 5;//F8
    BYTE1(remap2[F9]) = 6;//F9
    BYTE1(remap2[F10]) = 3;//F10
    BYTE1(remap2[F11]) = 2;//F11
    BYTE1(remap2[F12]) = 1;//F12
    BYTE1(remap2[NUM_9]) = 0;// 9/(
    BYTE1(remap2[NUM_0]) = 5;// 0/)
    BYTE1(remap2[LEFT_SHIFT]) = 7;//Shift
    BYTE1(remap2[LEFT_U_BRACE]) = 0;// [/{
    BYTE1(remap2[RIGHT_U_BRACE]) = 7;// ]/}

    BYTE0(remap3[A]) = 0;//A
    BYTE0(remap3[C]) = 0;//C
    BYTE0(remap3[V]) = 0;//V
    BYTE0(remap3[F]) = 0;//F
    BYTE0(remap3[R]) = 0;//R
    BYTE0(remap3[G]) = 0;//G
    BYTE0(remap3[H]) = 1;//H
    BYTE0(remap3[Q]) = 1;//Q
    BYTE0(remap3[P]) = 1;//P
    BYTE0(remap3[CAP_LOCK]) = 1;//CAP_LOCK
    BYTE0(remap3[QUOTE]) = 1;// '/"
    BYTE0(remap3[SEMI_COLON]) = 1;//;/:
    BYTE0(remap3[LEFT_CTRL]) = 2;// CTRL
    BYTE0(remap3[LEFT_ALT]) = 0;// ALT
    BYTE0(remap3[LEFT_SHIFT]) = 0;//Shift
    BYTE0(remap3[ESC]) = 1;// ESC
    BYTE0(remap3[TAB]) = 1;// TAB

    BYTE1(remap3[A]) = 0;//A
    BYTE1(remap3[C]) = 4;//C
    BYTE1(remap3[V]) = 6;//V
    BYTE1(remap3[F]) = 3;//F
    BYTE1(remap3[R]) = 2;//R
    BYTE1(remap3[G]) = 1;//G
    BYTE1(remap3[H]) = 4;//H
    BYTE1(remap3[Q]) = 5;//Q
    BYTE1(remap3[P]) = 6;//P
    BYTE1(remap3[CAP_LOCK]) = 3;//CAP_LOCK
    BYTE1(remap3[QUOTE]) = 2;// '/"
    BYTE1(remap3[SEMI_COLON]) = 1;//;/:
    BYTE1(remap3[LEFT_CTRL]) = 0;// CTRL
    BYTE1(remap3[LEFT_ALT]) = 5;// ALT
    BYTE1(remap3[LEFT_SHIFT]) = 7;//Shift
    BYTE1(remap3[ESC]) = 0;// ESC
    BYTE1(remap3[TAB]) = 7;// TAB


    BYTE0(remap1[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE0(remap1[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE0(remap1[JOY_LEFT]) = 0xFF;//1
    BYTE0(remap1[JOY_RIGHT]) = 0xFF;//1
    BYTE0(remap1[JOY_UP]) = 0xFF;//1
    BYTE0(remap1[JOY_DOWN]) = 0xFF;//1

    BYTE1(remap1[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE1(remap1[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE1(remap1[JOY_LEFT]) = 0xFF;//1
    BYTE1(remap1[JOY_RIGHT]) = 0xFF;//1
    BYTE1(remap1[JOY_UP]) = 0xFF;//1
    BYTE1(remap1[JOY_DOWN]) = 0xFF;//1

    BYTE0(remap2[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE0(remap2[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE0(remap2[JOY_LEFT]) = 0xFF;//1
    BYTE0(remap2[JOY_RIGHT]) = 0xFF;//1
    BYTE0(remap2[JOY_UP]) = 0xFF;//1
    BYTE0(remap2[JOY_DOWN]) = 0xFF;//1

    BYTE1(remap2[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE1(remap2[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE1(remap2[JOY_LEFT]) = 0xFF;//1
    BYTE1(remap2[JOY_RIGHT]) = 0xFF;//1
    BYTE1(remap2[JOY_UP]) = 0xFF;//1
    BYTE1(remap2[JOY_DOWN]) = 0xFF;//1

    BYTE0(remap3[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE0(remap3[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE0(remap3[JOY_LEFT]) = 0xFF;//1
    BYTE0(remap3[JOY_RIGHT]) = 0xFF;//1
    BYTE0(remap3[JOY_UP]) = 0xFF;//1
    BYTE0(remap3[JOY_DOWN]) = 0xFF;//1

    BYTE1(remap3[TOUCH_BAR_LEFT]) = 0xFF;//1
    BYTE1(remap3[TOUCH_BAR_RIGHT]) = 0xFF;//1
    BYTE1(remap3[JOY_LEFT]) = 0xFF;//1
    BYTE1(remap3[JOY_RIGHT]) = 0xFF;//1
    BYTE1(remap3[JOY_UP]) = 0xFF;//1
    BYTE1(remap3[JOY_DOWN]) = 0xFF;//1
}

uint8_t _read_joy_adc(void)
{
    RFK.joy_value_x = RFK.joy_value_arr[0];
    RFK.joy_value_y = RFK.joy_value_arr[1];
    RFK.joy_sta = 0;
    if(RFK.joy_value_x > JOY_VALUE_RIGHT)RFK.joy_sta |= 0x08;
    if(RFK.joy_value_x < JOY_VALUE_LEFT)RFK.joy_sta |= 0x04;
    if(RFK.joy_value_y > JOY_VALUE_DOWN)RFK.joy_sta |= 0x02;
    if(RFK.joy_value_y < JOY_VALUE_UP)RFK.joy_sta |= 0x01;
//    uart1_report("get adc:[%d, %d] sta:%d\r\n",
//                 RFK.joy_value_x, RFK.joy_value_y, RFK.joy_sta);
}

uint8_t _touchbar_state(void)
{
    static uint8_t tbs = 0;
    uint8_t touchbarBuffer = RFK.mapBuffer[2] & 0xFE;
    uint8_t ret = 0;
    if(tbs != touchbarBuffer)
    {
        if(tbs == 0 || touchbarBuffer == 0)
        {
            ret = 0;
        }
        else if(tbs > touchbarBuffer && touchbarBuffer < 5)//left->vol down
        {
            ret = 2;
        }
        else if(tbs < touchbarBuffer && touchbarBuffer > 128) //right->vol up
        {
            ret = 1;
        }
        tbs = touchbarBuffer;
    }
    return ret;
}

uint8_t _scan_from_spi(void)
{
    memset(RFK.spiBuffer, 0x00, IO_NUMBER / 8 + 1);
    HAL_GPIO_WritePin(SCAN_PL_GPIO_Port, SCAN_PL_Pin, GPIO_PIN_SET);

    RFK.spiHandle->pRxBuffPtr = (uint8_t*) RFK.spiBuffer;
    RFK.spiHandle->RxXferCount = IO_NUMBER / 8 + 1;
    __HAL_SPI_ENABLE(RFK.spiHandle);
    while (RFK.spiHandle->RxXferCount > 0U)
    {
        if (__HAL_SPI_GET_FLAG(RFK.spiHandle, SPI_FLAG_RXNE))
        {
            /* read the received data */
            (*(uint8_t*) RFK.spiHandle->pRxBuffPtr) = *(__IO uint8_t*) &RFK.spiHandle->Instance->DR;
            RFK.spiHandle->pRxBuffPtr += sizeof(uint8_t);
            RFK.spiHandle->RxXferCount--;
        }
    }
    __HAL_SPI_DISABLE(RFK.spiHandle);

    HAL_GPIO_WritePin(SCAN_PL_GPIO_Port, SCAN_PL_Pin, GPIO_PIN_RESET);

    return 0;
}

uint8_t _scan_debug(void)
{
    memset(RFK.spiBuffer, 0xFF, sizeof(RFK.spiBuffer));
    memset(RFK.spiCeBuffer, 0xFF, sizeof(RFK.spiCeBuffer));

}

uint8_t _recheck_key(void)
{
    _scan_from_spi();
    memcpy(RFK.preSpiBuffer, RFK.spiBuffer, IO_NUMBER / 8 + 1);

    DelayUs(2);
    _scan_from_spi();

    uint8_t mask;
    for (int i = 0; i < IO_NUMBER / 8 + 1; i++)
    {
        mask = RFK.preSpiBuffer[i] ^ RFK.spiBuffer[i];
        RFK.spiBuffer[i] |= mask;
    }

//    uart1_report("get spi:[%d, %d, %d]\r\n",
//                 RFK.spiBuffer[1], RFK.spiBuffer[2], RFK.spiBuffer[3]);
    memset(RFK.mapBuffer, 0xFF, sizeof(RFK.mapBuffer));
    memcpy(RFK.mapBuffer, RFK.spiBuffer + 1, IO_NUMBER / 8);
    RFK.mapBuffer[0] = ~RFK.mapBuffer[0];
    RFK.mapBuffer[1] = ~RFK.mapBuffer[1];
    RFK.mapBuffer[2] = ~RFK.mapBuffer[2];
}

uint8_t _remap_key(void)
{
    static uint8_t tbr_keep_cnt = 2;
    static uint8_t tbl_keep_cnt = 10;
    memset(RFK.hidBuffer, 0x00, sizeof(RFK.hidBuffer));
    _read_joy_adc();

    uint8_t isPressed = 0;
    uint8_t tmp = _touchbar_state();
//    uart1_report("%d\r\n", tmp);
    RFK.hidBuffer[0] = 0x01;
    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            //0 or 1
            if(i*8+j != PAD_NUM_1 && RFK.remapPtr == remap1)
            {
                if (RFK.remapPtr[i * 8 + j] == 0)continue;
            }
            if(i*8+j != F1 && RFK.remapPtr == remap2)
            {
                if (RFK.remapPtr[i * 8 + j] == 0)continue;
            }
            if(i*8+j != A && RFK.remapPtr == remap3)
            {
                if (RFK.remapPtr[i * 8 + j] == 0)continue;
            }
            if(i*8+j == TOUCH_BAR_RIGHT)
            {
                if(tbr_keep_cnt == 10)
                {
                    isPressed = (tmp == 2);
                    if(isPressed) tbr_keep_cnt--;
                }
                else if(tbr_keep_cnt == 0)
                {
                    isPressed = 1;
                    tbr_keep_cnt = 10;
                }
                else
                {
                    isPressed = 1;
                    tbr_keep_cnt--;
                }
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else if(i*8+j == TOUCH_BAR_LEFT)
            {
                if(tbl_keep_cnt == 10)
                {
                    isPressed = (tmp == 1);
                    if(isPressed) tbl_keep_cnt--;
                }
                else if(tbl_keep_cnt == 0)
                {
                    isPressed = 1;
                    tbl_keep_cnt = 10;
                }
                else
                {
                    isPressed = 1;
                    tbl_keep_cnt--;
                }
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else if(i*8+j == JOY_UP)
            {
                isPressed = RFK.joy_sta & 0x01 || RFK.joy_sta == 0x05 || RFK.joy_sta == 0x09;
//                if(isPressed)uart1_report("lick up\r\n");
//                else uart1_report("unlick up %d\r\n", RFK.joy_sta);
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else if(i*8+j == JOY_DOWN)
            {
                isPressed = RFK.joy_sta == 0x02 || RFK.joy_sta == 0x06 || RFK.joy_sta == 0x0A;
//                if(isPressed)uart1_report("lick down\r\n");
//                else uart1_report("unlick down %d\r\n", RFK.joy_sta);
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else if(i*8+j == JOY_LEFT)
            {
                isPressed = RFK.joy_sta == 0x04 || RFK.joy_sta == 0x05 || RFK.joy_sta == 0x06;
//                if(isPressed)uart1_report("lick left\r\n");
//                else uart1_report("unlick left %d\r\n", RFK.joy_sta);
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else if(i*8+j == JOY_RIGHT)
            {
                isPressed = RFK.joy_sta == 0x08 || RFK.joy_sta == 0x09 || RFK.joy_sta == 0x0A;
//                if(isPressed)uart1_report("lick right\r\n");
//                else uart1_report("unlick right %d\r\n", RFK.joy_sta);
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
            else
            {
                isPressed = (RFK.mapBuffer[BYTE0(RFK.remapPtr[i * 8 + j])] & (0x01 << BYTE1(RFK.remapPtr[i * 8 + j])))
                    >> BYTE1(RFK.remapPtr[i * 8 + j]);
                //逐报文检查是否按下
//                if(i*8+j==VOLUME_UP && isPressed)uart1_report("i:%d j:%d lick\r\n", i, j);
                RFK.hidBuffer[i + 1] = RFK.hidBuffer[i + 1] | (isPressed << j);
            }
        }
    }
}

uint8_t _report(void)
{
    RFK.hidBuffer[0] = 0x01;
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,RFK.hidBuffer,18);
}

void RFK_init(void)
{
    RFK.spiHandle = &hspi1;
    RFK.totalKeyNum = IO_NUMBER;
    memset(RFK.spiBuffer, 0xFF, sizeof(RFK.spiBuffer));
    memset(RFK.preSpiBuffer, 0xFF, sizeof(RFK.preSpiBuffer));
    RFK.keyboardSta = 0;
    RFK.freq = 4000;

    _remap_init();

    RFK.scanKeyboard = _recheck_key;
    RFK.buildHidBuffer = _remap_key;
    RFK.reportHid = _report;
    RFK.switch_map = _switch_map;
}
