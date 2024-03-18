/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
 *      ONLP System Platform Interface.
 *
 ***********************************************************/

/* This is definitions for x86-64-ufispace-s9180-32x*/
/* OID map*/
/*
 * [01] CHASSIS - 
 *            |----[01]ONLP_THERMAL_CPU_PKG
 *            |----[02]ONLP_THERMAL_MAC
 *            |----[03]ONLP_THERMAL_ENV_MACCASE
 *            |----[04]ONLP_THERMAL_ENV_SSDCASE
 *            |----[05]ONLP_THERMAL_ENV_PSUCASE
 *            |----[06]ONLP_THERMAL_ENV_BMC
 *            |
 *            |----[01] ONLP_LED_SYS_SYS
 *            |----[02] ONLP_LED_SYS_FAN
 *            |----[03] ONLP_LED_SYS_PSU_0
 *            |----[04] ONLP_LED_SYS_PSU_1
 *            |----[05] ONLP_LED_SYS_ID
 *            |
 *            |----[01] ONLP_PSU_0----[07] ONLP_THERMAL_PSU_0
 *                               |----[09] ONLP_PSU_0_FAN
 *            |----[02] ONLP_PSU_1----[08] ONLP_THERMAL_PSU_1
 *                               |----[10] ONLP_PSU_1_FAN
 *            |
 *            |----[01] ONLP_FAN_1_F
 *            |----[02] ONLP_FAN_1_R
 *            |----[03] ONLP_FAN_2_F
 *            |----[04] ONLP_FAN_2_R
 *            |----[05] ONLP_FAN_3_F
 *            |----[06] ONLP_FAN_3_R
 *            |----[07] ONLP_FAN_4_F
 *            |----[08] ONLP_FAN_4_R
 * 
 */
#include "platform_lib.h"


static onlp_oid_t __onlp_oid_adv_bmc_info[] = {
    /* BMC Thermal */
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_BMC),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC_Front),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC_Rear),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_Battery),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_OOB_conn),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_SFP_conn),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PCIE_conn),

    /* Others */
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_SYS),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_1),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_2),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_1),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_2),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_3),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_4),

    ONLP_PSU_ID_CREATE(ONLP_PSU_1),
    ONLP_PSU_ID_CREATE(ONLP_PSU_2),

    ONLP_FAN_ID_CREATE(ONLP_FAN_1_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_1_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4_R),
};

static onlp_oid_t __onlp_oid_std_info[] = {
    /* SYS Thermal */
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_FRONT_MAC),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_ASIC),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU1),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU2),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU3),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU4),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU_BOARD),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU1_NEAR),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU2_NEAR),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_MAC_REAR),
    ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_QSFP_NEAR),

    /* Others */
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_SYS),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_1),
    ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_2),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_1),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_2),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_3),
    ONLP_LED_ID_CREATE(ONLP_LED_FANTRAY_4),

    ONLP_PSU_ID_CREATE(ONLP_PSU_1),
    ONLP_PSU_ID_CREATE(ONLP_PSU_2),

    ONLP_FAN_ID_CREATE(ONLP_FAN_1_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_1_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_2_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_3_R),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4_F),
    ONLP_FAN_ID_CREATE(ONLP_FAN_4_R),
};

#define SYS_EEPROM_SIZE     512
#define SYSFS_CPLD1_VER_H   SYSFS_CPLD1 "cpld_model_id"//"cpld_version_h"
#define SYSFS_BIOS_VER      "/sys/class/dmi/id/bios_version"

#define CMD_BMC_VER_1       "expr `ipmitool mc info"IPMITOOL_REDIRECT_FIRST_ERR" | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f1` + 0"
#define CMD_BMC_VER_2       "expr `ipmitool mc info"IPMITOOL_REDIRECT_ERR" | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f2` + 0"
#define CMD_BMC_VER_3       "echo $((`ipmitool mc info"IPMITOOL_REDIRECT_ERR" | grep 'Aux Firmware Rev Info' -A 2 | sed -n '2p'` + 0))"

