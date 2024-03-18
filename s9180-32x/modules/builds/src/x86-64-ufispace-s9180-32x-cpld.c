/*
 * This driver is for i2c interface cpld driver for the ufispace_s9180_32x platform,
 * which is the MB cpld.
 *
 * Copyright (C) 2024 UfiSpace Technology Corporation.
 * Melo Lin <melo.lin@ufispace.com>
 *
 * Based on ad7414.c
 * Copyright 2006 Stefan Roese <sr at denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dmi.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include "x86-64-ufispace-s9180-32x-cpld.h"

#if !defined(SENSOR_DEVICE_ATTR_RO)
#define SENSOR_DEVICE_ATTR_RO(_name, _func, _index)		\
	SENSOR_DEVICE_ATTR(_name, 0444, _func##_show, NULL, _index)
#endif

#if !defined(SENSOR_DEVICE_ATTR_RW)
#define SENSOR_DEVICE_ATTR_RW(_name, _func, _index)		\
	SENSOR_DEVICE_ATTR(_name, 0644, _func##_show, _func##_store, _index)

#endif

#if !defined(SENSOR_DEVICE_ATTR_WO)
#define SENSOR_DEVICE_ATTR_WO(_name, _func, _index)		\
	SENSOR_DEVICE_ATTR(_name, 0200, NULL, _func##_store, _index)
#endif


#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) \
    printk(KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#define BSP_LOG_R(fmt, args...) \
    _bsp_log (LOG_READ, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)
#define BSP_LOG_W(fmt, args...) \
    _bsp_log (LOG_WRITE, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)

#define I2C_READ_BYTE_DATA(ret, lock, i2c_client, reg) \
{ \
    mutex_lock(lock); \
    ret = i2c_smbus_read_byte_data(i2c_client, reg); \
    mutex_unlock(lock); \
    BSP_LOG_R("cpld[%d], reg=0x%03x, reg_val=0x%02x", data->index, reg, ret); \
}

#define I2C_WRITE_BYTE_DATA(ret, lock, i2c_client, reg, val) \
{ \
    mutex_lock(lock); \
    ret = i2c_smbus_write_byte_data(i2c_client, reg, val); \
    mutex_unlock(lock); \
    BSP_LOG_W("cpld[%d], reg=0x%03x, reg_val=0x%02x", data->index, reg, val); \
}

#define _DEVICE_ATTR(_name)     \
    &sensor_dev_attr_##_name.dev_attr.attr


/* CPLD sysfs attributes index  */
enum cpld_sysfs_attributes {
     /* MB CPLD - Group 1. Gerneral Control */
    // General Control for Offset 0x00 Board Type and Revision
    CPLD_TYPE_REV_RAW,
    CPLD_BUILD_REV,
    CPLD_HW_REV,
    CPLD_MODEL_ID,
    // General Control for Offset 0x07 Extend Board Type
    CPLD_BOARD_ID,
    // General Control for Offset 0x01 CPLD Revision Register
    CPLD_CPLD_REV,
    CPLD_RELEASE_BIT,
    MB_CPLD_FW_VERSION,
    // General Control for Offset 0x41 Buffer Enable
    CPLD_ENABLE_RAW,
    CPLD_U5239_ENABLE,
    CPLD_U5240_ENABLE,
    // General Control for Offset 0x42 ROV Status
    CPLD_ROV_RAW,
    CPLD_PINJ5_ROV0,
    CPLD_PINN4_ROV1,
    CPLD_PINK5_ROV2,
    CPLD_PINN5_SYNC,
    CPLD_PINN6_TAP_L,
    // General Control for Offset 0x43 Download Select 
    CPLD_DOWNLOAD_SEL,
    // General Control for Offset 0x45 CPLD LED 
    CPLD_LED_RAW,
    CPLD_CPLD1_LED_CTRL,
    CPLD_LED_CLEAR,

    /* MB CPLD - Group 2. LED Status */ 
    // LED Status for Offset 0x80 Port QSFP LED0~LED3 Status
    CPLD_QSFP_LED0_TO_LED3_COLOR,
    CPLD_QSFP_LED0_COLOR,
    CPLD_QSFP_LED1_COLOR,
    CPLD_QSFP_LED2_COLOR,
    CPLD_QSFP_LED3_COLOR,

    // LED Status for Offset 0x81 Port QSFP LED4~LED7 Status
    CPLD_QSFP_LED4_TO_LED7_COLOR,
    CPLD_QSFP_LED4_COLOR,
    CPLD_QSFP_LED5_COLOR,
    CPLD_QSFP_LED6_COLOR,
    CPLD_QSFP_LED7_COLOR,

    // LED Status for Offset 0x82 Port QSFP LED8~LED11 Status 
    CPLD_QSFP_LED8_TO_LED11_COLOR,
    CPLD_QSFP_LED8_COLOR,
    CPLD_QSFP_LED9_COLOR,
    CPLD_QSFP_LED10_COLOR,
    CPLD_QSFP_LED11_COLOR,

    // LED Status for Offset 0x83 Port QSFP LED12~LED15 Status
    CPLD_QSFP_LED12_TO_LED15_COLOR,
    CPLD_QSFP_LED12_COLOR,
    CPLD_QSFP_LED13_COLOR,
    CPLD_QSFP_LED14_COLOR,
    CPLD_QSFP_LED15_COLOR,

    // LED Status for Offset 0x84 Port QSFP LED16~LED19 Status
    CPLD_QSFP_LED16_TO_LED19_COLOR,
    CPLD_QSFP_LED16_COLOR,
    CPLD_QSFP_LED17_COLOR,
    CPLD_QSFP_LED18_COLOR,
    CPLD_QSFP_LED19_COLOR,

    // LED Status for Offset 0x85 Port QSFP LED20~LED23 Status
    CPLD_QSFP_LED20_TO_LED23_COLOR,
    CPLD_QSFP_LED20_COLOR,
    CPLD_QSFP_LED21_COLOR,
    CPLD_QSFP_LED22_COLOR,
    CPLD_QSFP_LED23_COLOR,

