/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for MOTOR_PWM_LEFT */
#define MOTOR_PWM_LEFT_INST                                                TIMG7
#define MOTOR_PWM_LEFT_INST_IRQHandler                          TIMG7_IRQHandler
#define MOTOR_PWM_LEFT_INST_INT_IRQN                            (TIMG7_INT_IRQn)
#define MOTOR_PWM_LEFT_INST_CLK_FREQ                                    20000000
/* GPIO defines for channel 0 */
#define GPIO_MOTOR_PWM_LEFT_C0_PORT                                        GPIOA
#define GPIO_MOTOR_PWM_LEFT_C0_PIN                                DL_GPIO_PIN_28
#define GPIO_MOTOR_PWM_LEFT_C0_IOMUX                              (IOMUX_PINCM3)
#define GPIO_MOTOR_PWM_LEFT_C0_IOMUX_FUNC              IOMUX_PINCM3_PF_TIMG7_CCP0
#define GPIO_MOTOR_PWM_LEFT_C0_IDX                           DL_TIMER_CC_0_INDEX

/* Defines for MOTOR_PWM_RIGHT */
#define MOTOR_PWM_RIGHT_INST                                              TIMG12
#define MOTOR_PWM_RIGHT_INST_IRQHandler                        TIMG12_IRQHandler
#define MOTOR_PWM_RIGHT_INST_INT_IRQN                          (TIMG12_INT_IRQn)
#define MOTOR_PWM_RIGHT_INST_CLK_FREQ                                   20000000
/* GPIO defines for channel 0 */
#define GPIO_MOTOR_PWM_RIGHT_C0_PORT                                       GPIOB
#define GPIO_MOTOR_PWM_RIGHT_C0_PIN                               DL_GPIO_PIN_20
#define GPIO_MOTOR_PWM_RIGHT_C0_IOMUX                            (IOMUX_PINCM48)
#define GPIO_MOTOR_PWM_RIGHT_C0_IOMUX_FUNC            IOMUX_PINCM48_PF_TIMG12_CCP0
#define GPIO_MOTOR_PWM_RIGHT_C0_IDX                          DL_TIMER_CC_0_INDEX