/* ufispace APIs */
static int get_platform_info(onlp_platform_info_t* pi);
static int get_sysi_platform_info(onlp_platform_info_t* info);
static int get_platform_thermal_temp(int* thermal_temp);
static int decide_fan_percentage(int is_up, int new_temp);

bool bmc_enable =false;

/**
 * @brief Initialize the system platform subsystem.
 */
int onlp_sysi_init(void)
{
    /* check if s9180 platform as advanced or standard (whether there is BMC) */
    if(ufi_sysi_bmc_en_get()) {
        bmc_enable = true;
    }
    else {
        bmc_enable = false;
    }
    
    return ONLP_STATUS_OK;
}

/**
 * @brief Return the raw contents of the ONIE system eeprom.
 * @param data [out] Receives the data pointer to the ONIE data.
 * @param size [out] Receives the size of the data (if available).
 * @notes This function is only necessary if you cannot provide
 * the physical base address as per onlp_sysi_onie_data_phys_addr_get().
 */
int onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(SYS_EEPROM_SIZE);
    if(onlp_file_read(rdata, SYS_EEPROM_SIZE, size, SYS_EEPROM_PATH) == ONLP_STATUS_OK) {
        if(*size == SYS_EEPROM_SIZE) {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }

    AIM_LOG_INFO("Unable to get data from eeprom \n");
    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}

/**
 * @brief Free the data returned by onlp_sys_onie_data_get()
 * @param data The data pointer.
 * @notes If onlp_sysi_onie_data_get() is called to retreive the
 * contents of the ONIE system eeprom then this function
 * will be called to perform any cleanup that may be necessary
 * after the data has been used.
 */
void onlp_sysi_onie_data_free(uint8_t* data)
{
    if (data) {
        aim_free(data);
    }
}

/**
 * @brief Return the ONIE system information for this platform.
 * @param onie The onie information structure.
 * @notes If all previous attempts to get the eeprom data fail
 * then this routine will be called. Used as a translation option
 * for platforms without access to an ONIE-formatted eeprom.
 */
int onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/**
 * @brief This function returns the root oid list for the platform.
 * @param table [out] Receives the table.
 * @param max The maximum number of entries you can fill.
 */
int onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    memset(table, 0, max*sizeof(onlp_oid_t));

    if(bmc_enable) {
        memset(table, 0, max*sizeof(onlp_oid_t));
        memcpy(table, __onlp_oid_adv_bmc_info, sizeof(__onlp_oid_adv_bmc_info));
    } 
    else {
        memset(table, 0, max*sizeof(onlp_oid_t));
        memcpy(table, __onlp_oid_std_info, sizeof(__onlp_oid_std_info));
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Perform necessary platform fan management.
 * @note This function should automatically adjust the FAN speeds
 * according to the platform conditions.
 */
int onlp_sysi_platform_manage_fans(void)
{
    if(bmc_enable) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int rc, is_up ,new_temp, thermal_temp, diff;
    static int new_perc = 0, ori_perc = 0;
    static int ori_temp = 0;
    onlp_thermal_info_t thermal_info;
    memset(&thermal_info, 0, sizeof(thermal_info));
    
    /* get new temperature */
    if ((rc = get_platform_thermal_temp(&thermal_temp)) != ONLP_STATUS_OK) {
        goto _EXIT;
    }

    new_temp = thermal_temp;
    diff = new_temp - ori_temp;

    if (diff == 0) {
        goto _EXIT;
    } 
    else {
        is_up = (diff > 0 ? 1 : 0);    
    }
    
    new_perc = decide_fan_percentage(is_up, new_temp);
    
    if (ori_perc == new_perc) {
        goto _EXIT;
    }

    AIM_LOG_INFO("The Fan Speeds Percent are now at %d%%", new_perc);

    if ((rc = onlp_fani_percentage_set(ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_ASIC), new_perc)) != ONLP_STATUS_OK) {
        goto _EXIT;
    }
            
    /* update */
    ori_perc = new_perc;
    ori_temp = new_temp;

    _EXIT :
    return rc;
}

