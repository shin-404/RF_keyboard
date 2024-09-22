//
// Created by Shin on 2024/4/19.
//

#ifndef MAIN_C_RF_KEYBOARD_H
#define MAIN_C_RF_KEYBOARD_H

#include "spi.h"

#define IO_NUMBER 24


typedef struct {
    SPI_HandleTypeDef *spiHandle;
    uint16_t joy_value_arr[200];
    uint16_t joy_value_x;
    uint16_t joy_value_y;
    uint8_t joy_sta;//0:none 1-up 2-down 4-left 8-right

    uint8_t totalKeyNum;      //8*3 pcs 74hc165
    uint8_t keyboardSta;       //check the level of map
    uint8_t spiBuffer[IO_NUMBER / 8 + 1];
    uint8_t preSpiBuffer[IO_NUMBER / 8 + 1];
    uint8_t spiCeBuffer[IO_NUMBER / 8 + 1];
    uint8_t mapBuffer[IO_NUMBER / 8 + 1];
    uint8_t hidBuffer[18];
    uint16_t *remapPtr;
    uint32_t freq;

    uint8_t (*scanKeyboard)(void); //scan function
    uint8_t (*buildHidBuffer)(void);
    uint8_t (*reportHid)(void);
    uint8_t (*switch_map)(uint8_t);
} _RF_Keyboard_st;

extern _RF_Keyboard_st RFK;

void RFK_init(void);

#endif //MAIN_C_RF_KEYBOARD_H
