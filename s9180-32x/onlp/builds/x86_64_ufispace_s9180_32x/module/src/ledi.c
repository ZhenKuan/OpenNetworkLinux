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
 *      LED Platform Implementation.
 *
 ***********************************************************/
#include "platform_lib.h"

#define MAX_OUTPUT_SIZE 256
#define IPMITOOL_RAW_CMD_SYS_LED "ipmitool raw 0x3c 0x20 0x0"  //bmc oem command
#define IPMITOOL_RAW_CMD_FANTRAY_LED "ipmitool raw 0x3c 0x21 0x0"  //bmc oem command
#define LED_STATUS ONLP_LED_STATUS_PRESENT
#define LED_CAPS   ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING | \
                    ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING
#define ID_LED_CAPS   ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING
#define FLEXE_LED_CAPS ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_GREEN

#define CHASSIS_LED_INFO(id, desc)               \
    {                                            \
        { ONLP_LED_ID_CREATE(id), desc, POID_0},\
        LED_STATUS,                              \
        LED_CAPS,                                \
    }

#define CHASSIS_FLEXE_LED_INFO(id, desc)               \
    {                                            \
        { ONLP_LED_ID_CREATE(id), desc, POID_0},\
        LED_STATUS,                              \
        FLEXE_LED_CAPS,                                \
    }

#define CHASSIS_ID_LED_INFO(id, desc)               \
    {                                            \
        { ONLP_LED_ID_CREATE(id), desc, POID_0},\
        LED_STATUS,                              \
        ID_LED_CAPS,                                \
    }

/* ufispace APIs */
// Extern ufispace APIs
extern int get_std_fan_info(int id, onlp_fan_info_t* info);
// s9180-32x-adv/std
static int get_led_local_id(int id, int *local_id);
// s9180-32x-adv
static int get_adv_system_led(int local_id, onlp_led_info_t* info);
// s9180-32x-std
static int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask);
static int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask);
static int get_std_ledi_info(int local_id, onlp_led_info_t* info);
static int set_std_system_led(onlp_led_mode_t mode);
static int set_std_fan_led(onlp_led_mode_t mode);
static int set_std_psu_led(int led_id, onlp_led_mode_t mode);
static int set_std_fan_tray_led(onlp_oid_t id, onlp_led_mode_t mode);

typedef struct 
{
    char *ipmi_cmd;
    int index;
} bmc_oem_led_info;