    // LED Status for Offset 0x86 Port QSFP LED24~LED27 Status
    CPLD_QSFP_LED24_TO_LED27_COLOR,
    CPLD_QSFP_LED24_COLOR,
    CPLD_QSFP_LED25_COLOR,
    CPLD_QSFP_LED26_COLOR,
    CPLD_QSFP_LED27_COLOR,

    // LED Status for Offset 0x87 Port QSFP LED28~LED31 Status
    CPLD_QSFP_LED28_TO_LED31_COLOR,
    CPLD_QSFP_LED28_COLOR,
    CPLD_QSFP_LED29_COLOR,
    CPLD_QSFP_LED30_COLOR,
    CPLD_QSFP_LED31_COLOR,

    // LED Status for Offset 0x88 Port SFP+ LED Status
    CPLD_SFP0_LED,
    CPLD_SFP1_LED,

    // LED Status for Offset 0x90 Port QSFP LED0~LED7 Blinking control
    CPLD_QSFP_BLINKING_PORT0_7,

    // LED Status for Offset 0x91 Port QSFP LED8~LED15 Blinking control
    CPLD_QSFP_BLINKING_PORT8_15,

    // LED Status for Offset 0x92 Port QSFP LED16~LED23 Blinking control 
    CPLD_QSFP_BLINKING_PORT16_23,

    // LED Status for Offset 0x93 Port QSFP LED24~LED31 Blinking control 
    CPLD_QSFP_BLINKING_PORT24_31,

    // LED Status for Offset 0x94 Port SFP+ LED Blinking control
    CPLD_SFP_BLINKING_SFP0,
    CPLD_SFP_BLINKING_SFP1,
    
    //BSP DEBUG
    BSP_DEBUG
};

enum data_type {
    DATA_HEX,
    DATA_DEC,
    DATA_UNK,
};

typedef struct  {
    u8 reg;
    u8 mask;
    u8 data_type;
} attr_reg_map_t;

