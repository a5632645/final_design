#include "system.h"
#include "gpio.h"
#include "delay.h"
#include "uart.h"
#include "touchkey.h"
#include "pwm.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ch554.h"
#include "ch554_usb.h"

// --------------------------------------------------------------------------------
// timer
// --------------------------------------------------------------------------------
static void Timer_Init(void);
static void Timer_Interrupt(void) __interrupt(INT_NO_TMR2);
static uint16_t Timer_GetTime(void);
static volatile uint16_t timerTick_ = 0;
#define SPI_TIMEOUT_MS 5

// --------------------------------------------------------------------------------
// Touchkey Slider
// --------------------------------------------------------------------------------
#define TIN0_PIN P11
#define TIN1_PIN P14
#define TIN2_PIN P15
#define TKEY_DOWN_THRESHOLD 2000
#define TKEY_UP_THRESHOLD 500
#define TKEY_POS_SCALE 0x7f
static const __code enum TouchKeyChannel tkeyChannelTable[] = {
    TK_P11, TK_P15, TK_P14
};
#define TKEY_NUM (sizeof(tkeyChannelTable) / sizeof(enum TouchKeyChannel))

static void TouchKey_Init(void);
static uint8_t currentTkey = 0;
static volatile int8_t tkeyFingerPosition = 0;
uint16_t touchkeyFreeValues[TKEY_NUM];
uint16_t absValues[TKEY_NUM];
uint8_t pressFlag = 0;

// --------------------------------------------------------------------------------
// OH49E HALL ADC
// --------------------------------------------------------------------------------
#define ADC_PIN P32
static void HallADC_Init(void);
static volatile uint8_t hallAdcValue = 0;
static uint8_t lowAdcThreadhold = 0;
static uint8_t highAdcThreadhold = 0;

// --------------------------------------------------------------------------------
// Key SPI Commuunication
// --------------------------------------------------------------------------------
#define SPI_SCK_PIN P17
#define SPI_MISO_PIN P16
#define SPI_CS_PIN P33
#define SPI_SLAVE_SCS_PIN P14
#define SPI_WR_PIN P31
#define SPI_IRQ_ENABLE_PIN P34 // 下降沿产生IRQ
// TODO: 焊接所有的MCS和CS_PREV

static void KeyCommunication_Init(void);
static void KeyCommunication_Send(void);
static void KeyCommunication_Receive(void);

inline uint16_t Abs(uint16_t a, uint16_t b) {
    if (a > b) {
        return a - b;
    }
    else {
        return 0;
    }
}

void main(void) {
    CLK_config();
    DLY_ms(5);

    HallADC_Init();
    TouchKey_Init();
    KeyCommunication_Init();
    Timer_Init();
    EA = 1;

    for (;;) {
        if (PIN_read(SPI_CS_PIN) == 0 && currentTkey != 2) {
            if (PIN_read(SPI_WR_PIN) == 0) {
                KeyCommunication_Send();
            }
            else {
                KeyCommunication_Receive();
            }
        }

        if (ADC_START == 0) {
            hallAdcValue = ADC_DATA;
            ADC_START = 1;
        }

        if (TouchKey_Ready() == 1) {
            uint16_t keyData = TKEY_DAT;

            // uint16_t diff = Abs(touchkeyFreeValues[currentTkey], keyData);
            uint16_t diff = 0;
            if (touchkeyFreeValues[currentTkey] > keyData) {
                diff = touchkeyFreeValues[currentTkey] - keyData;
            }
            uint8_t mask = 1 << currentTkey;
            if (diff < TKEY_UP_THRESHOLD) {
                if (pressFlag & mask) {
                    pressFlag &= ~mask;
                }
            }
            else if (diff > TKEY_DOWN_THRESHOLD) {
                if (!(pressFlag & mask)) {
                    pressFlag |= mask;
                }
            }

            absValues[currentTkey] = diff;
            ++currentTkey;
            if (currentTkey >= TKEY_NUM) {
                currentTkey = 0;

                if (pressFlag) {
                    int16_t denominator = (absValues[0] + absValues[1] + absValues[2]) / TKEY_POS_SCALE;
                    if (denominator != 0) {
                        int16_t nominator = absValues[2] - absValues[0];
                        int16_t div = nominator / denominator;
                        if (div < -TKEY_POS_SCALE) tkeyFingerPosition = -TKEY_POS_SCALE;
                        else if (div > TKEY_POS_SCALE) tkeyFingerPosition = TKEY_POS_SCALE;
                        else tkeyFingerPosition = div;
                    }
                }
                else {
                    tkeyFingerPosition = 0;
                }
            }

            TouchKey_Channel(tkeyChannelTable[currentTkey]);
        }
    }
}