/* Defines for I2C_0 */
#define I2C_0_INST                                                          I2C1
#define I2C_0_INST_IRQHandler                                    I2C1_IRQHandler
#define I2C_0_INST_INT_IRQN                                        I2C1_INT_IRQn
#define I2C_0_BUS_SPEED_HZ                                                400000
#define GPIO_I2C_0_SDA_PORT                                                GPIOB
#define GPIO_I2C_0_SDA_PIN                                         DL_GPIO_PIN_3
#define GPIO_I2C_0_IOMUX_SDA                                     (IOMUX_PINCM16)
#define GPIO_I2C_0_IOMUX_SDA_FUNC                      IOMUX_PINCM16_PF_I2C1_SDA
#define GPIO_I2C_0_SCL_PORT                                                GPIOB
#define GPIO_I2C_0_SCL_PIN                                         DL_GPIO_PIN_2
#define GPIO_I2C_0_IOMUX_SCL                                     (IOMUX_PINCM15)
#define GPIO_I2C_0_IOMUX_SCL_FUNC                      IOMUX_PINCM15_PF_I2C1_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                            4000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_4_MHZ_115200_BAUD                                        (2)
#define UART_0_FBRD_4_MHZ_115200_BAUD                                       (11)
/* Defines for UART_1 */
#define UART_1_INST                                                        UART1
#define UART_1_INST_FREQUENCY                                           40000000
#define UART_1_INST_IRQHandler                                  UART1_IRQHandler
#define UART_1_INST_INT_IRQN                                      UART1_INT_IRQn
#define GPIO_UART_1_RX_PORT                                                GPIOA
#define GPIO_UART_1_TX_PORT                                                GPIOA
#define GPIO_UART_1_RX_PIN                                         DL_GPIO_PIN_9
#define GPIO_UART_1_TX_PIN                                         DL_GPIO_PIN_8
#define GPIO_UART_1_IOMUX_RX                                     (IOMUX_PINCM20)
#define GPIO_UART_1_IOMUX_TX                                     (IOMUX_PINCM19)
#define GPIO_UART_1_IOMUX_RX_FUNC                      IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART_1_IOMUX_TX_FUNC                      IOMUX_PINCM19_PF_UART1_TX
#define UART_1_BAUD_RATE                                                (115200)
#define UART_1_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_1_FBRD_40_MHZ_115200_BAUD                                      (45)
/* Defines for UART_2 */
#define UART_2_INST                                                        UART2
#define UART_2_INST_FREQUENCY                                           40000000
#define UART_2_INST_IRQHandler                                  UART2_IRQHandler
#define UART_2_INST_INT_IRQN                                      UART2_INT_IRQn
#define GPIO_UART_2_RX_PORT                                                GPIOB
#define GPIO_UART_2_TX_PORT                                                GPIOB
#define GPIO_UART_2_RX_PIN                                        DL_GPIO_PIN_16
#define GPIO_UART_2_TX_PIN                                        DL_GPIO_PIN_15
#define GPIO_UART_2_IOMUX_RX                                     (IOMUX_PINCM33)
#define GPIO_UART_2_IOMUX_TX                                     (IOMUX_PINCM32)
#define GPIO_UART_2_IOMUX_RX_FUNC                      IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_2_IOMUX_TX_FUNC                      IOMUX_PINCM32_PF_UART2_TX
#define UART_2_BAUD_RATE                                                (115200)
#define UART_2_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_2_FBRD_40_MHZ_115200_BAUD                                      (45)
/* Defines for UART_3 */
#define UART_3_INST                                                        UART3
#define UART_3_INST_FREQUENCY                                           40000000
#define UART_3_INST_IRQHandler                                  UART3_IRQHandler
#define UART_3_INST_INT_IRQN                                      UART3_INT_IRQn
#define GPIO_UART_3_RX_PORT                                                GPIOB
#define GPIO_UART_3_TX_PORT                                                GPIOA
#define GPIO_UART_3_RX_PIN                                        DL_GPIO_PIN_13
#define GPIO_UART_3_TX_PIN                                        DL_GPIO_PIN_26
#define GPIO_UART_3_IOMUX_RX                                     (IOMUX_PINCM30)
#define GPIO_UART_3_IOMUX_TX                                     (IOMUX_PINCM59)
#define GPIO_UART_3_IOMUX_RX_FUNC                      IOMUX_PINCM30_PF_UART3_RX
#define GPIO_UART_3_IOMUX_TX_FUNC                      IOMUX_PINCM59_PF_UART3_TX
#define UART_3_BAUD_RATE                                                (115200)
#define UART_3_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_3_FBRD_40_MHZ_115200_BAUD                                      (45)





/* Defines for ADC12_0 */
#define ADC12_0_INST                                                        ADC0
#define ADC12_0_INST_IRQHandler                                  ADC0_IRQHandler
#define ADC12_0_INST_INT_IRQN                                    (ADC0_INT_IRQn)
#define ADC12_0_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC12_0_ADCMEM_0_REF                     DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC12_0_ADCMEM_0_REF_VOLTAGE_V                                       3.3
#define GPIO_ADC12_0_C0_PORT                                               GPIOA
#define GPIO_ADC12_0_C0_PIN                                       DL_GPIO_PIN_27
#define GPIO_ADC12_0_IOMUX_C0                                    (IOMUX_PINCM60)
#define GPIO_ADC12_0_IOMUX_C0_FUNC                (IOMUX_PINCM60_PF_UNCONNECTED)



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define UART_1_INST_DMA_TRIGGER                              (DMA_UART1_RX_TRIG)
/* Defines for DMA_CH1 */
#define DMA_CH1_CHAN_ID                                                      (1)
#define UART_2_INST_DMA_TRIGGER                              (DMA_UART2_RX_TRIG)
/* Defines for DMA_CH2 */
#define DMA_CH2_CHAN_ID                                                      (2)
#define UART_3_INST_DMA_TRIGGER                              (DMA_UART3_RX_TRIG)


/* Port definition for Pin Group MOTOR_DIR_LEFT1 */
#define MOTOR_DIR_LEFT1_PORT                                             (GPIOA)

