/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *      Platform Library
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/sfpi.h>
#include "x86_64_ufispace_s9180_32x_log.h"

#define ONLP_TRY(_expr)                                                 \
    do {                                                                \
        int _rv = (_expr);                                              \
        if(ONLP_FAILURE(_rv)) {                                         \
            AIM_LOG_ERROR("%s returned %{onlp_status}\r\n", #_expr, _rv);   \
            return _rv;                                                 \
        }                                                               \
    } while(0)

#define POID_0 0
#define I2C_BUS(_bus) (_bus)

#define COMM_STR_NOT_SUPPORTED              "not supported"
#define COMM_STR_NOT_AVAILABLE              "not available"

#define SYS_FMT                             "/sys/bus/i2c/devices/%d-%04x/%s"
#define SYS_FMT_OFFSET                      "/sys/bus/i2c/devices/%d-%04x/%s_%d"
#define SYS_GPIO_FMT                        "/sys/class/gpio/gpio%d/value"
//MB CPLD
#define PATH_MB_CPLD                        "/sys/bus/i2c/devices/44-0033/"
#define LPC_BSP_FMT                         "/sys/devices/platform/x86_64_ufispace_s9180_32x_lpc/bsp/"
#define SYS_CPU_CORETEMP_PREFIX             "/sys/devices/platform/coretemp.0/hwmon/hwmon1/"
#define SYS_CPU_CORETEMP_PREFIX2            "/sys/devices/platform/coretemp.0/"

#define SYS_CPU_TEMP_PREFIX                 "/sys/class/hwmon/hwmon0/"
#define SYS_CORE_TEMP_PREFIX                "/sys/class/hwmon/hwmon2/"
#define SYS_CPU_BOARD_TEMP_PREFIX           "/sys/class/hwmon/hwmon3/"
#define SYS_PSU1_NEAR_TEMP_PREFIX           "/sys/class/hwmon/hwmon4/"
#define SYS_PSU2_NEAR_TEMP_PREFIX           "/sys/class/hwmon/hwmon7/"
#define SYS_MAC_REAR_TEMP_PREFIX            "/sys/class/hwmon/hwmon5/"
#define SYS_QSFP_NEAR_TEMP_PREFIX           "/sys/class/hwmon/hwmon6/"

#define I2C_STUCK_CHECK_CMD                 "i2cget -f -y 0 0x70 > /dev/null 2>&1"
#define MUX_RESET_PATH                      "/sys/devices/platform/x86_64_ufispace_s9180_32x_lpc/mb_cpld/mux_reset_all"
#define SYSFS_DEVICES                       "/sys/bus/i2c/devices/"
#define SYSFS_CPLD1                         SYSFS_DEVICES "44-0033/"
#define SYS_FAN_PREFIX                      "/sys/class/hwmon/hwmon1/device/"
#define SYS_EEPROM_PATH                     "/sys/bus/i2c/devices/0-0051/eeprom"
#define PSU1_EEPROM_PATH                    "/sys/bus/i2c/devices/58-0050/eeprom"
#define PSU2_EEPROM_PATH                    "/sys/bus/i2c/devices/57-0050/eeprom"

#define PSU_STATUS_PRSNT                    (1)
#define PSU_STATUS_POWER_GOOD               (1)
#define PSU_STATUS_POWER_FAILED             (0)
#define PSU_STATUS_ABS                      (0)
#define FAN_PRESENT                         (0)
#define FAN_CTRL_SET1                       (1)
#define FAN_CTRL_SET2                       (2)
#define MAX_SYS_FAN_NUM                     (8)
#define STD_BOARD_THERMAL_NUM               (6)
#define ADV_BOARD_THERMAL_NUM               (8)
#define SYS_FAN_NUM                         (8)
#define FAN_TRAY_NUM                        (4)

/* I2c bus */
#define I2C_BUS_0                           (0)
#define I2C_BUS_1                           (1)
#define I2C_BUS_2                           (2)
#define I2C_BUS_3                           (3)
#define I2C_BUS_4                           (4)
#define I2C_BUS_5                           (5)
#define I2C_BUS_6                           (6)
#define I2C_BUS_7                           (7)
#define I2C_BUS_8                           (8)  
#define I2C_BUS_9                           (9)  
#define I2C_BUS_44                          (44)            /* cpld */
#define I2C_BUS_50                          (50)            /* SYS LED */
#define I2C_BUS_57                          (57)            /* PSU2 */
#define I2C_BUS_58                          (58)            /* PSU1 */
#define I2C_BUS_59                          (59)            /* FRU  */
#define I2C_BUS_PSU1                        I2C_BUS_58      /* PSU1 */
#define I2C_BUS_PSU2                        I2C_BUS_57      /* PSU2 */

/* Thermal threshold */
#define THERMAL_WARNING_DEFAULT             (77000)
#define THERMAL_ERROR_DEFAULT               (95000)
#define THERMAL_SHUTDOWN_DEFAULT            (105000)
#define THERMAL_CPU_WARNING                 THERMAL_WARNING_DEFAULT
#define THERMAL_CPU_ERROR                   THERMAL_ERROR_DEFAULT
#define THERMAL_CPU_SHUTDOWN                THERMAL_SHUTDOWN_DEFAULT
#define THERMAL_CPU_STD_WARNING             THERMAL_WARNING_DEFAULT
#define THERMAL_CPU_STD_ERROR               THERMAL_ERROR_DEFAULT
#define THERMAL_CPU_STD_SHUTDOWN            THERMAL_SHUTDOWN_DEFAULT

#define THERMAL_MAC_FRONT_ERROR             (65)
#define THERMAL_MAC_FRONT_SHUTDOWN          (75)
#define THERMAL_MAC_REAR_ERROR              (65)
#define THERMAL_MAC_REAR_SHUTDOWN           (75)
#define THERMAL_MAC_ERROR                   (95)
#define THERMAL_MAC_SHUTDOWN                (105)
#define THERMAL_ENV_BMC_ERROR               (65)
#define THERMAL_ENV_BMC_SHUTDOWN            (75)
#define THERMAL_TEMP_BATTERY_ERROR          (65)
#define THERMAL_TEMP_BATTERY_SHUTDOWN       (75)
#define THERMAL_TEMP_OOB_CONN_ERROR         (65)
#define THERMAL_TEMP_OOB_CONN_SHUTDOWN      (75)
#define THERMAL_TEMP_SFP_CONN_ERROR         (65)
#define THERMAL_TEMP_SFP_CONN_SHUTDOWN      (75)
#define THERMAL_TEMP_PCIE_CONN_ERROR        (65)
#define THERMAL_TEMP_PCIE_CONN_SHUTDOWN     (75)
#define THERMAL_PSU_AMB_ERROR               (60)
#define THERMAL_PSU_AMB_SHUTDOWN            (65)
#define THERMAL_PSU_HS_ERROR                (60)
#define THERMAL_PSU_HS_SHUTDOWN             (65)

#define THERMAL_STATE_NOT_SUPPORT           (0)

#define THERMAL_ERROR_FAN_PERC              (100)

#define THERMAL_WARNING_FAN_PERC            (80)

#define THERMAL_NORMAL_DEFAULT              (72000)
#define THERMAL_NORMAL_FAN_PERC             (50)

/* CPU core-temp sysfs ID */
#define CPU_PKG_CORE_TEMP_SYS_ID            "1"

#define BMC_EN_FILE_PATH                    "/etc/onl/bmc_en"

/* BMC attr */
#define BMC_ATTR_NAME_Temp_BMC                    "Temp_BMC"
#define BMC_ATTR_NAME_Temp_MAC                    "Temp_MAC"  
#define BMC_ATTR_NAME_Temp_MAC_Front              "Temp_MAC_Front"
#define BMC_ATTR_NAME_Temp_MAC_Rear               "Temp_MAC_Rear"
#define BMC_ATTR_NAME_Temp_Battery                "Temp_Battery"
#define BMC_ATTR_NAME_Temp_OOB_conn               "Temp_OOB_conn"
#define BMC_ATTR_NAME_Temp_SFP_conn               "Temp_SFP_conn"
#define BMC_ATTR_NAME_Temp_PCIE_conn              "Temp_PCIE_conn"
#define BMC_ATTR_NAME_Temp_PSU1_AMB               "Temp_PSU1_AMB"
#define BMC_ATTR_NAME_Temp_PSU1_HS                "Temp_PSU1_HS"
#define BMC_ATTR_NAME_Temp_PSU2_AMB               "Temp_PSU2_AMB"
#define BMC_ATTR_NAME_Temp_PSU2_HS                "Temp_PSU2_HS" 
#define BMC_ATTR_NAME_FAN_1_F                     "FAN_1_F"
#define BMC_ATTR_NAME_FAN_2_F                     "FAN_2_F"
#define BMC_ATTR_NAME_FAN_3_F                     "FAN_3_F"
#define BMC_ATTR_NAME_FAN_4_F                     "FAN_4_F"
#define BMC_ATTR_NAME_FAN_1_R                     "FAN_1_R"
#define BMC_ATTR_NAME_FAN_2_R                     "FAN_2_R"
#define BMC_ATTR_NAME_FAN_3_R                     "FAN_3_R"
#define BMC_ATTR_NAME_FAN_4_R                     "FAN_4_R"
#define BMC_ATTR_NAME_Fan_PSU1                    "Fan_PSU1"
#define BMC_ATTR_NAME_Fan_PSU2                    "Fan_PSU2"
#define BMC_ATTR_NAME_Fan1_PRSNT                  "Fan1_PRSNT"
#define BMC_ATTR_NAME_Fan2_PRSNT                  "Fan2_PRSNT"
#define BMC_ATTR_NAME_Fan3_PRSNT                  "Fan3_PRSNT"
#define BMC_ATTR_NAME_Fan4_PRSNT                  "Fan4_PRSNT"
#define BMC_ATTR_NAME_PSU1_PRSNT                  "PSU1_PRSNT"
#define BMC_ATTR_NAME_PSU2_PRSNT                  "PSU2_PRSNT"
#define BMC_ATTR_NAME_CPU_PRSNT                   "CPU_PRSNT" 
#define BMC_ATTR_NAME_ALL_PWRGOOD                 "ALL_PWRGOOD"
#define BMC_ATTR_NAME_THERMAL_ALERT               "THERMAL_ALERT"
#define BMC_ATTR_NAME_HWM_P0V9_VSEN               "HWM_P0V9_VSEN"
#define BMC_ATTR_NAME_HWM_VDDC_VSEN               "HWM_VDDC_VSEN"
#define BMC_ATTR_NAME_HWM_P2V5_VSEN               "HWM_P2V5_VSEN" 
#define BMC_ATTR_NAME_P3V3_VSEN                   "P3V3_VSEN"
#define BMC_ATTR_NAME_BMC_P1V26_VSEN              "BMC_P1V26_VSEN"
#define BMC_ATTR_NAME_BMC_P1V538_VSEN             "BMC_P1V538_VSEN"
#define BMC_ATTR_NAME_BMC_P3V3_VSEN               "BMC_P3V3_VSEN"
#define BMC_ATTR_NAME_BMC_P5V_VSEN                "BMC_P5V_VSEN"
#define BMC_ATTR_NAME_P5V_SB_MON                  "P5V_SB_MON"
#define BMC_ATTR_NAME_P3V3_SB_MON                 "P3V3_SB_MON"
#define BMC_ATTR_NAME_PERI_P3V3_MON               "PERI_P3V3_MON"
#define BMC_ATTR_NAME_P3V3_MON                    "P3V3_MON"
#define BMC_ATTR_NAME_P2V5_MON                    "P2V5_MON"
#define BMC_ATTR_NAME_P1V2_MON                    "P1V2_MON"
#define BMC_ATTR_NAME_P0V9_MON                    "P0V9_MON"
#define BMC_ATTR_NAME_VDD_CORE_MON                "VDD_CORE_MON"
#define BMC_ATTR_NAME_PSU1_INT                    "PSU1_INT"
#define BMC_ATTR_NAME_PSU2_INT                    "PSU2_INT"
#define BMC_ATTR_NAME_PSU1_PWROK                  "PSU1_PWROK"
#define BMC_ATTR_NAME_PSU2_PWROK                  "PSU2_PWROK"
#define BMC_ATTR_NAME_PIN_PSU1                    "PIN_PSU1"
#define BMC_ATTR_NAME_PIN_PSU2                    "PIN_PSU2"
#define BMC_ATTR_NAME_POUT_PSU1                   "POUT_PSU1" 
#define BMC_ATTR_NAME_POUT_PSU2                   "POUT_PSU2" 
#define BMC_ATTR_NAME_VIN_PSU1                    "VIN_PSU1" 
#define BMC_ATTR_NAME_VIN_PSU2                    "VIN_PSU2" 
#define BMC_ATTR_NAME_VOUT_PSU1                   "VOUT_PSU1" 
#define BMC_ATTR_NAME_VOUT_PSU2                   "VOUT_PSU2" 
#define BMC_ATTR_NAME_IIN_PSU1                    "IIN_PSU1" 
#define BMC_ATTR_NAME_IIN_PSU2                    "IIN_PSU2" 
#define BMC_ATTR_NAME_IOUT_PSU1                   "IOUT_PSU1"  
#define BMC_ATTR_NAME_IOUT_PSU2                   "IOUT_PSU2"  

/* BMC cmd */
#define BMC_SENSOR_CACHE                          "/tmp/bmc_sensor_cache"
#define BMC_OEM_CACHE                             "/tmp/bmc_oem_cache"
#define IPMITOOL_REDIRECT_FIRST_ERR               " 2>/tmp/ipmitool_err_msg"
#define IPMITOOL_REDIRECT_ERR                     " 2>>/tmp/ipmitool_err_msg"
#define FAN_CACHE_TIME                      (10)
#define PSU_CACHE_TIME                      (30)
#define THERMAL_CACHE_TIME                  (10)
#define BMC_FIELDS_MAX                      (20)
#define FANDIR_CACHE_TIME                   (60)

/*   IPMITOOL_CMD_TIMEOUT get from ipmitool test.
 *   Test Case: Run 100 times of CMD_BMC_SENSOR_CACHE command and 100 times of CMD_FRU_CACHE_SET command and get the execution times.
 *              We take 10s as The IPMITOOL_CMD_TIMEOUT value
 *              since the CMD_BMC_SENSOR_CACHE execution times value is between 0.216s - 2.926s and
 *                    the CMD_FRU_CACHE_SET execution times value is between 0.031s - 0.076s.
 */

#define IPMITOOL_CMD_TIMEOUT        10
#define CMD_BMC_SENSOR_CACHE        "timeout %ds ipmitool sdr -c get %s"\
                                    "> "BMC_SENSOR_CACHE IPMITOOL_REDIRECT_ERR

#define CMD_BMC_FAN_TRAY_DIR        "timeout %ds ipmitool raw 0x3c 0x31 0x0 | xargs"IPMITOOL_REDIRECT_ERR
#define CMD_BMC_PSU_FAN_DIR         "timeout %ds ipmitool raw 0x3c 0x30 0x0 | xargs"IPMITOOL_REDIRECT_ERR
#define CMD_BMC_OEM_CACHE           CMD_BMC_FAN_TRAY_DIR" > "BMC_OEM_CACHE";"CMD_BMC_PSU_FAN_DIR" >> "BMC_OEM_CACHE
#define BMC_FRU_LINE_SIZE           (256)
#define BMC_FRU_ATTR_KEY_VALUE_SIZE ONLP_CONFIG_INFO_STR_MAX
#define BMC_FRU_ATTR_KEY_VALUE_LEN  (BMC_FRU_ATTR_KEY_VALUE_SIZE - 1)
#define BMC_FRU_KEY_MANUFACTURER    "Product Manufacturer"
#define BMC_FRU_KEY_NAME            "Product Name"
#define BMC_FRU_KEY_PART_NUMBER     "Product Part Number"
#define BMC_FRU_KEY_SERIAL          "Product Serial"

#define CMD_FRU_CACHE_SET "timeout %ds ipmitool fru print %d " \
                        IPMITOOL_REDIRECT_ERR \
                        " | grep %s" \
                        " | awk -F: '/:/{gsub(/^ /,\"\", $0);gsub(/ +:/,\":\",$0);gsub(/: +/,\":\", $0);print $0}'" \
                        " > %s"

#define BMC_FAN_DIR_UNK             (0)
#define BMC_FAN_DIR_B2F             (1)
#define BMC_FAN_DIR_F2B             (2)
#define BMC_ATTR_STATUS_ABS         (0)
#define BMC_ATTR_STATUS_PRES        (1)
#define BMC_ATTR_INVALID_VAL        (999999)

enum sensor
{
    FAN_SENSOR = 0,
    PSU_SENSOR,
    THERMAL_SENSOR,
};

enum bmc_attr_id {
    BMC_ATTR_ID_START = 0,
    BMC_ATTR_ID_Temp_BMC = BMC_ATTR_ID_START,
    BMC_ATTR_ID_Temp_MAC,
    BMC_ATTR_ID_Temp_MAC_Front,
    BMC_ATTR_ID_Temp_MAC_Rear,
    BMC_ATTR_ID_Temp_Battery,
    BMC_ATTR_ID_Temp_OOB_conn,
    BMC_ATTR_ID_Temp_SFP_conn,
    BMC_ATTR_ID_Temp_PCIE_conn,
    BMC_ATTR_ID_Temp_PSU1_AMB,
    BMC_ATTR_ID_Temp_PSU1_HS,
    BMC_ATTR_ID_Temp_PSU2_AMB,
    BMC_ATTR_ID_Temp_PSU2_HS,
    BMC_ATTR_ID_FAN_1_F,    // RPM
    BMC_ATTR_ID_FAN_2_F,    // RPM
    BMC_ATTR_ID_FAN_3_F,    // RPM
    BMC_ATTR_ID_FAN_4_F,    // RPM
    BMC_ATTR_ID_FAN_1_R,    // RPM
    BMC_ATTR_ID_FAN_2_R,    // RPM
    BMC_ATTR_ID_FAN_3_R,    // RPM
    BMC_ATTR_ID_FAN_4_R,    // RPM
    BMC_ATTR_ID_Fan_PSU1,   // RPM
    BMC_ATTR_ID_Fan_PSU2,   // RPM
    BMC_ATTR_ID_Fan1_PRSNT,
    BMC_ATTR_ID_Fan2_PRSNT,
    BMC_ATTR_ID_Fan3_PRSNT,
    BMC_ATTR_ID_Fan4_PRSNT,
    BMC_ATTR_ID_PSU1_PRSNT,
    BMC_ATTR_ID_PSU2_PRSNT,
    BMC_ATTR_ID_CPU_PRSNT,
    BMC_ATTR_ID_ALL_PWRGOOD,
    BMC_ATTR_ID_THERMAL_ALERT,
    BMC_ATTR_ID_HWM_P0V9_VSEN,
    BMC_ATTR_ID_HWM_VDDC_VSEN,
    BMC_ATTR_ID_HWM_P2V5_VSEN,
    BMC_ATTR_ID_P3V3_VSEN,
    BMC_ATTR_ID_BMC_P1V26_VSEN,
    BMC_ATTR_ID_BMC_P1V538_VSEN,
    BMC_ATTR_ID_BMC_P3V3_VSEN,
    BMC_ATTR_ID_BMC_P5V_VSEN,
    BMC_ATTR_ID_P5V_SB_MON,
    BMC_ATTR_ID_P3V3_SB_MON,
    BMC_ATTR_ID_PERI_P3V3_MON,
    BMC_ATTR_ID_P3V3_MON,
    BMC_ATTR_ID_P2V5_MON,
    BMC_ATTR_ID_P1V2_MON,
    BMC_ATTR_ID_P0V9_MON,
    BMC_ATTR_ID_VDD_CORE_MON,
    BMC_ATTR_ID_PSU1_INT,
    BMC_ATTR_ID_PSU2_INT,
    BMC_ATTR_ID_PSU1_PWROK,
    BMC_ATTR_ID_PSU2_PWROK,
    BMC_ATTR_ID_PIN_PSU1,
    BMC_ATTR_ID_PIN_PSU2,
    BMC_ATTR_ID_POUT_PSU1,
    BMC_ATTR_ID_POUT_PSU2,
    BMC_ATTR_ID_VIN_PSU1,
    BMC_ATTR_ID_VIN_PSU2,
    BMC_ATTR_ID_VOUT_PSU1,
    BMC_ATTR_ID_VOUT_PSU2,
    BMC_ATTR_ID_IIN_PSU1,
    BMC_ATTR_ID_IIN_PSU2,
    BMC_ATTR_ID_IOUT_PSU1,
    BMC_ATTR_ID_IOUT_PSU2,
    BMC_ATTR_ID_LAST = BMC_ATTR_ID_IOUT_PSU2,
    BMC_ATTR_ID_INVALID,
};

/* SYS */
#define CPLD_REG                (0x33)
#define CPLD_VER_OFFSET         (0x01)

/* QSFP */
#define QSFP_PRES_REG1          (0x20)
#define QSFP_PRES_REG2          (0x21)
#define QSFP_PRES_OFFSET1       (0x00)
#define QSFP_PRES_OFFSET2       (0x01)

/* fru_id */
enum bmc_fru_id {
    BMC_FRU_IDX_ONLP_PSU_1,
    BMC_FRU_IDX_ONLP_PSU_2,

    BMC_FRU_IDX_INVALID = -1,
};

/* FAN */
#define FAN_GPIO_ADDR           (0x20)
#define FAN_1_2_PRESENT_MASK    (0x40)
#define FAN_3_4_PRESENT_MASK    (0x04)
#define FAN_5_6_PRESENT_MASK    (0x40)
#define FAN_7_8_PRESENT_MASK    (0x04)

#define MAX_SYS_FAN_RPM         (22000)

/* fan_id */
enum onlp_fan_id {
    ONLP_FAN_1_F = 1,
    ONLP_FAN_1_R,
    ONLP_FAN_2_F,
    ONLP_FAN_2_R,
    ONLP_FAN_3_F,
    ONLP_FAN_3_R,
    ONLP_FAN_4_F,
    ONLP_FAN_4_R,
    ONLP_PSU_1_FAN,
    ONLP_PSU_2_FAN,
    ONLP_FAN_MAX,
};

/* LED */
#define LED_REG                 (0x75)
#define LED_OFFSET              (0x02)
#define LED_PWOK_OFFSET         (0x03)

#define LED_SYS_AND_MASK        (0xFE)
#define LED_SYS_GMASK           (0x01)
#define LED_SYS_YMASK           (0x00)

#define LED_FAN_AND_MASK        (0xF9)
#define LED_FAN_OFF_OR_MASK     (0x00)
#define LED_FAN_GMASK           (0x02)
#define LED_FAN_YMASK           (0x06)

#define LED_PSU2_AND_MASK       (0xEF)
#define LED_PSU2_GMASK          (0x00)
#define LED_PSU2_YMASK          (0x10)
#define LED_PSU2_ON_AND_MASK    (0xFD)
#define LED_PSU2_ON_OR_MASK     (0x02)
#define LED_PSU2_OFF_AND_MASK   (0xFD)
#define LED_PSU2_OFF_OR_MASK    (0x00)
#define LED_PSU1_AND_MASK       (0xF7)
#define LED_PSU1_GMASK          (0x00)
#define LED_PSU1_YMASK          (0x08)
#define LED_PSU1_ON_AND_MASK    (0xFE)
#define LED_PSU1_ON_OR_MASK     (0x01)
#define LED_PSU1_OFF_AND_MASK   (0xFE)
#define LED_PSU1_OFF_OR_MASK    (0x00)
#define LED_SYS_ON_MASK         (0x00)
#define LED_SYS_OFF_MASK        (0x33)

/* led_id */
enum onlp_led_id {
    ONLP_LED_SYS_SYS = 1,
    ONLP_LED_SYS_FAN,
    ONLP_LED_SYS_PSU_1,
    ONLP_LED_SYS_PSU_2,
    ONLP_LED_FANTRAY_1,
    ONLP_LED_FANTRAY_2,
    ONLP_LED_FANTRAY_3,
    ONLP_LED_FANTRAY_4,
    ONLP_LED_MAX
};

/* PSU */
#define PSU_MUX_MASK            (0x01)

#define PSU_THERMAL1_OFFSET     (0x8D)
#define PSU_THERMAL2_OFFSET     (0x8E)
#define PSU_THERMAL_REG         (0x58)
#define PSU_FAN_RPM_REG         (0x58)
#define PSU_FAN_RPM_OFFSET      (0x90)
#define PSU_REG                 (0x58)
#define PSU_VOUT_OFFSET1        (0x20)
#define PSU_VOUT_OFFSET2        (0x8B)
#define PSU_IOUT_OFFSET         (0x8C)
#define PSU_POUT_OFFSET         (0x96)
#define PSU_PIN_OFFSET          (0x97)
#define PSU_VIN_OFFSET          (0x88)
#define PSU_IIN_OFFSET          (0x89)

#define PSU_STATE_REG           (0x25)
#define PSU1_PRESENT_OFFSET     (0x04)
#define PSU2_PRESENT_OFFSET     (0x01)
#define PSU1_PWGOOD_OFFSET      (0x03)
#define PSU2_PWGOOD_OFFSET      (0x00)

#define MAX_PSU_FAN_RPM         (18000)

/* psu_id */
enum onlp_psu_id {
    ONLP_PSU_1 = 1,
    ONLP_PSU_2,
    ONLP_PSU_MAX,
};

/* thermal_id */ 
enum onlp_bmc_thermal_id{
    // BMC
    ONLP_THERMAL_ID_Temp_BMC = 1,
    ONLP_THERMAL_ID_Temp_MAC,
    ONLP_THERMAL_ID_Temp_MAC_Front,
    ONLP_THERMAL_ID_Temp_MAC_Rear,
    ONLP_THERMAL_ID_Temp_Battery,
    ONLP_THERMAL_ID_Temp_OOB_conn,
    ONLP_THERMAL_ID_Temp_SFP_conn,
    ONLP_THERMAL_ID_Temp_PCIE_conn,
    ONLP_THERMAL_ID_Temp_PSU1_AMB,
    ONLP_THERMAL_ID_Temp_PSU2_AMB,
    ONLP_THERMAL_MAX_BMC,
};

/* thermal_id */
enum onlp_thermal_id {
    //sys
    ONLP_THERMAL_ID_FRONT_MAC = 1,
    ONLP_THERMAL_ID_ASIC,
    ONLP_THERMAL_ID_CPU1,
    ONLP_THERMAL_ID_CPU2,
    ONLP_THERMAL_ID_CPU3,
    ONLP_THERMAL_ID_CPU4,
    ONLP_THERMAL_ID_PSU1_1,
    ONLP_THERMAL_ID_PSU2_1,
    ONLP_THERMAL_ID_CPU_BOARD,
    ONLP_THERMAL_ID_PSU1_NEAR,
    ONLP_THERMAL_ID_PSU2_NEAR,
    ONLP_THERMAL_ID_MAC_REAR ,
    ONLP_THERMAL_ID_QSFP_NEAR,
    ONLP_THERMAL_MAX,
};

/* Shortcut for CPU thermal threshold value. */
#define THERMAL_THRESHOLD_INIT_DEFAULTS  \
    { THERMAL_WARNING_DEFAULT, \
        THERMAL_ERROR_DEFAULT,   \
        THERMAL_SHUTDOWN_DEFAULT }

/** onlp_psu_type */
typedef enum onlp_psu_type_e {
    ONLP_PSU_TYPE_AC,
    ONLP_PSU_TYPE_DC12,
    ONLP_PSU_TYPE_DC48,
    ONLP_PSU_TYPE_LAST = ONLP_PSU_TYPE_DC48,
    ONLP_PSU_TYPE_COUNT,
    ONLP_PSU_TYPE_INVALID = -1,
} onlp_psu_type_t;

typedef enum bmc_data_type_e {
    BMC_DATA_BOOL,
    BMC_DATA_FLOAT,
} bmc_data_type_t;

typedef enum brd_rev_id_e {
    BRD_PROTO,
    BRD_ALPHA,
    BRD_BETA,
    BRD_PVT,
} brd_rev_id_t;

enum hw_plat
{
    HW_PLAT_PROTO     = 0x1,
    HW_PLAT_ALPHA     = 0x2,
    HW_PLAT_BETA      = 0x4,
    HW_PLAT_PVT       = 0x8,
    HW_PLAT_ALL       = 0xf,
};

typedef struct bmc_info_s
{
    int plat;
    char name[20];
    int data_type;
    float data;
} bmc_info_t;

typedef struct bmc_fru_attr_s
{
    char key[BMC_FRU_ATTR_KEY_VALUE_SIZE];
    char val[BMC_FRU_ATTR_KEY_VALUE_SIZE];
}bmc_fru_attr_t;

typedef struct bmc_fru_s
{
    int bmc_fru_id;
    char init_done;
    char cache_files[BMC_FRU_ATTR_KEY_VALUE_SIZE];
    bmc_fru_attr_t vendor;
    bmc_fru_attr_t name;
    bmc_fru_attr_t part_num;
    bmc_fru_attr_t serial;
}bmc_fru_t;

typedef struct bmc_oem_data_s {
    char init_done;
    int raw_idx;
    int col_idx;
    int data;
} bmc_oem_data_t;

typedef struct psu_sup_info_s {
    char vendor[BMC_FRU_ATTR_KEY_VALUE_SIZE];
    char name[BMC_FRU_ATTR_KEY_VALUE_SIZE];
    int type;
} psu_support_info_t;

typedef struct board_s
{
    int hw_rev;
    int board_id;
    int hw_build;
    int model_id;
}board_t;

typedef struct temp_thld_s
{
    int warning;
    int error;
    int shutdown;
}temp_thld_t;

int exec_cmd(char *cmd, char* out, int size);

int read_file_hex(int* value, const char* fmt, ...);

int vread_file_hex(int* value, const char* fmt, va_list vargs);

void init_lock();

int check_bmc_alive(void);
int read_bmc_sensor(int bmc_cache_index, int sensor_type, float *data);
int read_bmc_fru(int fru_id, bmc_fru_t *data);

void check_and_do_i2c_mux_reset(int port);

uint8_t shift_bit(uint8_t mask);

uint8_t shift_bit_mask(uint8_t val, uint8_t mask);

uint8_t operate_bit(uint8_t reg_val, uint8_t bit, uint8_t bit_val);

uint8_t get_bit_value(uint8_t reg_val, uint8_t bit);

int get_hw_rev_id(void);

int get_psu_present_status(int local_id, int *pw_present);
//int get_psu_type(int local_id, int *psu_type, bmc_fru_t *fru_in);
int get_cpu_hw_rev_id(int *rev_id, int *dev_phase, int *build_id);
int get_board_version(board_t *board);
int get_thermal_thld(int thermal_local_id, temp_thld_t *temp_thld);
int get_gpio_max(int *gpio_max);

bool ufi_sysi_bmc_en_get(void);
extern bool bmc_enable;

#endif  /* __PLATFORM_LIB_H__ */