static attr_reg_map_t attr_reg[]= {

    /* MB CPLD - Group 1. Gerneral Control */
    // General Control for Offset 0x00 Board Type and Revision
    [CPLD_TYPE_REV_RAW]             =   {CPLD_REG_00,   MASK_ALL,     DATA_DEC},
    [CPLD_BUILD_REV]                =   {CPLD_REG_00,   MASK_BIT1_0,  DATA_HEX},
    [CPLD_HW_REV]                   =   {CPLD_REG_00,   MASK_BIT3_2,  DATA_HEX},
    [CPLD_MODEL_ID]                 =   {CPLD_REG_00,   MASK_BIT7_4,  DATA_HEX},
    // General Control for Offset 0x07 Extend Board Type
    [CPLD_BOARD_ID]                 =   {CPLD_REG_07,   MASK_BIT3_0,  DATA_HEX},
    // General Control for Offset 0x01 CPLD Revision Register
    [CPLD_CPLD_REV]                 =   {CPLD_REG_01,   MASK_BIT5_0,  DATA_DEC},  //minor
    [CPLD_RELEASE_BIT]              =   {CPLD_REG_01,   MASK_BIT6,    DATA_HEX},  //major
    [MB_CPLD_FW_VERSION]            =   {CPLD_REG_NONE, MASK_NONE,    DATA_HEX},
    // General Control for Offset 0x41 Buffer Enable
    [CPLD_ENABLE_RAW]               =   {CPLD_REG_41,   MASK_BIT1_0,  DATA_HEX},
    [CPLD_U5239_ENABLE]             =   {CPLD_REG_41,   MASK_BIT0,    DATA_HEX},
    [CPLD_U5240_ENABLE]             =   {CPLD_REG_41,   MASK_BIT1,    DATA_HEX},
    // General Control for Offset 0x42 ROV Status
    [CPLD_ROV_RAW]                  =   {CPLD_REG_42,   MASK_BIT5_0,  DATA_HEX},
    [CPLD_PINJ5_ROV0]               =   {CPLD_REG_42,   MASK_BIT0,    DATA_HEX},
    [CPLD_PINN4_ROV1]               =   {CPLD_REG_42,   MASK_BIT1,    DATA_HEX},
    [CPLD_PINK5_ROV2]               =   {CPLD_REG_42,   MASK_BIT2,    DATA_HEX},
    [CPLD_PINN5_SYNC]               =   {CPLD_REG_42,   MASK_BIT4,    DATA_HEX},
    [CPLD_PINN6_TAP_L]              =   {CPLD_REG_42,   MASK_BIT5,    DATA_HEX},
    // General Control for Offset 0x43 Download Select 
    [CPLD_DOWNLOAD_SEL]             =   {CPLD_REG_43,   MASK_BIT0,    DATA_HEX},
    // General Control for Offset 0x45 CPLD LED
    [CPLD_LED_RAW]                  =   {CPLD_REG_45,   MASK_BIT1_0,  DATA_HEX},
    [CPLD_CPLD1_LED_CTRL]           =   {CPLD_REG_45,   MASK_BIT0,    DATA_HEX},
    [CPLD_LED_CLEAR]                =   {CPLD_REG_45,   MASK_BIT1,    DATA_HEX},

    /* MB CPLD - Group 2. LED Status */ 
    // LED Status for Offset 0x80 Port QSFP LED0~LED3 Status
    [CPLD_QSFP_LED0_TO_LED3_COLOR]    =   {CPLD_REG_80,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED0_COLOR]            =   {CPLD_REG_80,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED1_COLOR]            =   {CPLD_REG_80,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED2_COLOR]            =   {CPLD_REG_80,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED3_COLOR]            =   {CPLD_REG_80,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x81 Port QSFP LED4~LED7 Status
    [CPLD_QSFP_LED4_TO_LED7_COLOR]    =   {CPLD_REG_81,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED4_COLOR]            =   {CPLD_REG_81,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED5_COLOR]            =   {CPLD_REG_81,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED6_COLOR]            =   {CPLD_REG_81,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED7_COLOR]            =   {CPLD_REG_81,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x82 Port QSFP LED8~LED11 Status 
    [CPLD_QSFP_LED8_TO_LED11_COLOR]   =   {CPLD_REG_82,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED8_COLOR]            =   {CPLD_REG_82,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED9_COLOR]            =   {CPLD_REG_82,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED10_COLOR]           =  {CPLD_REG_82,  MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED11_COLOR]           =  {CPLD_REG_82,  MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x83 Port QSFP LED12~LED15 Status
    [CPLD_QSFP_LED12_TO_LED15_COLOR]  =   {CPLD_REG_83,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED12_COLOR]           =   {CPLD_REG_83,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED13_COLOR]           =   {CPLD_REG_83,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED14_COLOR]           =   {CPLD_REG_83,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED15_COLOR]           =   {CPLD_REG_83,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x84 Port QSFP LED16~LED19 Status
    [CPLD_QSFP_LED16_TO_LED19_COLOR]  =   {CPLD_REG_84,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED16_COLOR]           =   {CPLD_REG_84,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED17_COLOR]           =   {CPLD_REG_84,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED18_COLOR]           =   {CPLD_REG_84,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED19_COLOR]           =   {CPLD_REG_84,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x85 Port QSFP LED20~LED23 Status
    [CPLD_QSFP_LED20_TO_LED23_COLOR]  =   {CPLD_REG_85,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED20_COLOR]           =   {CPLD_REG_85,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED21_COLOR]           =   {CPLD_REG_85,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED22_COLOR]           =   {CPLD_REG_85,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED23_COLOR]           =   {CPLD_REG_85,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x86 Port QSFP LED24~LED27 Status
    [CPLD_QSFP_LED24_TO_LED27_COLOR]  =   {CPLD_REG_86,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED24_COLOR]           =   {CPLD_REG_86,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED25_COLOR]           =   {CPLD_REG_86,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED26_COLOR]           =   {CPLD_REG_86,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED27_COLOR]           =   {CPLD_REG_86,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x87 Port QSFP LED28~LED31 Status
    [CPLD_QSFP_LED28_TO_LED31_COLOR]  =   {CPLD_REG_87,   MASK_ALL,    DATA_HEX},
    [CPLD_QSFP_LED28_COLOR]           =   {CPLD_REG_87,   MASK_BIT1_0,    DATA_HEX},
    [CPLD_QSFP_LED29_COLOR]           =   {CPLD_REG_87,   MASK_BIT3_2,    DATA_HEX},
    [CPLD_QSFP_LED30_COLOR]           =   {CPLD_REG_87,   MASK_BIT5_4,    DATA_HEX},
    [CPLD_QSFP_LED31_COLOR]           =   {CPLD_REG_87,   MASK_BIT7_6,    DATA_HEX},

    // LED Status for Offset 0x88 Port SFP+ LED Status
    [CPLD_SFP0_LED]                   =   {CPLD_REG_88,   MASK_BIT0,    DATA_HEX},
    [CPLD_SFP1_LED]                   =   {CPLD_REG_88,   MASK_BIT1,    DATA_HEX},

    // LED Status for Offset 0x90 Port QSFP LED0~LED7 Blinking control
    [CPLD_QSFP_BLINKING_PORT0_7]      =   {CPLD_REG_90,   MASK_ALL,   DATA_HEX},

    // LED Status for Offset 0x91 Port QSFP LED8~LED15 Blinking control
    [CPLD_QSFP_BLINKING_PORT8_15]     =   {CPLD_REG_91,   MASK_ALL,   DATA_HEX},

    // LED Status for Offset 0x92 Port QSFP LED16~LED23 Blinking control 
    [CPLD_QSFP_BLINKING_PORT16_23]    =   {CPLD_REG_92,   MASK_ALL,   DATA_HEX},

    // LED Status for Offset 0x93 Port QSFP LED24~LED31 Blinking control 
    [CPLD_QSFP_BLINKING_PORT24_31]    =   {CPLD_REG_93,   MASK_ALL,   DATA_HEX},
    
    // LED Status for Offset 0x94 Port SFP+ LED Blinking control
    [CPLD_SFP_BLINKING_SFP0]          =   {CPLD_REG_94,   MASK_BIT0,     DATA_HEX},
    [CPLD_SFP_BLINKING_SFP1]          =   {CPLD_REG_94,   MASK_BIT1,     DATA_HEX},

    //BSP DEBUG
    [BSP_DEBUG]                       =   {CPLD_REG_NONE, MASK_NONE,  DATA_UNK},
};

enum bsp_log_types {
    LOG_NONE,
    LOG_RW,
    LOG_READ,
    LOG_WRITE
};

enum bsp_log_ctrl {
    LOG_DISABLE,
    LOG_ENABLE
};

/* CPLD sysfs attributes hook functions  */
static ssize_t cpld_show(struct device *dev,
        struct device_attribute *da, char *buf);
static ssize_t cpld_store(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count);
static u8 _cpld_reg_read(struct device *dev, u8 reg, u8 mask);
static ssize_t cpld_reg_read(struct device *dev, char *buf, u8 reg, u8 mask, u8 data_type);
static ssize_t cpld_reg_write(struct device *dev, const char *buf, size_t count, u8 reg, u8 mask);
static ssize_t bsp_read(char *buf, char *str);
static ssize_t bsp_write(const char *buf, char *str, size_t str_len, size_t count);
static ssize_t bsp_callback_show(struct device *dev,
        struct device_attribute *da, char *buf);
static ssize_t bsp_callback_store(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count);
static ssize_t mb_cpld_fw_version_show(struct device *dev,
                    struct device_attribute *da,
                    char *buf);

static LIST_HEAD(cpld_client_list);  /* client list for cpld */
static struct mutex list_lock;  /* mutex for client list */

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

struct cpld_data {
    int index;                  /* CPLD index */
    struct mutex access_lock;   /* mutex for cpld access */
    u8 access_reg;              /* register to access */
};

/* CPLD device id and data */
static const struct i2c_device_id cpld_id[] = {
    { "s9180_32x_cpld1",  cpld1 },
    {}
};

char bsp_debug[2]="0";
u8 enable_log_read=LOG_DISABLE;
u8 enable_log_write=LOG_DISABLE;

/* Addresses scanned for cpld */
static const unsigned short cpld_i2c_addr[] = { 0x33, I2C_CLIENT_END };

/* define all support register access of cpld in attribute */

/* MB CPLD Registers attributes */
static SENSOR_DEVICE_ATTR_RO(cpld_type_rev_raw,                 cpld,                     CPLD_TYPE_REV_RAW);
static SENSOR_DEVICE_ATTR_RO(cpld_build_rev,                    cpld,                     CPLD_BUILD_REV);
static SENSOR_DEVICE_ATTR_RO(cpld_hw_rev,                       cpld,                     CPLD_HW_REV);
static SENSOR_DEVICE_ATTR_RO(cpld_model_id,                     cpld,                     CPLD_MODEL_ID);
static SENSOR_DEVICE_ATTR_RO(cpld_board_id,                     cpld,                     CPLD_BOARD_ID);
static SENSOR_DEVICE_ATTR_RO(cpld_cpld_rev,                     cpld,                     CPLD_CPLD_REV);
static SENSOR_DEVICE_ATTR_RO(cpld_release_bit,                  cpld,                     CPLD_RELEASE_BIT);
static SENSOR_DEVICE_ATTR_RO(mb_cpld_fw_version,                mb_cpld_fw_version,       MB_CPLD_FW_VERSION);
static SENSOR_DEVICE_ATTR_RW(cpld_enable_raw,                   cpld,                     CPLD_ENABLE_RAW);
static SENSOR_DEVICE_ATTR_RW(cpld_u5239_enable,                 cpld,                     CPLD_U5239_ENABLE);
static SENSOR_DEVICE_ATTR_RW(cpld_u5240_enable,                 cpld,                     CPLD_U5240_ENABLE);
static SENSOR_DEVICE_ATTR_RO(cpld_rov_raw,                      cpld,                     CPLD_ROV_RAW);
static SENSOR_DEVICE_ATTR_RO(cpld_pinj5_rov0,                   cpld,                     CPLD_PINJ5_ROV0);
static SENSOR_DEVICE_ATTR_RO(cpld_pinn4_rov1,                   cpld,                     CPLD_PINN4_ROV1);
static SENSOR_DEVICE_ATTR_RO(cpld_pink5_rov2,                   cpld,                     CPLD_PINK5_ROV2);
static SENSOR_DEVICE_ATTR_RO(cpld_pinn5_sync,                   cpld,                     CPLD_PINN5_SYNC);
static SENSOR_DEVICE_ATTR_RO(cpld_pinn6_tap_l,                  cpld,                     CPLD_PINN6_TAP_L);
static SENSOR_DEVICE_ATTR_RW(cpld_download_sel,                 cpld,                     CPLD_DOWNLOAD_SEL);
static SENSOR_DEVICE_ATTR_RW(cpld_led_raw,                      cpld,                     CPLD_LED_RAW);
static SENSOR_DEVICE_ATTR_RW(cpld_cpld1_led_ctrl,               cpld,                     CPLD_CPLD1_LED_CTRL);
static SENSOR_DEVICE_ATTR_RW(cpld_led_clear,                    cpld,                     CPLD_LED_CLEAR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led0_to_led3_color,      cpld,                     CPLD_QSFP_LED0_TO_LED3_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led0_color,              cpld,                     CPLD_QSFP_LED0_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led1_color,              cpld,                     CPLD_QSFP_LED1_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led2_color,              cpld,                     CPLD_QSFP_LED2_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led3_color,              cpld,                     CPLD_QSFP_LED3_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led4_to_led7_color,      cpld,                     CPLD_QSFP_LED4_TO_LED7_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led4_color,              cpld,                     CPLD_QSFP_LED4_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led5_color,              cpld,                     CPLD_QSFP_LED5_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led6_color,              cpld,                     CPLD_QSFP_LED6_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led7_color,              cpld,                     CPLD_QSFP_LED7_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led8_to_led11_color,     cpld,                     CPLD_QSFP_LED8_TO_LED11_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led8_color,              cpld,                     CPLD_QSFP_LED8_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led9_color,              cpld,                     CPLD_QSFP_LED9_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led10_color,             cpld,                     CPLD_QSFP_LED10_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led11_color,             cpld,                     CPLD_QSFP_LED11_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led12_to_led15_color,     cpld,                    CPLD_QSFP_LED12_TO_LED15_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led12_color,              cpld,                    CPLD_QSFP_LED12_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led13_color,              cpld,                    CPLD_QSFP_LED13_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led14_color,              cpld,                    CPLD_QSFP_LED14_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led15_color,              cpld,                    CPLD_QSFP_LED15_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led16_to_led19_color,     cpld,                    CPLD_QSFP_LED16_TO_LED19_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led16_color,              cpld,                    CPLD_QSFP_LED16_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led17_color,              cpld,                    CPLD_QSFP_LED17_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led18_color,              cpld,                    CPLD_QSFP_LED18_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led19_color,              cpld,                    CPLD_QSFP_LED19_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led20_to_led23_color,     cpld,                    CPLD_QSFP_LED20_TO_LED23_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led20_color,              cpld,                    CPLD_QSFP_LED20_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led21_color,              cpld,                    CPLD_QSFP_LED21_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led22_color,              cpld,                    CPLD_QSFP_LED22_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led23_color,              cpld,                    CPLD_QSFP_LED23_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led24_to_led27_color,     cpld,                    CPLD_QSFP_LED24_TO_LED27_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led24_color,              cpld,                    CPLD_QSFP_LED24_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led25_color,              cpld,                    CPLD_QSFP_LED25_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led26_color,              cpld,                    CPLD_QSFP_LED26_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led27_color,              cpld,                    CPLD_QSFP_LED27_COLOR);;

static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led28_to_led31_color,     cpld,                    CPLD_QSFP_LED28_TO_LED31_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led28_color,              cpld,                    CPLD_QSFP_LED28_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led29_color,              cpld,                    CPLD_QSFP_LED29_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led30_color,              cpld,                    CPLD_QSFP_LED30_COLOR);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_led31_color,              cpld,                    CPLD_QSFP_LED31_COLOR);

static SENSOR_DEVICE_ATTR_RW(cpld_sfp0_led,                     cpld,                     CPLD_SFP0_LED);
static SENSOR_DEVICE_ATTR_RW(cpld_sfp1_led,                     cpld,                     CPLD_SFP1_LED);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_blinking_port0_7,        cpld,                     CPLD_QSFP_BLINKING_PORT0_7);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_blinking_port8_15,       cpld,                     CPLD_QSFP_BLINKING_PORT8_15);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_blinking_port16_23,      cpld,                     CPLD_QSFP_BLINKING_PORT16_23);
static SENSOR_DEVICE_ATTR_RW(cpld_qsfp_blinking_port24_31,      cpld,                     CPLD_QSFP_BLINKING_PORT24_31);
static SENSOR_DEVICE_ATTR_RW(cpld_sfp_blinking_sfp0,            cpld,                     CPLD_SFP_BLINKING_SFP0);
static SENSOR_DEVICE_ATTR_RW(cpld_sfp_blinking_sfp1,            cpld,                     CPLD_SFP_BLINKING_SFP1);

//BSP DEBUG
static SENSOR_DEVICE_ATTR_RW(bsp_debug,                         bsp_callback,             BSP_DEBUG);

/* define support attributes of cpldx */

/* cpld 1 */
static struct attribute *cpld1_attributes[] = {
    _DEVICE_ATTR(cpld_type_rev_raw),
    _DEVICE_ATTR(cpld_build_rev),
    _DEVICE_ATTR(cpld_hw_rev),
    _DEVICE_ATTR(cpld_model_id),
    _DEVICE_ATTR(cpld_board_id),
    _DEVICE_ATTR(cpld_cpld_rev),
    _DEVICE_ATTR(cpld_release_bit),
    _DEVICE_ATTR(mb_cpld_fw_version),
    _DEVICE_ATTR(cpld_enable_raw),
    _DEVICE_ATTR(cpld_u5239_enable),
    _DEVICE_ATTR(cpld_u5240_enable),
    _DEVICE_ATTR(cpld_rov_raw),
    _DEVICE_ATTR(cpld_pinj5_rov0),
    _DEVICE_ATTR(cpld_pinn4_rov1),
    _DEVICE_ATTR(cpld_pink5_rov2),
    _DEVICE_ATTR(cpld_pinn5_sync),
    _DEVICE_ATTR(cpld_pinn6_tap_l),
    _DEVICE_ATTR(cpld_download_sel),
    _DEVICE_ATTR(cpld_led_raw),
    _DEVICE_ATTR(cpld_cpld1_led_ctrl),
    _DEVICE_ATTR(cpld_led_clear),

    _DEVICE_ATTR(cpld_qsfp_led0_to_led3_color),
    _DEVICE_ATTR(cpld_qsfp_led0_color),
    _DEVICE_ATTR(cpld_qsfp_led1_color),
    _DEVICE_ATTR(cpld_qsfp_led2_color),
    _DEVICE_ATTR(cpld_qsfp_led3_color),

    _DEVICE_ATTR(cpld_qsfp_led4_to_led7_color),
    _DEVICE_ATTR(cpld_qsfp_led4_color),
    _DEVICE_ATTR(cpld_qsfp_led5_color),
    _DEVICE_ATTR(cpld_qsfp_led6_color),
    _DEVICE_ATTR(cpld_qsfp_led7_color),

    _DEVICE_ATTR(cpld_qsfp_led8_to_led11_color),
    _DEVICE_ATTR(cpld_qsfp_led8_color),
    _DEVICE_ATTR(cpld_qsfp_led9_color),
    _DEVICE_ATTR(cpld_qsfp_led10_color),
    _DEVICE_ATTR(cpld_qsfp_led11_color),

    _DEVICE_ATTR(cpld_qsfp_led12_to_led15_color),
    _DEVICE_ATTR(cpld_qsfp_led12_color),
    _DEVICE_ATTR(cpld_qsfp_led13_color),
    _DEVICE_ATTR(cpld_qsfp_led14_color),
    _DEVICE_ATTR(cpld_qsfp_led15_color),

    _DEVICE_ATTR(cpld_qsfp_led16_to_led19_color),
    _DEVICE_ATTR(cpld_qsfp_led16_color),
    _DEVICE_ATTR(cpld_qsfp_led17_color),
    _DEVICE_ATTR(cpld_qsfp_led18_color),
    _DEVICE_ATTR(cpld_qsfp_led19_color),

    _DEVICE_ATTR(cpld_qsfp_led20_to_led23_color),
    _DEVICE_ATTR(cpld_qsfp_led20_color),
    _DEVICE_ATTR(cpld_qsfp_led21_color),
    _DEVICE_ATTR(cpld_qsfp_led22_color),
    _DEVICE_ATTR(cpld_qsfp_led23_color),

    _DEVICE_ATTR(cpld_qsfp_led24_to_led27_color),
    _DEVICE_ATTR(cpld_qsfp_led24_color),
    _DEVICE_ATTR(cpld_qsfp_led25_color),
    _DEVICE_ATTR(cpld_qsfp_led26_color),
    _DEVICE_ATTR(cpld_qsfp_led27_color),

    _DEVICE_ATTR(cpld_qsfp_led28_to_led31_color),
    _DEVICE_ATTR(cpld_qsfp_led28_color),
    _DEVICE_ATTR(cpld_qsfp_led29_color),
    _DEVICE_ATTR(cpld_qsfp_led30_color),
    _DEVICE_ATTR(cpld_qsfp_led31_color),

    _DEVICE_ATTR(cpld_sfp0_led),
    _DEVICE_ATTR(cpld_sfp1_led),
    _DEVICE_ATTR(cpld_qsfp_blinking_port0_7),
    _DEVICE_ATTR(cpld_qsfp_blinking_port8_15),
    _DEVICE_ATTR(cpld_qsfp_blinking_port16_23),
    _DEVICE_ATTR(cpld_qsfp_blinking_port24_31),
    _DEVICE_ATTR(cpld_sfp_blinking_sfp0),
    _DEVICE_ATTR(cpld_sfp_blinking_sfp1),

    _DEVICE_ATTR(bsp_debug),

    NULL
};

/* cpld 1 attributes group */
static const struct attribute_group cpld1_group = {
    .attrs = cpld1_attributes,
};

/* reg shift */
static u8 _shift(u8 mask)
{
    int i=0, mask_one=1;

    for(i=0; i<8; ++i) {
        if ((mask & mask_one) == 1)
            return i;
        else
            mask >>= 1;
    }

    return -1;
}

/* reg mask and shift */
static u8 _mask_shift(u8 val, u8 mask)
{
    int shift=0;

    shift = _shift(mask);

    return (val & mask) >> shift;
}

static u8 _parse_data(char *buf, unsigned int data, u8 data_type)
{
    if(buf == NULL) {
        return -1;
    }

    if(data_type == DATA_HEX) {
        return sprintf(buf, "0x%02x", data);
    } else if(data_type == DATA_DEC) {
        return sprintf(buf, "%u", data);
    } else {
        return -1;
    }
    return 0;
}

static int _bsp_log(u8 log_type, char *fmt, ...)
{
    if ((log_type==LOG_READ  && enable_log_read) ||
        (log_type==LOG_WRITE && enable_log_write)) {
        va_list args;
        int r;

        va_start(args, fmt);
        r = vprintk(fmt, args);
        va_end(args);

        return r;
    } else {
        return 0;
    }
}

static int _config_bsp_log(u8 log_type)
{
    switch(log_type) {
        case LOG_NONE:
            enable_log_read = LOG_DISABLE;
            enable_log_write = LOG_DISABLE;
            break;
        case LOG_RW:
            enable_log_read = LOG_ENABLE;
            enable_log_write = LOG_ENABLE;
            break;
        case LOG_READ:
            enable_log_read = LOG_ENABLE;
            enable_log_write = LOG_DISABLE;
            break;
        case LOG_WRITE:
            enable_log_read = LOG_DISABLE;
            enable_log_write = LOG_ENABLE;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

/* get bsp value */
static ssize_t bsp_read(char *buf, char *str)
{
    ssize_t len=0;

    len=sprintf(buf, "%s", str);
    BSP_LOG_R("reg_val=%s", str);

    return len;
}

/* set bsp value */
static ssize_t bsp_write(const char *buf, char *str, size_t str_len, size_t count)
{
    snprintf(str, str_len, "%s", buf);
    BSP_LOG_W("reg_val=%s", str);

    return count;
}

/* get bsp parameter value */
static ssize_t bsp_callback_show(struct device *dev,
        struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    int str_len=0;
    char *str=NULL;

    switch (attr->index) {
        case BSP_DEBUG:
            str = bsp_debug;
            str_len = sizeof(bsp_debug);
            break;
        default:
            return -EINVAL;
    }
    return bsp_read(buf, str);
}

/* set bsp parameter value */
static ssize_t bsp_callback_store(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    int str_len=0;
    char *str=NULL;
    ssize_t ret = 0;
    u8 bsp_debug_u8 = 0;

    switch (attr->index) {
        case BSP_DEBUG:
            str = bsp_debug;
            str_len = sizeof(bsp_debug);
            ret = bsp_write(buf, str, str_len, count);

            if (kstrtou8(buf, 0, &bsp_debug_u8) < 0) {
                return -EINVAL;
            } else if (_config_bsp_log(bsp_debug_u8) < 0) {
                return -EINVAL;
            }
            return ret;
        default:
            return -EINVAL;
    }
    return 0;
}

/* get cpld register value */
static ssize_t cpld_show(struct device *dev,
        struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u8 reg = 0;
    u8 mask = MASK_NONE;
    u8 data_type=DATA_UNK;

    switch (attr->index) {
        case CPLD_TYPE_REV_RAW:
        case CPLD_BUILD_REV:
        case CPLD_HW_REV:
        case CPLD_MODEL_ID:
        case CPLD_BOARD_ID:
        case CPLD_CPLD_REV:
        case CPLD_RELEASE_BIT:
        case CPLD_ENABLE_RAW:
        case CPLD_U5239_ENABLE:
        case CPLD_U5240_ENABLE:
        case CPLD_ROV_RAW:
        case CPLD_PINJ5_ROV0:
        case CPLD_PINN4_ROV1:
        case CPLD_PINK5_ROV2:
        case CPLD_PINN5_SYNC:
        case CPLD_PINN6_TAP_L:
        case CPLD_DOWNLOAD_SEL:
        case CPLD_LED_RAW:
        case CPLD_CPLD1_LED_CTRL:
        case CPLD_LED_CLEAR:
        case CPLD_QSFP_LED0_TO_LED3_COLOR:
        case CPLD_QSFP_LED0_COLOR:
        case CPLD_QSFP_LED1_COLOR:
        case CPLD_QSFP_LED2_COLOR:
        case CPLD_QSFP_LED3_COLOR:
        case CPLD_QSFP_LED4_TO_LED7_COLOR:
        case CPLD_QSFP_LED4_COLOR:
        case CPLD_QSFP_LED5_COLOR:
        case CPLD_QSFP_LED6_COLOR:
        case CPLD_QSFP_LED7_COLOR:
        case CPLD_QSFP_LED8_TO_LED11_COLOR:
        case CPLD_QSFP_LED8_COLOR:
        case CPLD_QSFP_LED9_COLOR:
        case CPLD_QSFP_LED10_COLOR:
        case CPLD_QSFP_LED11_COLOR:
        case CPLD_QSFP_LED12_TO_LED15_COLOR:
        case CPLD_QSFP_LED12_COLOR:
        case CPLD_QSFP_LED13_COLOR:
        case CPLD_QSFP_LED14_COLOR:
        case CPLD_QSFP_LED15_COLOR:
        case CPLD_QSFP_LED16_TO_LED19_COLOR:
        case CPLD_QSFP_LED16_COLOR:
        case CPLD_QSFP_LED17_COLOR:
        case CPLD_QSFP_LED18_COLOR:
        case CPLD_QSFP_LED19_COLOR:
        case CPLD_QSFP_LED20_TO_LED23_COLOR:
        case CPLD_QSFP_LED20_COLOR:
        case CPLD_QSFP_LED21_COLOR:
        case CPLD_QSFP_LED22_COLOR:
        case CPLD_QSFP_LED23_COLOR:
        case CPLD_QSFP_LED24_TO_LED27_COLOR:
        case CPLD_QSFP_LED24_COLOR:
        case CPLD_QSFP_LED25_COLOR:
        case CPLD_QSFP_LED26_COLOR:
        case CPLD_QSFP_LED27_COLOR:
        case CPLD_QSFP_LED28_TO_LED31_COLOR:
        case CPLD_QSFP_LED28_COLOR:
        case CPLD_QSFP_LED29_COLOR:
        case CPLD_QSFP_LED30_COLOR:
        case CPLD_QSFP_LED31_COLOR:
        case CPLD_SFP0_LED:
        case CPLD_SFP1_LED:
        case CPLD_QSFP_BLINKING_PORT0_7:
        case CPLD_QSFP_BLINKING_PORT8_15:
        case CPLD_QSFP_BLINKING_PORT16_23:
        case CPLD_QSFP_BLINKING_PORT24_31:
        case CPLD_SFP_BLINKING_SFP0:
        case CPLD_SFP_BLINKING_SFP1:
            reg = attr_reg[attr->index].reg;
            mask= attr_reg[attr->index].mask;
            data_type = attr_reg[attr->index].data_type;
            break;
        default:
            return -EINVAL;
    }
    return cpld_reg_read(dev, buf, reg, mask, data_type);
}

/* set cpld register value */
static ssize_t cpld_store(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u8 reg = 0;
    u8 mask = MASK_NONE;

    switch (attr->index) {
        case CPLD_TYPE_REV_RAW:
        case CPLD_BUILD_REV:
        case CPLD_HW_REV:
        case CPLD_MODEL_ID:
        case CPLD_BOARD_ID:
        case CPLD_CPLD_REV:
        case CPLD_RELEASE_BIT:
        case CPLD_ENABLE_RAW:
        case CPLD_U5239_ENABLE:
        case CPLD_U5240_ENABLE:
        case CPLD_ROV_RAW:
        case CPLD_PINJ5_ROV0:
        case CPLD_PINN4_ROV1:
        case CPLD_PINK5_ROV2:
        case CPLD_PINN5_SYNC:
        case CPLD_PINN6_TAP_L:
        case CPLD_DOWNLOAD_SEL:
        case CPLD_LED_RAW:
        case CPLD_CPLD1_LED_CTRL:
        case CPLD_LED_CLEAR:
        case CPLD_QSFP_LED0_TO_LED3_COLOR:
        case CPLD_QSFP_LED0_COLOR:
        case CPLD_QSFP_LED1_COLOR:
        case CPLD_QSFP_LED2_COLOR:
        case CPLD_QSFP_LED3_COLOR:
        case CPLD_QSFP_LED4_TO_LED7_COLOR:
        case CPLD_QSFP_LED4_COLOR:
        case CPLD_QSFP_LED5_COLOR:
        case CPLD_QSFP_LED6_COLOR:
        case CPLD_QSFP_LED7_COLOR:
        case CPLD_QSFP_LED8_TO_LED11_COLOR:
        case CPLD_QSFP_LED8_COLOR:
        case CPLD_QSFP_LED9_COLOR:
        case CPLD_QSFP_LED10_COLOR:
        case CPLD_QSFP_LED11_COLOR:
        case CPLD_QSFP_LED12_TO_LED15_COLOR:
        case CPLD_QSFP_LED12_COLOR:
        case CPLD_QSFP_LED13_COLOR:
        case CPLD_QSFP_LED14_COLOR:
        case CPLD_QSFP_LED15_COLOR:
        case CPLD_QSFP_LED16_TO_LED19_COLOR:
        case CPLD_QSFP_LED16_COLOR:
        case CPLD_QSFP_LED17_COLOR:
        case CPLD_QSFP_LED18_COLOR:
        case CPLD_QSFP_LED19_COLOR:
        case CPLD_QSFP_LED20_TO_LED23_COLOR:
        case CPLD_QSFP_LED20_COLOR:
        case CPLD_QSFP_LED21_COLOR:
        case CPLD_QSFP_LED22_COLOR:
        case CPLD_QSFP_LED23_COLOR:
        case CPLD_QSFP_LED24_TO_LED27_COLOR:
        case CPLD_QSFP_LED24_COLOR:
        case CPLD_QSFP_LED25_COLOR:
        case CPLD_QSFP_LED26_COLOR:
        case CPLD_QSFP_LED27_COLOR:
        case CPLD_QSFP_LED28_TO_LED31_COLOR:
        case CPLD_QSFP_LED28_COLOR:
        case CPLD_QSFP_LED29_COLOR:
        case CPLD_QSFP_LED30_COLOR:
        case CPLD_QSFP_LED31_COLOR:
        case CPLD_SFP0_LED:
        case CPLD_SFP1_LED:
        case CPLD_QSFP_BLINKING_PORT0_7:
        case CPLD_QSFP_BLINKING_PORT8_15:
        case CPLD_QSFP_BLINKING_PORT16_23:
        case CPLD_QSFP_BLINKING_PORT24_31:
        case CPLD_SFP_BLINKING_SFP0:
        case CPLD_SFP_BLINKING_SFP1:
            reg = attr_reg[attr->index].reg;
            mask= attr_reg[attr->index].mask;
            break;
        default:
            return -EINVAL;
    }
    return cpld_reg_write(dev, buf, count, reg, mask);
}

/* get cpld register value */
static u8 _cpld_reg_read(struct device *dev,
                    u8 reg,
                    u8 mask)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    int reg_val;

    I2C_READ_BYTE_DATA(reg_val, &data->access_lock, client, reg);

    if (unlikely(reg_val < 0)) {
        return reg_val;
    } else {
        reg_val=_mask_shift(reg_val, mask);
        return reg_val;
    }
}

/* get cpld register value */
static ssize_t cpld_reg_read(struct device *dev,
                    char *buf,
                    u8 reg,
                    u8 mask,
                    u8 data_type)
{
    int reg_val;

    reg_val = _cpld_reg_read(dev, reg, mask);
    if (unlikely(reg_val < 0)) {
        dev_err(dev, "cpld_reg_read() error, reg_val=%d\n", reg_val);
        return reg_val;
    } else {
        return _parse_data(buf, reg_val, data_type);
    }
}

/* set cpld register value */
static ssize_t cpld_reg_write(struct device *dev,
                    const char *buf,
                    size_t count,
                    u8 reg,
                    u8 mask)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    u8 reg_val, reg_val_now, shift;
    int ret = 0;

    if (kstrtou8(buf, 0, &reg_val) < 0)
        return -EINVAL;

    //apply continuous bits operation if mask is specified, discontinuous bits are not supported
    if (mask != MASK_ALL) {
        reg_val_now = _cpld_reg_read(dev, reg, MASK_ALL);
        if (unlikely(reg_val_now < 0)) {
            dev_err(dev, "cpld_reg_write() error, reg_val_now=%d\n", reg_val_now);
            return reg_val_now;
        } else {
            //clear bits in reg_val_now by the mask
            reg_val_now &= ~mask;
            //get bit shift by the mask
            shift = _shift(mask);
            //calculate new reg_val
            reg_val = reg_val_now | (reg_val << shift);
        }
    }

    I2C_WRITE_BYTE_DATA(ret, &data->access_lock,
                    client, reg, reg_val);

    if (unlikely(ret < 0)) {
        dev_err(dev, "cpld_reg_write() error, return=%d\n", ret);
        return ret;
    }

    return count;
}

/* get cpld_fw_version register value */
//static ssize_t cpld_version_h_show(struct device *dev,
static ssize_t mb_cpld_fw_version_show(struct device *dev,
                    struct device_attribute *da,
                    char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

    if (attr->index == MB_CPLD_FW_VERSION) {
        return sprintf(buf, "%d.%02d.%03d",
                _cpld_reg_read(dev, attr_reg[CPLD_RELEASE_BIT].reg, attr_reg[CPLD_RELEASE_BIT].mask),  //major
                _cpld_reg_read(dev, attr_reg[CPLD_CPLD_REV].reg, attr_reg[CPLD_CPLD_REV].mask),  //minor
                _cpld_reg_read(dev, attr_reg[CPLD_BUILD_REV].reg, attr_reg[CPLD_BUILD_REV].mask));
    }
    return -1;
}

/* add valid cpld client to list */
static void cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = NULL; 

    node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
    if (!node) {
        dev_info(&client->dev,
            "Can't allocate cpld_client_node for index %d\n",
            client->addr);
        return;
    }

    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
    mutex_unlock(&list_lock);
}

/* remove exist cpld client in list */
static void cpld_remove_client(struct i2c_client *client)
{
    struct list_head    *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

    mutex_lock(&list_lock);
    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node,
                struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }
    mutex_unlock(&list_lock);
}