/**
 * @brief Perform necessary platform LED management.
 * @note This function should automatically adjust the LED indicators
 * according to the platform conditions.
 * Not support in s9180-adv (due to it's controlled by BMC)
 */
int onlp_sysi_platform_manage_leds(void)
{
    if(bmc_enable) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
    
    int psu1_status, psu2_status, rc, i;
    static int pre_psu1_status = 0, pre_psu2_status = 0;
    
    //-------------------------------
    int fan_tray_id = 0;
    static int fantray_present = 0;  /* fan tray present number */
    //-------------------------------
    
    onlp_psu_info_t psu_info;
    onlp_fan_info_t fan_info;

    //-------------------------------

    memset(&psu_info, 0, sizeof(onlp_psu_info_t));   
    memset(&fan_info, 0, sizeof(onlp_fan_info_t));
    uint32_t fan_arr[] = { ONLP_FAN_ID_CREATE(ONLP_FAN_1_F),
                            ONLP_FAN_ID_CREATE(ONLP_FAN_1_R),      
                            ONLP_FAN_ID_CREATE(ONLP_FAN_2_F),
                            ONLP_FAN_ID_CREATE(ONLP_FAN_2_R),
                            ONLP_FAN_ID_CREATE(ONLP_FAN_3_F),
                            ONLP_FAN_ID_CREATE(ONLP_FAN_3_R),
                            ONLP_FAN_ID_CREATE(ONLP_FAN_4_F), 
                            ONLP_FAN_ID_CREATE(ONLP_FAN_4_R),  };

    /* PSU LED CTRL */
    if ((rc = onlp_psui_info_get(ONLP_PSU_ID_CREATE(ONLP_PSU_1), &psu_info)) != ONLP_STATUS_OK) {
        goto _EXIT;
    }

    psu1_status = psu_info.status;
    if (psu1_status != pre_psu1_status) {
        /* psu absent */
        if((psu1_status & ONLP_PSU_STATUS_PRESENT) == 0) {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_1), ONLP_LED_MODE_OFF);
        }
        /* psu present、power not ok */
        else if(psu1_status != ONLP_PSU_STATUS_PRESENT) {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_1), ONLP_LED_MODE_YELLOW);
        }
        /* power ok */ 
        else {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_1), ONLP_LED_MODE_GREEN);
        }

        if (rc != ONLP_STATUS_OK) {
            goto _EXIT;
        }
        pre_psu1_status = psu1_status;
    }

    if ((rc = onlp_psui_info_get(ONLP_PSU_ID_CREATE(ONLP_PSU_2), &psu_info)) != ONLP_STATUS_OK) {
        goto _EXIT;
    }

    psu2_status = psu_info.status;
    if( psu2_status != pre_psu2_status) {
        /* psu absent */
        if((psu2_status & ONLP_PSU_STATUS_PRESENT) == 0) {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_2), ONLP_LED_MODE_OFF);
        }
        /* psu present、power not ok */
        else if(psu2_status != ONLP_PSU_STATUS_PRESENT) {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_2), ONLP_LED_MODE_YELLOW);
        } 
        /* power ok */
        else {
            rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_PSU_2), ONLP_LED_MODE_GREEN);
        }

        if (rc != ONLP_STATUS_OK) {
            goto _EXIT;
        }
        pre_psu2_status = psu2_status;
    }
    
    /* FAN LED CTRL */
    for (i=0; i<SYS_FAN_NUM; i++) {
        if (i%2 == 1) {
            
            switch (i) {
                case 1:
                    fan_tray_id = ONLP_LED_FANTRAY_1;
                    break;
                case 3:
                    fan_tray_id = ONLP_LED_FANTRAY_2;
                    break;
                case 5:
                    fan_tray_id = ONLP_LED_FANTRAY_3;
                    break;
                case 7:
                    fan_tray_id = ONLP_LED_FANTRAY_4;
                    break;
            }
            
            /* Front FAN */
            if ((rc = onlp_fani_info_get(fan_arr[i], &fan_info)) != ONLP_STATUS_OK) {
                goto _EXIT;
            }

            /* Front FAN Present */
            if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 1) {
                /* Rear FAN */
                if ((rc = onlp_fani_info_get(fan_arr[i-1], &fan_info)) != ONLP_STATUS_OK) {
                    goto _EXIT;
                }

                /* Rear FAN Present -> Fan Tray Present */
                if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 1) {
                    rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(fan_tray_id), ONLP_LED_MODE_GREEN);
                    fantray_present += 1;
                }
                else {
                    goto _EXIT;
                }
            }
            else {
                rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(fan_tray_id), ONLP_LED_MODE_OFF);
            }
        }
    }

    /* Front Panel FAN Status */
    /* All Fan Tray is present */
    if (fantray_present == FAN_TRAY_NUM) {
        rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN), ONLP_LED_MODE_GREEN);
    }
    /* All Fan Tray is not present */
    else if (fantray_present == 0) {
        rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN), ONLP_LED_MODE_OFF);
    }
     /* Some Fan Tray present while some don't */
    else if ((fantray_present < FAN_TRAY_NUM) && (fantray_present > 0)) {
        rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN), ONLP_LED_MODE_YELLOW);
    }
    else {
        rc = onlp_ledi_mode_set(ONLP_LED_ID_CREATE(ONLP_LED_SYS_FAN), ONLP_LED_MODE_OFF);
        goto _EXIT;
    }

    if (rc != ONLP_STATUS_OK) {
        goto _EXIT;
    }