#define TIMER2_FREQ (F_CPU / 12)
#define TIMER2_MS 1
#define TIMER2_RELOAD (0xffff - TIMER2_FREQ * TIMER2_MS / 1000 + 1)
void Timer_Init(void) {
    T2MOD &= ~(bTMR_CLK | bT2_CLK); // Fsys/12
    RCAP2 = TIMER2_RELOAD;
    T2COUNT = TIMER2_RELOAD;
    TR2 = 1;
    ET2 = 1;
    TF2 = 0; // !need manual clear
}

void Timer_Interrupt(void) __interrupt(INT_NO_TMR2) {
    TF2 = 0;
    ++timerTick_;
}

uint16_t Timer_GetTime(void) {
    return timerTick_;
}

// ---------------------------------------------------------------------------------
static void TouchKey_Init(void) {
    PIN_input(TIN0_PIN);
    PIN_input(TIN1_PIN);
    PIN_input(TIN2_PIN);
    TouchKey_Fast();
    for (uint8_t i = 0; i < TKEY_NUM; i++) {
        touchkeyFreeValues[i] = TouchKey_PollRead(tkeyChannelTable[i]);
    }
    
    TouchKey_Channel(tkeyChannelTable[0]);
}                

void HallADC_Init(void) {
    PIN_input(ADC_PIN);
    ADC_enable();
    ADC_fast();
    ADC_input(ADC_PIN);
    ADC_START = 1;
}

void KeyCommunication_Init(void) {
    PIN_input(SPI_CS_PIN);
    PIN_input(SPI_WR_PIN);
    PIN_output(SPI_IRQ_ENABLE_PIN);
    PIN_low(SPI_IRQ_ENABLE_PIN);
    IT1 = 1;
    IE1 = 0;
}

void KeyCommunication_Send(void) {
    // SPI设置
    PIN_high(SPI_IRQ_ENABLE_PIN);
    DLY_us(20);
    PIN_output(SPI_SLAVE_SCS_PIN);
    SCS = 1;
    SPI0_SETUP = bS0_MODE_SLV;
    SPI0_CTRL = bS0_2_WIRE | bS0_CLR_ALL | bS0_AUTO_IF | bS0_MISO_OE;
    SPI0_CTRL &= ~bS0_CLR_ALL;
    PIN_output(SPI_MISO_PIN);
    PIN_input(SPI_SCK_PIN);

    SPI0_S_PRE = 0b11011000;
    SCS = 0;
    PIN_low(SPI_IRQ_ENABLE_PIN);// 请求以及CH442E转发送
    while (S0_IF_BYTE == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;
    SPI0_DATA = 0b10101010;
    while (S0_IF_BYTE  == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;
    SPI0_DATA = hallAdcValue;
    while (S0_IF_BYTE  == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;
    SPI0_DATA = tkeyFingerPosition;
    while (S0_IF_BYTE  == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;

    PIN_input(SPI_SLAVE_SCS_PIN);
}

void KeyCommunication_Receive(void) {
    // SPI设置
    PIN_high(SPI_IRQ_ENABLE_PIN);
    DLY_us(20);
    PIN_output(SPI_SLAVE_SCS_PIN);
    SCS = 1;
    SPI0_SETUP = bS0_MODE_SLV;
    SPI0_CTRL = bS0_DATA_DIR | bS0_2_WIRE | bS0_AUTO_IF | bS0_CLR_ALL;
    SPI0_CTRL &= ~bS0_CLR_ALL;
    PIN_input(SPI_MISO_PIN);
    PIN_input(SPI_SCK_PIN);

    SCS = 0;
    PIN_low(SPI_IRQ_ENABLE_PIN);// 请求以及CH442E转发送
    while (S0_IF_BYTE == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;
    highAdcThreadhold = SPI0_DATA;
    while (S0_IF_BYTE  == 0 && PIN_read(SPI_CS_PIN) == 0) {}
    S0_IF_BYTE  = 0;
    lowAdcThreadhold = SPI0_DATA;
    SCS = 1;

    PIN_input(SPI_SLAVE_SCS_PIN);
}
