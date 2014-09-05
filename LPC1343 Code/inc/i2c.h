/*****************************************************************************
 *   i2c.h:  Header file for NXP LPC13xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.19  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/
#ifndef __I2C_H 
#define __I2C_H

#define FAST_MODE_PLUS	0

#define BUFSIZE			6
#define MAX_TIMEOUT		0x00FFFFFF

#define I2CMASTER		0x01
#define I2CSLAVE		0x02

#define PCF8594_ADDR	0xA0
#define READ_WRITE		0x01

#define RD_BIT			0x01

#define I2C_IDLE			0
#define I2C_STARTED			1
#define I2C_RESTARTED		2
#define I2C_REPEATED_START	3
#define DATA_ACK			4
#define DATA_NACK			5

#define I2CONSET_I2EN		0x00000040  /* I2C Control Set Register */
#define I2CONSET_AA			0x00000004
#define I2CONSET_SI			0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004  /* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040

#define I2DAT_I2C			0x00000000  /* I2C Data Reg */
#define I2ADR_I2C			0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH			0x00000180  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL			0x00000180  /* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000020  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000020  /* Fast Plus I2C SCL Duty Cycle Low Reg */


extern void I2C_IRQHandler( void );
extern uint32_t I2CInit( uint32_t I2cMode );
extern uint32_t I2CStart( void );
extern uint32_t I2CStop( void );
extern uint32_t I2CEngine( void );

typedef struct
{
  volatile uint32_t PIO2_6;
       uint32_t RESERVED0[1];
  volatile uint32_t PIO2_0;
  volatile uint32_t RESET_PIO0_0;
  volatile uint32_t PIO0_1;
  volatile uint32_t PIO1_8;
       uint32_t RESERVED1[1];
  volatile uint32_t PIO0_2;

  volatile uint32_t PIO2_7;
  volatile uint32_t PIO2_8;
  volatile uint32_t PIO2_1;
  volatile uint32_t PIO0_3;
  volatile uint32_t PIO0_4;
  volatile uint32_t PIO0_5;
  volatile uint32_t PIO1_9;
  volatile uint32_t PIO3_4;

  volatile uint32_t PIO2_4;
  volatile uint32_t PIO2_5;
  volatile uint32_t PIO3_5;
  volatile uint32_t PIO0_6;
  volatile uint32_t PIO0_7;
  volatile uint32_t PIO2_9;
  volatile uint32_t PIO2_10;
  volatile uint32_t PIO2_2;

  volatile uint32_t PIO0_8;
  volatile uint32_t PIO0_9;
  union {
	  // SWCLK_PIO0_10 preferred name rather than original JTAG_TCK_PIO0_10
	  volatile uint32_t SWCLK_PIO0_10;
	  volatile uint32_t JTAG_TCK_PIO0_10;
  };
  volatile uint32_t PIO1_10;
  volatile uint32_t PIO2_11;
  union {
	  // R_PIO0_11 preferred name rather than original JTAG_TDI_PIO0_11
	  volatile uint32_t R_PIO0_11;
	  volatile uint32_t JTAG_TDI_PIO0_11;
  };
  union {
	  // R_PIO1_0 preferred name rather than original JTAG_TMS_PIO1_0
	  volatile uint32_t R_PIO1_0;
	  volatile uint32_t JTAG_TMS_PIO1_0;
  };
  union {
	  // R_PIO1_1 preferred name rather than original JTAG_TDO_PIO1_1
	  volatile uint32_t R_PIO1_1;
	  volatile uint32_t JTAG_TDO_PIO1_1;
  };

  union {
	  // R_PIO1_2 preferred name rather than original JTAG_nTRST_PIO1_2
	  volatile uint32_t R_PIO1_2;
	  volatile uint32_t JTAG_nTRST_PIO1_2;
  };
  volatile uint32_t PIO3_0;
  volatile uint32_t PIO3_1;
  volatile uint32_t PIO2_3;
  union {
	  // SWDIO_PIO1_3 preferred name rather than original ARM_SWDIO_PIO1_3
	  volatile uint32_t SWDIO_PIO1_3;
	  volatile uint32_t ARM_SWDIO_PIO1_3;
  };

  volatile uint32_t PIO1_4;
  volatile uint32_t PIO1_11;
  volatile uint32_t PIO3_2;

  volatile uint32_t PIO1_5;
  volatile uint32_t PIO1_6;
  volatile uint32_t PIO1_7;
  volatile uint32_t PIO3_3;
  union {
	  // SCK_LOC preferred name rather than original SCKLOC
	  volatile uint32_t SCK_LOC;                /*!< Offset: 0x0B0 SCK pin location select Register (R/W) */
	  volatile uint32_t SCKLOC;
  };
  volatile uint32_t DSR_LOC;                /*!< Offset: 0x0B4 DSR pin location select Register (R/W) */
  volatile uint32_t DCD_LOC;                /*!< Offset: 0x0B8 DCD pin location select Register (R/W) */
  volatile uint32_t RI_LOC;                 /*!< Offset: 0x0BC RI pin location Register (R/W) */
} LPC_IOCON_TypeDef_OLD;

#endif /* end __I2C_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