_EXIT :
    fantray_present = 0;
    return rc;
}

/**
 * @brief Return custom platform information.
 */
int onlp_sysi_platform_info_get(onlp_platform_info_t* info)
{
    // Clean info 
    memset(info, 0, sizeof(onlp_platform_info_t));
    
    if ( bmc_enable ){
        if (get_platform_info(info) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    } 
    else {
        int rc;
        if ((rc = get_sysi_platform_info(info)) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }

    return ONLP_STATUS_OK;
    
}

/**
 * @brief Friee a custom platform information structure.
 * Both std and adv platform are the same.
 */
void onlp_sysi_platform_info_free(onlp_platform_info_t* info)
{
    if (info && info->cpld_versions) {
        aim_free(info->cpld_versions);
    }

    if (info && info->other_versions) {
        aim_free(info->other_versions);
    }
}

/**
 * @brief Return the name of the the platform implementation.
 * @notes This will be called PRIOR to any other calls into the
 * platform driver, including the sysi_init() function below.
 *
 * The platform implementation name should match the current
 * ONLP platform name.
 *
 * IF the platform implementation name equals the current platform name,
 * initialization will continue.
 *
 * If the platform implementation name does not match, the following will be
 * attempted:
 *
 *    onlp_sysi_platform_set(current_platform_name);
 * If this call is successful, initialization will continue.
 * If this call fails, platform initialization will abort().
 *
 * The onlp_sysi_platform_set() function is optional.
 * The onlp_sysi_platform_get() is not optional.
 */
const char* onlp_sysi_platform_get(void)
{
    return "x86-64-ufispace-s9180-32x-r0";
}

static int get_platform_info(onlp_platform_info_t* pi)
{
    int len = 0;
    char bios_out[ONLP_CONFIG_INFO_STR_MAX] = {'\0'};
    char bmc_out1[8] = {0}, bmc_out2[8] = {0}, bmc_out3[8] = {0};

    /* get MB CPLD version */
    char mb_cpld1_ver[ONLP_CONFIG_INFO_STR_MAX] = {'\0'};
    ONLP_TRY(onlp_file_read((uint8_t*)&mb_cpld1_ver, ONLP_CONFIG_INFO_STR_MAX -1, &len, SYSFS_CPLD1_VER_H));

    pi->cpld_versions = aim_fstrdup(
        "\n"
        "[MB CPLD1] %s\n",
        mb_cpld1_ver);

    /* Get BIOS version */
    char tmp_str[ONLP_CONFIG_INFO_STR_MAX] = {'\0'};
    ONLP_TRY(onlp_file_read((uint8_t*)&tmp_str, ONLP_CONFIG_INFO_STR_MAX - 1, &len, SYSFS_BIOS_VER));

    /* Remove '\n' */
    sscanf (tmp_str, "%[^\n]", bios_out);

    /* Detect bmc status */
    if(check_bmc_alive() != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Timeout, BMC did not respond.\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get BMC version */
    if (exec_cmd(CMD_BMC_VER_1, bmc_out1, sizeof(bmc_out1)) < 0 ||
        exec_cmd(CMD_BMC_VER_2, bmc_out2, sizeof(bmc_out2)) < 0 ||
        exec_cmd(CMD_BMC_VER_3, bmc_out3, sizeof(bmc_out3))) {
            AIM_LOG_ERROR("unable to read BMC version\n");
            return ONLP_STATUS_E_INTERNAL;
    }

    pi->other_versions = aim_fstrdup(
        "\n"
        "[BIOS ] %s\n"
        "[BMC  ] %d.%d.%d\n",
        bios_out,
        atoi(bmc_out1), atoi(bmc_out2), atoi(bmc_out3));

    return ONLP_STATUS_OK;
}

static int get_sysi_platform_info(onlp_platform_info_t* info)
{
    int cpld_release, cpld_version, cpld_rev;

    cpld_rev = onlp_i2c_readb(I2C_BUS_44, CPLD_REG, CPLD_VER_OFFSET, ONLP_I2C_F_FORCE);
    if (cpld_rev < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    cpld_release = (((cpld_rev) >> 6 & 0x01));
    cpld_version = (((cpld_rev) & 0x3F));

    info->cpld_versions = aim_fstrdup(
                "CPLD is %d version(0:RD 1:Release), Revision is 0x%02x\n",
                cpld_release, cpld_version);

    return ONLP_STATUS_OK;
}

/* This is for standard platfrom */
static int get_platform_thermal_temp(int* thermal_temp){
    int i, temp, max_temp, rc;
    onlp_thermal_info_t thermal_info;
    memset(&thermal_info, 0, sizeof(thermal_info));

    uint32_t thermal_arr[] = {  ONLP_THERMAL_ID_FRONT_MAC,
                                ONLP_THERMAL_ID_ASIC,
                                ONLP_THERMAL_ID_CPU1, 
                                ONLP_THERMAL_ID_CPU2, 
                                ONLP_THERMAL_ID_CPU3, 
                                ONLP_THERMAL_ID_CPU4, };

    max_temp = 0;

    for (i=0; i<STD_BOARD_THERMAL_NUM; i++) {   
        
        if ((rc = onlp_thermali_info_get(thermal_arr[i], &thermal_info)) != ONLP_STATUS_OK) {
            return rc;
        }
        
        temp = thermal_info.mcelsius;
        if (temp > max_temp) {
            max_temp = temp;
        }
    }
    *thermal_temp = max_temp;

    return ONLP_STATUS_OK;
}

static int decide_fan_percentage(int is_up, int new_temp)
{
    int new_perc;
    if (is_up) {
        if (new_temp >= THERMAL_ERROR_DEFAULT) {
            new_perc = THERMAL_ERROR_FAN_PERC;
        } else if (new_temp >= THERMAL_WARNING_DEFAULT) {
            new_perc = THERMAL_WARNING_FAN_PERC;
        } else {
            new_perc = THERMAL_NORMAL_FAN_PERC;
        }
    } else {
        if (new_temp <= THERMAL_NORMAL_DEFAULT) {
            new_perc = THERMAL_NORMAL_FAN_PERC;
        } else if (new_temp <= THERMAL_WARNING_DEFAULT) {
            new_perc = THERMAL_WARNING_FAN_PERC;
        } else {
            new_perc = THERMAL_ERROR_FAN_PERC;
        }
    }

    return new_perc;
}