/* Defines for PIN_0: GPIOA.13 with pinCMx 35 on package pin 6 */
#define MOTOR_DIR_LEFT1_PIN_0_PIN                               (DL_GPIO_PIN_13)
#define MOTOR_DIR_LEFT1_PIN_0_IOMUX                              (IOMUX_PINCM35)
/* Port definition for Pin Group MOTOR_DIR_LEFT2 */
#define MOTOR_DIR_LEFT2_PORT                                             (GPIOB)

/* Defines for PIN_1: GPIOB.26 with pinCMx 57 on package pin 28 */
#define MOTOR_DIR_LEFT2_PIN_1_PIN                               (DL_GPIO_PIN_26)
#define MOTOR_DIR_LEFT2_PIN_1_IOMUX                              (IOMUX_PINCM57)
/* Port definition for Pin Group MOTOR_DIR_RIGHT1 */
#define MOTOR_DIR_RIGHT1_PORT                                            (GPIOB)

/* Defines for PIN_2: GPIOB.9 with pinCMx 26 on package pin 61 */
#define MOTOR_DIR_RIGHT1_PIN_2_PIN                               (DL_GPIO_PIN_9)
#define MOTOR_DIR_RIGHT1_PIN_2_IOMUX                             (IOMUX_PINCM26)
/* Port definition for Pin Group MOTOR_DIR_RIGHT2 */
#define MOTOR_DIR_RIGHT2_PORT                                            (GPIOB)

/* Defines for PIN_3: GPIOB.7 with pinCMx 24 on package pin 59 */
#define MOTOR_DIR_RIGHT2_PIN_3_PIN                               (DL_GPIO_PIN_7)
#define MOTOR_DIR_RIGHT2_PIN_3_IOMUX                             (IOMUX_PINCM24)
/* Port definition for Pin Group O1 */
#define O1_PORT                                                          (GPIOA)

/* Defines for PIN_4: GPIOA.31 with pinCMx 6 on package pin 39 */
#define O1_PIN_4_PIN                                            (DL_GPIO_PIN_31)
#define O1_PIN_4_IOMUX                                            (IOMUX_PINCM6)
/* Port definition for Pin Group O2 */
#define O2_PORT                                                          (GPIOA)

/* Defines for PIN_6: GPIOA.12 with pinCMx 34 on package pin 5 */
#define O2_PIN_6_PIN                                            (DL_GPIO_PIN_12)
#define O2_PIN_6_IOMUX                                           (IOMUX_PINCM34)
/* Port definition for Pin Group O3 */
#define O3_PORT                                                          (GPIOB)

/* Defines for PIN_7: GPIOB.8 with pinCMx 25 on package pin 60 */
#define O3_PIN_7_PIN                                             (DL_GPIO_PIN_8)
#define O3_PIN_7_IOMUX                                           (IOMUX_PINCM25)
/* Port definition for Pin Group O4 */
#define O4_PORT                                                          (GPIOB)

/* Defines for PIN_8: GPIOB.27 with pinCMx 58 on package pin 29 */
#define O4_PIN_8_PIN                                            (DL_GPIO_PIN_27)
#define O4_PIN_8_IOMUX                                           (IOMUX_PINCM58)
/* Port definition for Pin Group ENCODER */
#define ENCODER_PORT                                                     (GPIOB)

/* Defines for left_a: GPIOB.23 with pinCMx 51 on package pin 22 */
/* pins affected by this interrupt request:["left_a","left_b","right_a","right_b"] */
#define ENCODER_INT_IRQN                                        (GPIOB_INT_IRQn)
#define ENCODER_INT_IIDX                        (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_left_a_IIDX                                 (DL_GPIO_IIDX_DIO23)
#define ENCODER_left_a_PIN                                      (DL_GPIO_PIN_23)
#define ENCODER_left_a_IOMUX                                     (IOMUX_PINCM51)
/* Defines for left_b: GPIOB.12 with pinCMx 29 on package pin 64 */
#define ENCODER_left_b_IIDX                                 (DL_GPIO_IIDX_DIO12)
#define ENCODER_left_b_PIN                                      (DL_GPIO_PIN_12)
#define ENCODER_left_b_IOMUX                                     (IOMUX_PINCM29)
/* Defines for right_a: GPIOB.4 with pinCMx 17 on package pin 52 */
#define ENCODER_right_a_IIDX                                 (DL_GPIO_IIDX_DIO4)
#define ENCODER_right_a_PIN                                      (DL_GPIO_PIN_4)
#define ENCODER_right_a_IOMUX                                    (IOMUX_PINCM17)
/* Defines for right_b: GPIOB.5 with pinCMx 18 on package pin 53 */
#define ENCODER_right_b_IIDX                                 (DL_GPIO_IIDX_DIO5)
#define ENCODER_right_b_PIN                                      (DL_GPIO_PIN_5)
#define ENCODER_right_b_IOMUX                                    (IOMUX_PINCM18)
/* Port definition for Pin Group GRAY */
#define GRAY_PORT                                                        (GPIOB)

