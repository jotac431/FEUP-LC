#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#define KB_IRQ 1
#define DELAY_US        20000

// KBC Ports
#define OUT_BUF  0x60
#define IN_BUF   0x60
#define STAT_REG 0x64
#define KBC_CMD_REG 0x64
#define KBC_CMD_ARG 0x60 

// Status Register
#define STAT_REG_PAR BIT(7)
#define STAT_REG_TIM BIT(6)
#define STAT_REG_AUX BIT(5)
#define STAT_REG_INH BIT(4)
#define STAT_REG_A2  BIT(3)
#define STAT_REG_SYS BIT(2)
#define STAT_REG_IBF BIT(1)
#define STAT_REG_OBF BIT(0)

// KBC commands
#define KBC_READ_COM        0x20
#define KBC_WRITE_COM       0x60
#define KBC_CHECK           0xAA
#define KBC_CHECK_INTERFACE 0xAB
#define KBD_DIS             0xAD
#define KBD_EN              0xAE

// Scancodes
#define ESQ_BREAKCODE       0x81
#define TWO_BYTE_SCANCODE   0xE0
#define W_MAKECODE          0x11
#define W_BREAKCODE         0X91
#define S_MAKECODE          0X1F
#define S_BREAKCODE         0X9F
#define UPARROW_MAKECODE    0X48
#define UPARROW_BREAKCODE   0XC8
#define DOWNARROW_MAKECODE  0X50
#define DOWNARROW_BREAKCODE 0XD0


// KBC command byte
#define KBC_DIS2 BIT(5)
#define KBC_DIS BIT(4)
#define KBC_INT2 BIT(1)
#define KBC_INT BIT(0)


#endif /* _LCOM_I8042_H */