/* cpld drvier probe */
static int cpld_probe(struct i2c_client *client,
                    const struct i2c_device_id *dev_id)
{
    int status;
    struct cpld_data *data = NULL;
    int ret = -EPERM;

    data = kzalloc(sizeof(struct cpld_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    /* init cpld data for client */
    i2c_set_clientdata(client, data);
    mutex_init(&data->access_lock);

    if (!i2c_check_functionality(client->adapter,
                I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_info(&client->dev,
            "i2c_check_functionality failed (0x%x)\n",
            client->addr);
        status = -EIO;
        goto exit;
    }

    /* get cpld id from device */
    ret = i2c_smbus_read_byte_data(client, CPLD_REG_07);

    if (ret < 0) {
        dev_info(&client->dev,
            "fail to get cpld id (0x%x) at addr (0x%x)\n",
            CPLD_REG_07, client->addr);
        status = -EIO;
        goto exit;
    }

    if (INVALID(ret, cpld1, 0xf)) {
        dev_info(&client->dev,
            "cpld id %d(device) not valid\n", ret);
    }

    data->index = dev_id->driver_data;

    /* register sysfs hooks for different cpld group */
    dev_info(&client->dev, "probe cpld with index %d\n", data->index);
    switch (data->index) {
    case cpld1:
        status = sysfs_create_group(&client->dev.kobj,
                    &cpld1_group);
        break;
    default:
        status = -EINVAL;
    }

    if (status)
        goto exit;

    dev_info(&client->dev, "chip found\n");

    /* add probe chip to client list */
    cpld_add_client(client);

    return 0;
exit:
    switch (data->index) {
    case cpld1:
        sysfs_remove_group(&client->dev.kobj, &cpld1_group);
        break;
    default:
        break;
    }
    return status;
}

/* cpld drvier remove */
static int cpld_remove(struct i2c_client *client)
{
    struct cpld_data *data = i2c_get_clientdata(client);

    switch (data->index) {
    case cpld1:
        sysfs_remove_group(&client->dev.kobj, &cpld1_group);
        break;
    }

    cpld_remove_client(client);
    return 0;
}

MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
    .class      = I2C_CLASS_HWMON,
    .driver = {
        .name = "x86_64_ufispace_s9180_32x_cpld",
    },
    .probe = cpld_probe,
    .remove = cpld_remove,
    .id_table = cpld_id,
    .address_list = cpld_i2c_addr,
};

static int __init cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&cpld_driver);
}

static void __exit cpld_exit(void)
{
    i2c_del_driver(&cpld_driver);
}

MODULE_AUTHOR("Melo Lin <melo.lin@ufispace.com>");
MODULE_DESCRIPTION("x86_64_ufispace_s9180_32x_cpld driver");
MODULE_LICENSE("GPL");

module_init(cpld_init);
module_exit(cpld_exit);