static bmc_oem_led_info led_ipmitool_raw_command[] = {
   //CMD                      INDEX
    {/*not used*/},  
    {IPMITOOL_RAW_CMD_SYS_LED       ,0},         //SYS_LED
    {IPMITOOL_RAW_CMD_SYS_LED       ,1},         //FAN_LED
    {IPMITOOL_RAW_CMD_SYS_LED       ,2},         //PSU1_LED
    {IPMITOOL_RAW_CMD_SYS_LED       ,3},         //PSU2_LED
    {IPMITOOL_RAW_CMD_FANTRAY_LED   ,0},         //FANTRAY1_LED
    {IPMITOOL_RAW_CMD_FANTRAY_LED   ,1},         //FANTRAY2_LED
    {IPMITOOL_RAW_CMD_FANTRAY_LED   ,2},         //FANTRAY3_LED
    {IPMITOOL_RAW_CMD_FANTRAY_LED   ,3},         //FANTRAY4_LED
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t led_info[] = {
    {/*not used*/},
    CHASSIS_LED_INFO(ONLP_LED_SYS_SYS       ,"CHASSIS LED 1 (SYS LED)"),
    CHASSIS_LED_INFO(ONLP_LED_SYS_FAN       ,"CHASSIS LED 2 (FAN LED)"),
    CHASSIS_LED_INFO(ONLP_LED_SYS_PSU_1     ,"CHASSIS LED 3 (PSU 1 LED)"),
    CHASSIS_LED_INFO(ONLP_LED_SYS_PSU_2     ,"CHASSIS LED 4 (PSU 2 LED)"),
    CHASSIS_LED_INFO(ONLP_LED_FANTRAY_1     ,"REAR LED 1 (FANTRAY 1 LED)"),
    CHASSIS_LED_INFO(ONLP_LED_FANTRAY_2     ,"REAR LED 2 (FANTRAY 2 LED)"),
    CHASSIS_LED_INFO(ONLP_LED_FANTRAY_3     ,"REAR LED 3 (FANTRAY 3 LED)"),
    CHASSIS_LED_INFO(ONLP_LED_FANTRAY_4     ,"REAR LED 4 (FANTRAY 4 LED)"),
};

typedef enum led_act_e {
    ACTION_LED_RO = 0,
    ACTION_LED_RW,
    ACTION_LED_ATTR_MAX,
} led_act_t;

typedef struct {
    led_act_t action;
    int attr;
    int color_bit;
    int blink_bit;
    int onoff_bit;
} led_attr_t;

typedef enum cpld_attr_idx_e {
    CPLD_LED_SYS = 0,
    CPLD_LED_FAN,
    CPLD_LED_PSU,
    CPLD_LED_ID,
    CPLD_NONE,
} cpld_attr_idx_t;

static const led_attr_t led_attr[] = {
/*  led attribute            action              attr             color   blink  onoff */
    [ONLP_LED_SYS_SYS]    = {ACTION_LED_RW      ,CPLD_LED_SYS     ,1      ,0     ,1},
    [ONLP_LED_SYS_FAN]    = {ACTION_LED_RW      ,CPLD_LED_FAN     ,0      ,0     ,1},
    [ONLP_LED_SYS_PSU_1]  = {ACTION_LED_RW      ,CPLD_LED_PSU     ,-1     ,0     ,1},
    [ONLP_LED_SYS_PSU_2]  = {ACTION_LED_RW      ,CPLD_LED_PSU     ,-1     ,0     ,1},
    [ONLP_LED_FANTRAY_1]  = {ACTION_LED_RW      ,CPLD_LED_FAN     ,0      ,0     ,1},
    [ONLP_LED_FANTRAY_2]  = {ACTION_LED_RW      ,CPLD_LED_FAN     ,0      ,0     ,1},
    [ONLP_LED_FANTRAY_3]  = {ACTION_LED_RW      ,CPLD_LED_FAN     ,0      ,0     ,1},
    [ONLP_LED_FANTRAY_4]  = {ACTION_LED_RW      ,CPLD_LED_FAN     ,0      ,0     ,1},
};

uint32_t fan_arr[] = { ONLP_FAN_ID_CREATE(ONLP_FAN_1_F),
                        ONLP_FAN_ID_CREATE(ONLP_FAN_1_R),      
                        ONLP_FAN_ID_CREATE(ONLP_FAN_2_F),
                        ONLP_FAN_ID_CREATE(ONLP_FAN_2_R),
                        ONLP_FAN_ID_CREATE(ONLP_FAN_3_F),
                        ONLP_FAN_ID_CREATE(ONLP_FAN_3_R),
                        ONLP_FAN_ID_CREATE(ONLP_FAN_4_F), 
                        ONLP_FAN_ID_CREATE(ONLP_FAN_4_R),  };

/**
 * @brief Initialize the LED subsystem.
 */
int onlp_ledi_init(void)
{
    init_lock();
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information for the given LED
 * @param id The LED OID
 * @param rv [out] Receives the LED information.
 */
int onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* rv)
{
    int local_id;

    ONLP_TRY(get_led_local_id(id, &local_id));

    // Clean rv 
    memset(rv, 0, sizeof(onlp_led_info_t));

    if (bmc_enable) {
        ONLP_TRY(get_adv_system_led(local_id, rv));
    }
    else {
        ONLP_TRY(get_std_ledi_info(local_id, rv));
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the LED operational status.
 * @param id The LED OID
 * @param rv [out] Receives the operational status.
 */
int onlp_ledi_status_get(onlp_oid_t id, uint32_t* rv)
{
    int local_id;

    ONLP_TRY(get_led_local_id(id, &local_id));

    *rv = led_info[local_id].status;

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the LED header.
 * @param id The LED OID
 * @param rv [out] Receives the header.
 */
int onlp_ledi_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    int local_id;

    ONLP_TRY(get_led_local_id(id, &local_id));
    *rv = led_info[local_id].hdr;

    return ONLP_STATUS_OK;
}

/**
 * @brief Turn an LED on or off
 * @param id The LED OID
 * @param on_or_off (boolean) on if 1 off if 0
 * @param This function is only relevant if the ONOFF capability is set.
 * @notes See onlp_led_set() for a description of the default behavior.
 */
int onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    if (bmc_enable) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    int local_id;

    ONLP_TRY(get_led_local_id(id, &local_id));
    if (led_attr[local_id].action != ACTION_LED_RW) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (on_or_off) {
        return ONLP_STATUS_E_UNSUPPORTED;
    } 
    else {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }
}

/**
 * @brief Set the LED mode.
 * @param id The LED OID
 * @param mode The new mode.
 * @notes Only called if the mode is advertised in the LED capabilities.
 */
int onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    int led_id, rc;

    led_id = ONLP_OID_ID_GET(id);

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    switch (led_id) {
        case ONLP_LED_SYS_SYS:
            rc = set_std_system_led(mode);
            break;
        case ONLP_LED_SYS_FAN:
            rc = set_std_fan_led(mode);
            break;
        case ONLP_LED_SYS_PSU_1:
        case ONLP_LED_SYS_PSU_2:
            rc = set_std_psu_led(led_id, mode);
            break;
        //case ONLP_LED_SYS_PSU_2:
        //    rc = set_std_psu2_led(mode);
        //    break;
        case ONLP_LED_FANTRAY_1:
        case ONLP_LED_FANTRAY_2:
        case ONLP_LED_FANTRAY_3:
        case ONLP_LED_FANTRAY_4:
            rc = set_std_fan_tray_led(id, mode);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}

static int get_adv_system_led(int local_id, onlp_led_info_t* info) {
    int color_raw = 0;
    char cmd_output[MAX_OUTPUT_SIZE];

    // Get LED Info
    *info = led_info[local_id];

    if (local_id == sizeof(led_ipmitool_raw_command) / sizeof(led_ipmitool_raw_command[0])) {
        return ONLP_STATUS_E_INVALID;
    }

    // Get the ipmitool raw command output
    FILE* fp = popen(led_ipmitool_raw_command[local_id].ipmi_cmd, "r");
    if (fp == NULL) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (fgets(cmd_output, sizeof(cmd_output), fp) == NULL) {
        pclose(fp);
        return ONLP_STATUS_E_INVALID;
    }

    pclose(fp);

    // Decode the ipmitool raw command oputput for led color
    for (int i =0; i<=led_ipmitool_raw_command[local_id].index; i++) {
        if(i == 0) {
            color_raw = atoi(strtok(cmd_output, " "));
        }
        else {
            color_raw = atoi(strtok(NULL, " "));
        }
    }

    // Assuming that the LED modes and statuses are set based on the color
    if(color_raw == 2 /*Green*/) {
        info->status |= ONLP_LED_STATUS_ON;
        info->mode |= ONLP_LED_MODE_GREEN;
    } 
    else if(color_raw == 1/*Yellow*/) {
        info->status |= ONLP_LED_STATUS_ON;
        info->mode |= ONLP_LED_MODE_YELLOW;
    } 
    else if(color_raw == 0/*OFF*/) {
        info->status |= ONLP_LED_STATUS_PRESENT;
        info->mode |= ONLP_LED_MODE_OFF;
    } 
    else {
        return ONLP_STATUS_E_INVALID;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Get and check led local ID
 * @param id [in] OID
 * @param local_id [out] The led local id
 */
static int get_led_local_id(int id, int *local_id)
{
    int tmp_id;

    if(local_id == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    if(!ONLP_OID_IS_LED(id)) {
        return ONLP_STATUS_E_INVALID;
    }

    tmp_id = ONLP_OID_ID_GET(id);
    switch (tmp_id) {
        case ONLP_LED_SYS_SYS:
        case ONLP_LED_SYS_FAN:
        case ONLP_LED_SYS_PSU_1:
        case ONLP_LED_SYS_PSU_2:
        case ONLP_LED_FANTRAY_1:
        case ONLP_LED_FANTRAY_2:
        case ONLP_LED_FANTRAY_3:
        case ONLP_LED_FANTRAY_4:
            *local_id = tmp_id;
            return ONLP_STATUS_OK;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    return ONLP_STATUS_E_INVALID;
}

/**
 * @brief Get psu present
 * @param pw_exist
 * @param exist_offset
 * @param i2c_bus
 * @param psu_mask
 */
static int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask) 
{
    int psu_pres;

    psu_pres = onlp_i2c_readb(i2c_bus, PSU_STATE_REG, 0x0,
                                ONLP_I2C_F_FORCE);
    if (psu_pres < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    *pw_exist = (((psu_pres >> exist_offset) & psu_mask) ? 0 : 1);
    return ONLP_STATUS_OK;
}

/**
 * @brief Get psu pwgood
 * @param pw_good
 * @param good_offset
 * @param i2c_bus
 * @param psu_mask
 */
static int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask) 
{
    int psu_pwgood;

    psu_pwgood = onlp_i2c_readb(i2c_bus, PSU_STATE_REG, 0x0,
                                ONLP_I2C_F_FORCE);
    if (psu_pwgood < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    *pw_good = (((psu_pwgood >> good_offset) & psu_mask) ? 1 : 0);
    return ONLP_STATUS_OK;
}

static int get_std_ledi_info(int local_id, onlp_led_info_t* info) {
    int pw_exist, pw_good, rc, psu_mask, fan_f_id, fan_r_id;
    int exist_offset, good_offset, i2c_bus;
    int value;
    int i, fail_fan_count = 0;
    onlp_fan_info_t fan_info;

    memset(&fan_info, 0, sizeof(onlp_fan_info_t));

    *info = led_info[local_id];
    if((info->status & ONLP_LED_STATUS_PRESENT) == 0) {
            return ONLP_STATUS_OK;
    }

    /* Front Panel LED SYS status */ 
    if (local_id == ONLP_LED_SYS_SYS) {
        value = onlp_i2c_readb(I2C_BUS_50, LED_REG, LED_OFFSET, ONLP_I2C_F_FORCE);
        if (value < 0) {
		    return ONLP_STATUS_E_INTERNAL;
	    }

        //printf("[Melo!] sys green led value = %d\n", (value &= LED_SYS_GMASK));
        if ((value &= LED_SYS_GMASK) == 1) {
            info->status |= ONLP_LED_STATUS_ON;
            info->mode |= ONLP_LED_MODE_GREEN;
        }
        else {
            info->status |= ONLP_LED_MODE_OFF;
            info->mode &= ~ONLP_LED_MODE_GREEN;
        }       
    } 
    /* Front Panel LED PSU status */
    else if (local_id == ONLP_LED_SYS_PSU_1 || local_id == ONLP_LED_SYS_PSU_2) {

        psu_mask = PSU_MUX_MASK;

        if (local_id == ONLP_LED_SYS_PSU_1) {
            i2c_bus = I2C_BUS_PSU1;
            exist_offset = PSU1_PRESENT_OFFSET;
            good_offset = PSU1_PWGOOD_OFFSET;
        } 
        else {
            i2c_bus = I2C_BUS_PSU2;
            exist_offset = PSU2_PRESENT_OFFSET;
            good_offset = PSU2_PWGOOD_OFFSET;
        }

        /* get psu present and pwgood status */
        if ((rc = get_std_psu_present(&pw_exist, exist_offset, i2c_bus, psu_mask)) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }
        if ((rc = get_std_psu_pwgood(&pw_good, good_offset, i2c_bus, psu_mask)) != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }

        /* psu not present */
        if (pw_exist != PSU_STATUS_PRSNT) {
            info->status &= ~ONLP_LED_STATUS_ON;
            info->mode = ONLP_LED_MODE_OFF;
        } 
        /* psu present but not power good */
        else if (pw_good != PSU_STATUS_POWER_GOOD) {
            info->status |= ONLP_LED_STATUS_ON;
            info->mode |= ONLP_LED_MODE_YELLOW;
        } 
        else {
            info->status |= ONLP_LED_STATUS_ON;
            info->mode |= ONLP_LED_MODE_GREEN;
        }
    } 
    /* Front Panel LED FAN status */
    else if (local_id == ONLP_LED_SYS_FAN) {
        /* FAN LED CTRL */
        for (i=0; i<SYS_FAN_NUM; i++) {
            if (i%2 == 1) {
                /* Get Front FAN Info */
                if ((rc = onlp_fani_info_get(fan_arr[i], &fan_info)) != ONLP_STATUS_OK) {
                    return ONLP_STATUS_E_INTERNAL;
                }

                /* Front FAN Present */
                if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 1) {
                    /* Get Rear FAN Info */
                    if ((rc = onlp_fani_info_get(fan_arr[i-1], &fan_info)) != ONLP_STATUS_OK) {
                        return ONLP_STATUS_E_INTERNAL;
                    }

                    /* Rear FAN Present -> FanTray Present */
                    if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) != 1) {
                        return ONLP_STATUS_E_INTERNAL;
                    }
                }
                /* Fan Tray Not Present */
                else {
                    /* LED off */
                    fail_fan_count ++;
                }
            }
        }

        /* Front Panel FAN Status */
        /* All Fan Tray is present */
        if (fail_fan_count == 0) {
            info->status |= ONLP_LED_STATUS_ON;
            info->mode |= ONLP_LED_MODE_GREEN;
        }
        /* Some Fan Tray present while some don't */
        else if ((fail_fan_count < FAN_TRAY_NUM) && (fail_fan_count > 0)) {
            info->mode &= ~ONLP_LED_MODE_GREEN;
            info->mode |= ONLP_LED_MODE_YELLOW;
        }
        /* All Fan Tray is not present */
        else if (fail_fan_count == FAN_TRAY_NUM) {
            info->mode &= ~ONLP_LED_MODE_YELLOW;
            info->mode |= ONLP_LED_MODE_OFF;
        }
        else {
            return ONLP_STATUS_E_INTERNAL;
        }
    } 
    /* Rear Panel FAN TRAY LED status */
    else if (local_id == ONLP_LED_FANTRAY_1 || local_id == ONLP_LED_FANTRAY_2 || local_id == ONLP_LED_FANTRAY_3 || local_id == ONLP_LED_FANTRAY_4) {
        switch(local_id) {
            case ONLP_LED_FANTRAY_1:
                fan_f_id = 0;
                fan_r_id = 1;
                break;
            case ONLP_LED_FANTRAY_2:
                fan_f_id = 2;
                fan_r_id = 3;
                break;
            case ONLP_LED_FANTRAY_3:
                fan_f_id = 4;
                fan_r_id = 5;
                break;
            case ONLP_LED_FANTRAY_4:
                fan_f_id = 6;
                fan_r_id = 7;
                break;
            default:
                return ONLP_STATUS_E_INVALID;
        }

        /* Get Front FAN Info */
        if ((rc = onlp_fani_info_get(fan_arr[fan_f_id], &fan_info)) != ONLP_STATUS_OK) {
                    return ONLP_STATUS_E_INTERNAL;
        }

        if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 1) {
            /* Clean fan_info */
            memset(&fan_info, 0, sizeof(onlp_fan_info_t));

            /* Get Rear FAN Info */
            if ((rc = onlp_fani_info_get(fan_arr[fan_r_id], &fan_info)) != ONLP_STATUS_OK) {
                return ONLP_STATUS_E_INTERNAL;
            }

            /* Rear FAN Present -> Fan Tray Present */
            if ((fan_info.status & ONLP_FAN_STATUS_PRESENT) == 1) {
                info->status |= ONLP_LED_STATUS_ON;
                info->mode |= ONLP_LED_MODE_GREEN;
            }
        }
        else {
            // LED off
            info->status |= ONLP_LED_STATUS_PRESENT;
            info->mode |= ONLP_LED_MODE_OFF;
        }
    }
    else {
        info->status &= ~ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_E_INVALID;
    }

    return ONLP_STATUS_OK;
}

static int set_std_system_led(onlp_led_mode_t mode)
{
    int rc;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(mode == ONLP_LED_MODE_GREEN) {
        rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET, LED_SYS_AND_MASK, LED_SYS_GMASK, ONLP_I2C_F_FORCE);
    }
    else if(mode == ONLP_LED_MODE_YELLOW) {
        rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET, LED_SYS_AND_MASK, LED_SYS_YMASK, ONLP_I2C_F_FORCE);
    }
    else {
        return ONLP_STATUS_E_INVALID;
    }

    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static int set_std_fan_led(onlp_led_mode_t mode)
{
    int rc;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(mode == ONLP_LED_MODE_GREEN ) {
        rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET, LED_FAN_AND_MASK, LED_FAN_GMASK, ONLP_I2C_F_FORCE);
    }
    else if(mode == ONLP_LED_MODE_YELLOW) {
        rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET, LED_FAN_AND_MASK, LED_FAN_YMASK, ONLP_I2C_F_FORCE);
    } 
    else if(mode == ONLP_LED_MODE_OFF) {
        rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET, LED_FAN_AND_MASK, LED_FAN_OFF_OR_MASK, ONLP_I2C_F_FORCE);
    }
    else {
        return ONLP_STATUS_E_INVALID;
    }

    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static int set_std_psu_led(int led_id, onlp_led_mode_t mode)
{
    int rc;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (led_id == ONLP_LED_SYS_PSU_1) {
        if(mode == ONLP_LED_MODE_GREEN) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU1_ON_AND_MASK, LED_PSU1_ON_OR_MASK,
                                    ONLP_I2C_F_FORCE);
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET,
                                    LED_PSU1_AND_MASK, LED_PSU1_GMASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else if(mode == ONLP_LED_MODE_YELLOW) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU1_ON_AND_MASK, LED_PSU1_ON_OR_MASK,
                                    ONLP_I2C_F_FORCE);
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET,
                                    LED_PSU1_AND_MASK, LED_PSU1_YMASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else if(mode == ONLP_LED_MODE_OFF) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU1_OFF_AND_MASK, LED_PSU1_OFF_OR_MASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else {
            return ONLP_STATUS_E_INVALID;
        }

        if (rc < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else if (led_id == ONLP_LED_SYS_PSU_2) {
        if(mode == ONLP_LED_MODE_GREEN) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU2_ON_AND_MASK, LED_PSU2_ON_OR_MASK,
                                    ONLP_I2C_F_FORCE);
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET,
                                    LED_PSU2_AND_MASK,  LED_PSU2_GMASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else if(mode == ONLP_LED_MODE_YELLOW) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU2_ON_AND_MASK, LED_PSU2_ON_OR_MASK,
                                    ONLP_I2C_F_FORCE);
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_OFFSET,
                                    LED_PSU2_AND_MASK, LED_PSU2_YMASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else if(mode == ONLP_LED_MODE_OFF) {
            rc = onlp_i2c_modifyb(I2C_BUS_50, LED_REG, LED_PWOK_OFFSET,
                                    LED_PSU2_OFF_AND_MASK, LED_PSU2_OFF_OR_MASK,
                                    ONLP_I2C_F_FORCE);
        } 
        else {
            return ONLP_STATUS_E_INVALID;
        }

        if (rc < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    else {
        AIM_LOG_ERROR("[ERROR] LED ID SHOULD BE 3 OR 4, BUT = %d\r\n", led_id); 
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static int set_std_fan_tray_led(onlp_oid_t id, onlp_led_mode_t mode)
{
    int rc, temp_id;
    int offset;
    int and_mask, or_mask;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    temp_id = ONLP_OID_ID_GET(id);
    switch (temp_id) {
        case ONLP_LED_FANTRAY_1:
            offset = 3;
            if (mode == ONLP_LED_MODE_GREEN) {
                and_mask = 0xCF;
                or_mask = 0x20;
            } else if (mode == ONLP_LED_MODE_YELLOW) {
                and_mask = 0xCF;
                or_mask = 0x10;
            }
            break;
        case ONLP_LED_FANTRAY_2:
            offset = 3;
            if (mode == ONLP_LED_MODE_GREEN) {
                and_mask = 0xFC;
                or_mask = 0x02;
            } else if (mode == ONLP_LED_MODE_YELLOW) {
                and_mask = 0xFC;
                or_mask = 0x01;
            }
            break;
        case ONLP_LED_FANTRAY_3:
            offset = 2;
            if (mode == ONLP_LED_MODE_GREEN) {
                and_mask = 0xCF;
                or_mask = 0x20;
            } else if (mode == ONLP_LED_MODE_YELLOW) {
                and_mask = 0xCF;
                or_mask = 0x10;
            }
            break;
        case ONLP_LED_FANTRAY_4:
            offset = 2;
            if (mode == ONLP_LED_MODE_GREEN) {
                and_mask = 0xFC;
                or_mask = 0x02;
            } else if (mode == ONLP_LED_MODE_YELLOW) {
                and_mask = 0xFC;
                or_mask = 0x01;
            }
            break;
        default:
            return ONLP_STATUS_E_INVALID;
            break;
    }

    rc = onlp_i2c_modifyb(I2C_BUS_59, FAN_GPIO_ADDR, offset, and_mask,
                            or_mask, ONLP_I2C_F_FORCE);
    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