/* Defines for SERIAL_DAT: GPIOB.10 with pinCMx 27 on package pin 62 */
#define GRAY_SERIAL_DAT_PIN                                     (DL_GPIO_PIN_10)
#define GRAY_SERIAL_DAT_IOMUX                                    (IOMUX_PINCM27)
/* Defines for SERIAL_CLK: GPIOB.11 with pinCMx 28 on package pin 63 */
#define GRAY_SERIAL_CLK_PIN                                     (DL_GPIO_PIN_11)
#define GRAY_SERIAL_CLK_IOMUX                                    (IOMUX_PINCM28)
/* Port definition for Pin Group IMU */
#define IMU_PORT                                                         (GPIOA)

/* Defines for IMU_SDA: GPIOA.0 with pinCMx 1 on package pin 33 */
#define IMU_IMU_SDA_PIN                                          (DL_GPIO_PIN_0)
#define IMU_IMU_SDA_IOMUX                                         (IOMUX_PINCM1)
/* Defines for IMU_SCL: GPIOA.1 with pinCMx 2 on package pin 34 */
#define IMU_IMU_SCL_PIN                                          (DL_GPIO_PIN_1)
#define IMU_IMU_SCL_IOMUX                                         (IOMUX_PINCM2)
/* Defines for KEY_MY_UP: GPIOA.17 with pinCMx 39 on package pin 10 */
#define KEY_KEY_MY_UP_PORT                                               (GPIOA)
#define KEY_KEY_MY_UP_PIN                                       (DL_GPIO_PIN_17)
#define KEY_KEY_MY_UP_IOMUX                                      (IOMUX_PINCM39)
/* Defines for KEY_MY_DOWN: GPIOB.24 with pinCMx 52 on package pin 23 */
#define KEY_KEY_MY_DOWN_PORT                                             (GPIOB)
#define KEY_KEY_MY_DOWN_PIN                                     (DL_GPIO_PIN_24)
#define KEY_KEY_MY_DOWN_IOMUX                                    (IOMUX_PINCM52)
/* Defines for KEY_LEFT: GPIOA.15 with pinCMx 37 on package pin 8 */
#define KEY_KEY_LEFT_PORT                                                (GPIOA)
#define KEY_KEY_LEFT_PIN                                        (DL_GPIO_PIN_15)
#define KEY_KEY_LEFT_IOMUX                                       (IOMUX_PINCM37)
/* Defines for KEY_RIGHT: GPIOA.14 with pinCMx 36 on package pin 7 */
#define KEY_KEY_RIGHT_PORT                                               (GPIOA)
#define KEY_KEY_RIGHT_PIN                                       (DL_GPIO_PIN_14)
#define KEY_KEY_RIGHT_IOMUX                                      (IOMUX_PINCM36)
/* Defines for KEY_CENTER: GPIOB.25 with pinCMx 56 on package pin 27 */
#define KEY_KEY_CENTER_PORT                                              (GPIOB)
#define KEY_KEY_CENTER_PIN                                      (DL_GPIO_PIN_25)
#define KEY_KEY_CENTER_IOMUX                                     (IOMUX_PINCM56)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_MOTOR_PWM_LEFT_init(void);
void SYSCFG_DL_MOTOR_PWM_RIGHT_init(void);
void SYSCFG_DL_I2C_0_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_1_init(void);
void SYSCFG_DL_UART_2_init(void);
void SYSCFG_DL_UART_3_init(void);
void SYSCFG_DL_ADC12_0_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
