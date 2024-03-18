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
 *      Fan Platform Implementation.
 *
 ***********************************************************/
#include "platform_lib.h"

#define FAN_STATUS ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B
#define FAN_CAPS   ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE
#define SYS_FAN_FRONT_MAX_RPM   22000
#define SYS_FAN_REAR_MAX_RPM    22000

/* ufispace APIs */
// Extern ufisapce APIs
extern int get_adv_psu_present(int local_id, float *pw_present);
extern int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask);
extern int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask);
// s9180-32x-adv/std
static int get_fan_local_id(int id, int *local_id);
// s9180-32x-adv
static int update_adv_fani_fru_info(int local_id, onlp_fan_info_t* info);
static int get_adv_bmc_fan_info(int local_id, onlp_fan_info_t* info);
// s9180-32x-std
static int get_std_sys_fan_present(int local_id, onlp_fan_info_t* info);
static int get_std_psu_fan_info(int local_id, onlp_fan_info_t* info);
static int set_std_sys_fan_rpm_percent(int perc);
static int get_std_fan_sysfs_id(int id);
int get_std_fan_info(int id, onlp_fan_info_t* info);  /* ledi.c extern */

/*
 * Get the fan information.
 */
onlp_fan_info_t fan_info[] = {
    {/*not used*/},
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_1_F),
            .description = "CHASSIS FAN 1 FRONT",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_1_R),
            .description = "CHASSIS FAN 1 REAR",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_2_F),
            .description = "CHASSIS FAN 2 FRONT",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_2_R),
            .description = "CHASSIS FAN 2 REAR",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_3_F),
            .description = "CHASSIS FAN 3 FRONT",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_3_R),
            .description = "CHASSIS FAN 3 REAR",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_4_F),
            .description = "CHASSIS FAN 4 FRONT",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_FAN_4_R),
            .description = "CHASSIS FAN 4 REAR",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_PSU_1_FAN),
            .description = "PSU 1 FAN",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
    {
        .hdr = {
            .id = ONLP_FAN_ID_CREATE(ONLP_PSU_2_FAN),
            .description = "PSU 2 FAN",
            .poid = POID_0,
        },
        .status = FAN_STATUS,
        .caps = FAN_CAPS,
        .rpm = 0,
        .percentage = 0,
        .mode = ONLP_FAN_MODE_INVALID,
        .model = COMM_STR_NOT_SUPPORTED,
        .serial = COMM_STR_NOT_SUPPORTED,
    },
};

typedef struct
{
    int present;
    int rpm;
    int fru_id;
} fan_attr_t;

static const fan_attr_t fan_attr[] = {
    /*                   present                     rpm                     fru_id*/
    [ONLP_FAN_1_F]   = { BMC_ATTR_ID_Fan1_PRSNT     ,BMC_ATTR_ID_FAN_1_F    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_1_R]   = { BMC_ATTR_ID_Fan1_PRSNT     ,BMC_ATTR_ID_FAN_1_R    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_2_F]   = { BMC_ATTR_ID_Fan2_PRSNT     ,BMC_ATTR_ID_FAN_2_F    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_2_R]   = { BMC_ATTR_ID_Fan2_PRSNT     ,BMC_ATTR_ID_FAN_2_R    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_3_F]   = { BMC_ATTR_ID_Fan3_PRSNT     ,BMC_ATTR_ID_FAN_3_F    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_3_R]   = { BMC_ATTR_ID_Fan3_PRSNT     ,BMC_ATTR_ID_FAN_3_R    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_4_F]   = { BMC_ATTR_ID_Fan4_PRSNT     ,BMC_ATTR_ID_FAN_4_F    ,BMC_FRU_IDX_INVALID},
    [ONLP_FAN_4_R]   = { BMC_ATTR_ID_Fan4_PRSNT     ,BMC_ATTR_ID_FAN_4_R    ,BMC_FRU_IDX_INVALID},
    [ONLP_PSU_1_FAN] = { BMC_ATTR_ID_PSU1_PRSNT     ,BMC_ATTR_ID_Fan_PSU1   ,BMC_FRU_IDX_INVALID},
    [ONLP_PSU_2_FAN] = { BMC_ATTR_ID_PSU2_PRSNT     ,BMC_ATTR_ID_Fan_PSU2   ,BMC_FRU_IDX_INVALID},
};

/**
  * @brief Initialize the fan platform subsystem.
  */
int onlp_fani_init(void)
{
    init_lock();
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information structure for the given fan OID.
 * @param id The fan OID
 * @param rv [out] Receives the fan information.
 */
int onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* rv)
{
    int local_id;

    ONLP_TRY(get_fan_local_id(id, &local_id));

    // Clean rv 
    memset(rv, 0, sizeof(onlp_fan_info_t));

    if(bmc_enable){
        ONLP_TRY(get_adv_bmc_fan_info(local_id, rv));
    }
    else {
        int rc;
        *rv = fan_info[local_id];
        rv->caps |= ONLP_FAN_CAPS_GET_RPM;

        switch(local_id) {
        case ONLP_FAN_1_F:
        case ONLP_FAN_1_R:
        case ONLP_FAN_2_F:
        case ONLP_FAN_2_R:
        case ONLP_FAN_3_F:
        case ONLP_FAN_3_R:
        case ONLP_FAN_4_F:
        case ONLP_FAN_4_R:
            rc = get_std_fan_info(local_id, rv);
            break;
        case ONLP_PSU_1_FAN:
        case ONLP_PSU_2_FAN:
            rc = get_std_psu_fan_info(local_id, rv);
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
        }

        return rc;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the fan's operational status.
 * @param id The fan OID.
 * @param rv [out] Receives the fan's operations status flags.
 * @notes Only operational state needs to be returned -
 *        PRESENT/FAILED
 */
int onlp_fani_status_get(onlp_oid_t id, uint32_t* rv)
{
    int local_id;
    onlp_fan_info_t info ={0};

    ONLP_TRY(get_fan_local_id(id, &local_id));

    if(bmc_enable){
        ONLP_TRY(get_adv_bmc_fan_info(local_id, &info));
    }
    else {
        //int rc;
        info = fan_info[local_id];
        info.caps |= ONLP_FAN_CAPS_GET_RPM;

        switch(local_id) {
        case ONLP_FAN_1_F:
        case ONLP_FAN_1_R:
        case ONLP_FAN_2_F:
        case ONLP_FAN_2_R:
        case ONLP_FAN_3_F:
        case ONLP_FAN_3_R:
        case ONLP_FAN_4_F:
        case ONLP_FAN_4_R:
            ONLP_TRY(get_std_fan_info(local_id, &info));
            break;
        case ONLP_PSU_1_FAN:
        case ONLP_PSU_2_FAN:
            ONLP_TRY(get_std_psu_fan_info(local_id, &info));
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
            break;
        }
    }

    *rv = info.status;

    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan OID.
 * @param rv [out] Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int local_id;

    ONLP_TRY(get_fan_local_id(id, &local_id));
    *hdr = fan_info[local_id].hdr;

    return ONLP_STATUS_OK;
}

/**
 * @brief Set the fan speed in percentage.
 * @param id The fan OID.
 * @param p The new fan speed percentage.
 * @note This is only relevant if the PERCENTAGE capability is set.
 */
int onlp_fani_percentage_set(onlp_oid_t id, int percentage)
{
    int perc_val, rc;
    int local_id;

    if(bmc_enable) {
        AIM_LOG_INFO("CONTROLLED BY BMC\r\n");
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    ONLP_TRY(get_fan_local_id(id, &local_id));

    /*
     * Set fan speed
     * Driver accept value in range between 128 and 255.
     * Value 128 is 50%.
     * Value 200 is 80%.
     * Value 255 is 100%.
     */
    if (percentage == 100) {
        perc_val = 255;
    } 
    else if (percentage == 80) {
        perc_val = 200;
    } 
    else if (percentage == 50) {
        perc_val = 128;
    } 
    else {
        return ONLP_STATUS_E_INVALID;
    }

    switch (local_id)
    {
        case ONLP_FAN_1_F:
        case ONLP_FAN_1_R:
        case ONLP_FAN_2_F:
        case ONLP_FAN_2_R:
        case ONLP_FAN_3_F:
        case ONLP_FAN_3_R:
        case ONLP_FAN_4_F:
        case ONLP_FAN_4_R:
            rc = set_std_sys_fan_rpm_percent(perc_val);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    return rc;
}

/**
 * @brief Get and check fan local ID
 * @param id [in] OID
 * @param local_id [out] The fan local id
 */
static int get_fan_local_id(int id, int *local_id)
{
    int tmp_id;

    if(local_id == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    if(!ONLP_OID_IS_FAN(id)) {
        return ONLP_STATUS_E_INVALID;
    }

    tmp_id = ONLP_OID_ID_GET(id);

    switch (tmp_id) {
        case ONLP_FAN_1_F:
        case ONLP_FAN_1_R:
        case ONLP_FAN_2_F:
        case ONLP_FAN_2_R:
        case ONLP_FAN_3_F:
        case ONLP_FAN_3_R:
        case ONLP_FAN_4_F:
        case ONLP_FAN_4_R:
        case ONLP_PSU_1_FAN:
        case ONLP_PSU_2_FAN:
            *local_id = tmp_id;
            return ONLP_STATUS_OK;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    
    return ONLP_STATUS_E_INVALID;
}

/**
 * @brief Update the information of Model and Serial from FAN EEPROM
 * @param local_id The FAN Local ID
 * @param[out] info Receives the FAN information (model and serial).
 */
static int update_adv_fani_fru_info(int local_id, onlp_fan_info_t* info)
{
    bmc_fru_t fru = {0};

    /* Not support fru */
    if(fan_attr[local_id].fru_id == BMC_FRU_IDX_INVALID) {
        return ONLP_STATUS_OK;
    }

    /* read fru data */
    ONLP_TRY(read_bmc_fru(fan_attr[local_id].fru_id, &fru));

    /* update FRU model */
    memset(info->model, 0, sizeof(info->model));
    snprintf(info->model, sizeof(info->model), "%s", fru.name.val);

    /* update FRU serial */
    memset(info->serial, 0, sizeof(info->serial));
    snprintf(info->serial, sizeof(info->serial), "%s", fru.serial.val);

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the fan information from BMC
 * @param id [in] FAN ID
 * @param info [out] The fan information
 */
static int get_adv_bmc_fan_info(int local_id, onlp_fan_info_t* info)
{
    int rpm=0, percentage=0;
    float data=0;
    int sys_max_fan_speed = SYS_FAN_FRONT_MAX_RPM;
    int psu_max_fan_speed = MAX_SYS_FAN_RPM;
    int presence;
    float pw_present;

    *info = fan_info[local_id];

    /* present */
    if (local_id >= ONLP_FAN_1_F && local_id <= ONLP_FAN_4_R) {
        int bmc_attr = fan_attr[local_id].present;

        ONLP_TRY(read_bmc_sensor(bmc_attr, FAN_SENSOR, &data));
        presence = (int) data;

        if(presence == BMC_ATTR_STATUS_PRES) {
            info->status |= ONLP_FAN_STATUS_PRESENT;
        } else {
            info->status &= ~ONLP_FAN_STATUS_PRESENT;
        }
    } 
    // PSU 1 FAN
    else if(local_id == ONLP_PSU_1_FAN) {
        ONLP_TRY(get_adv_psu_present(ONLP_PSU_1, &pw_present));
        if( pw_present == PSU_STATUS_PRSNT ) {
            info->status |= ONLP_FAN_STATUS_PRESENT;
        } 
        else {
            info->status &= ~ONLP_FAN_STATUS_PRESENT;
        }
    } 
    // PSU 2 FAN
    else if(local_id == ONLP_PSU_2_FAN) {
        ONLP_TRY(get_adv_psu_present(ONLP_PSU_2, &pw_present));
        if( pw_present == PSU_STATUS_PRSNT ) {
            info->status |= ONLP_FAN_STATUS_PRESENT;
        } 
        else {
            info->status &= ~ONLP_FAN_STATUS_PRESENT;
            info->status &= ~ONLP_PSU_STATUS_PRESENT;
            info->status |=  ONLP_PSU_STATUS_UNPLUGGED;
        }
    }

    /* direction */
    if(info->status & ONLP_FAN_STATUS_PRESENT) {
        info->status |= ONLP_FAN_STATUS_F2B;
        info->status &= ~ONLP_FAN_STATUS_B2F; 
    } 
    else {
        info->status &= ~ONLP_FAN_STATUS_F2B;
        info->status &= ~ONLP_FAN_STATUS_B2F;
    }

    /* contents */
    if(info->status & ONLP_FAN_STATUS_PRESENT) {
        //get fan rpm
        int bmc_attr = fan_attr[local_id].rpm;
        ONLP_TRY(read_bmc_sensor(bmc_attr, FAN_SENSOR, &data));
        rpm = (int) data;

        if(rpm == BMC_ATTR_INVALID_VAL) {
            info->status |= ONLP_FAN_STATUS_FAILED;
            return ONLP_STATUS_OK;
        }

        //set rpm field
        info->rpm = rpm;

        switch(local_id) {
            case ONLP_FAN_1_F:
            case ONLP_FAN_2_F:
            case ONLP_FAN_3_F:
            case ONLP_FAN_4_F:
            {
                sys_max_fan_speed = SYS_FAN_FRONT_MAX_RPM;
                percentage = (info->rpm*100)/sys_max_fan_speed;
                info->percentage = (percentage >= 100) ? 100:percentage;
                info->status |= (rpm == 0) ? ONLP_FAN_STATUS_FAILED : 0;
                break;
            }
            case ONLP_FAN_1_R:
            case ONLP_FAN_2_R:
            case ONLP_FAN_3_R:
            case ONLP_FAN_4_R:
            {
                sys_max_fan_speed = SYS_FAN_REAR_MAX_RPM;
                percentage = (info->rpm*100)/sys_max_fan_speed;
                info->percentage = (percentage >= 100) ? 100:percentage;
                info->status |= (rpm == 0) ? ONLP_FAN_STATUS_FAILED : 0;
                break;
            }
            case ONLP_PSU_1_FAN:
            case ONLP_PSU_2_FAN:
            {
                percentage = (info->rpm*100)/psu_max_fan_speed;
                info->percentage = (percentage >= 100) ? 100:percentage;
                info->status |= (rpm == 0) ? ONLP_FAN_STATUS_FAILED : 0;
                break;
            }
            default:
                return ONLP_STATUS_E_INVALID;
        }

        /* Get FRU (model/serial) */
        ONLP_TRY(update_adv_fani_fru_info(local_id, info));
    }
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the fan present from sys
 * @param id [in] FAN ID
 * @param info [out] The fan information
 */
static int get_std_sys_fan_present(int local_id, onlp_fan_info_t* info) 
{
    int rv, fan_presence, i2c_bus, offset, fan_reg_mask;

    /* get fan presence*/
    i2c_bus = I2C_BUS_59;
    switch (local_id)
    {
        case ONLP_FAN_1_F:
        case ONLP_FAN_1_R:
            offset = 1;
            fan_reg_mask = FAN_1_2_PRESENT_MASK;
            break;
        case ONLP_FAN_2_F:
        case ONLP_FAN_2_R:
            offset = 1;
            fan_reg_mask = FAN_3_4_PRESENT_MASK;
            break;
        case ONLP_FAN_3_F:
        case ONLP_FAN_3_R:
            offset = 0;
            fan_reg_mask = FAN_5_6_PRESENT_MASK;
            break;
        case ONLP_FAN_4_F:
        case ONLP_FAN_4_R:
            offset = 0;
            fan_reg_mask = FAN_7_8_PRESENT_MASK;
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    rv = onlp_i2c_readb(i2c_bus, FAN_GPIO_ADDR, offset, ONLP_I2C_F_FORCE);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    fan_presence = (rv & fan_reg_mask) ? 0 : 1;

    if (!fan_presence) {
        info->status &= ~ONLP_FAN_STATUS_PRESENT; // clear present
    } 
    else {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the sys fan information from sys
 * @param id [in] FAN ID
 * @param info [out] The fan information
*/
int get_std_fan_info(int id, onlp_fan_info_t* info) 
{
    int rv, fan_status, fan_rpm, perc_val, percentage;
    int max_fan_speed = MAX_SYS_FAN_RPM;
    fan_status = 0;
    fan_rpm = 0;
    int sysfs_id;

    rv = get_std_sys_fan_present(id, info);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    sysfs_id = get_std_fan_sysfs_id(id);
    if (!sysfs_id) {
        return ONLP_STATUS_E_INTERNAL;
    }

    //rv = onlp_file_read_int(&fan_status, SYS_FAN_PREFIX "fan%d_alarm", id);
    rv = onlp_file_read_int(&fan_status, SYS_FAN_PREFIX "fan%d_alarm", sysfs_id);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* fan status > 1, means failure */
    if (fan_status > 0) {
        info->status |= ONLP_FAN_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }

    rv = onlp_file_read_int(&fan_rpm, SYS_FAN_PREFIX "fan%d_input", sysfs_id);
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    info->rpm = fan_rpm;

    /* get speed percentage*/
    switch (sysfs_id)
    {
        case ONLP_FAN_1_F:
        case ONLP_FAN_2_F:
        case ONLP_FAN_3_F:
        case ONLP_FAN_4_F:
            rv = onlp_file_read_int(&perc_val, SYS_FAN_PREFIX "pwm%d",
                                    FAN_CTRL_SET1);
            break;
        case ONLP_FAN_1_R:
        case ONLP_FAN_2_R:
        case ONLP_FAN_3_R:
        case ONLP_FAN_4_R:
            rv = onlp_file_read_int(&perc_val, SYS_FAN_PREFIX "pwm%d",
                                    FAN_CTRL_SET2);
            break;
        default:
            return ONLP_STATUS_E_INVALID;
    }
    if (rv < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    percentage = (info->rpm*100)/max_fan_speed;
    info->percentage = percentage;

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the psu fan information from sys
 * @param id [in] FAN ID
 * @param info [out] The fan information
*/
static int get_std_psu_fan_info(int local_id, onlp_fan_info_t* info)  
{
    int pw_exist, pw_good, exist_offset, good_offset;
    int i2c_bus, psu_mask, rc;
    unsigned int tmp_fan_rpm, fan_rpm;
    int max_fan_speed = MAX_PSU_FAN_RPM;

    if (local_id == ONLP_PSU_1_FAN) {
        i2c_bus = I2C_BUS_PSU1;
        exist_offset = PSU1_PRESENT_OFFSET;
        good_offset = PSU1_PWGOOD_OFFSET;
    } 
    else if (local_id == ONLP_PSU_2_FAN) {
        i2c_bus = I2C_BUS_PSU2;
        exist_offset = PSU2_PRESENT_OFFSET;
        good_offset = PSU2_PWGOOD_OFFSET;
    } 
    else {
        return ONLP_STATUS_E_INTERNAL;
    }

    psu_mask = PSU_MUX_MASK;

    /* check psu status */
    if ((rc = get_std_psu_present(&pw_exist, exist_offset, I2C_BUS_0, psu_mask))
            != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (pw_exist != PSU_STATUS_PRSNT) {
        info->rpm = 0;
        info->status &= ~ONLP_FAN_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    } 
    else {
        info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    if ((rc = get_std_psu_pwgood(&pw_good, good_offset, I2C_BUS_0, psu_mask))
            != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (pw_good != PSU_STATUS_POWER_GOOD) {
        info->rpm = 0;
        return ONLP_STATUS_OK;
    }

    tmp_fan_rpm = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_FAN_RPM_OFFSET, ONLP_I2C_F_FORCE);

    fan_rpm = (unsigned int)tmp_fan_rpm;
    fan_rpm = (fan_rpm & 0x07FF) * (1 << ((fan_rpm >> 11) & 0x1F));
    info->rpm = (int)fan_rpm;
    info->percentage = (info->rpm*100)/max_fan_speed;

    return ONLP_STATUS_OK;
}

static int set_std_sys_fan_rpm_percent(int perc)
{
    int rc;

    if ( bmc_enable ) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    rc = onlp_file_write_int(perc, SYS_FAN_PREFIX "pwm%d", FAN_CTRL_SET1);

    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    rc = onlp_file_write_int(perc, SYS_FAN_PREFIX "pwm%d", FAN_CTRL_SET2);
    if (rc < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

static int get_std_fan_sysfs_id(int id)
{
    int sysfs_id;
    switch (id)
    {
        case ONLP_FAN_1_F:
            sysfs_id = 8;
            break;
        case ONLP_FAN_2_F:
            sysfs_id = 7;
            break;
        case ONLP_FAN_3_F:
            sysfs_id = 6;
            break;
        case ONLP_FAN_4_F:
            sysfs_id = 5;
            break;
        case ONLP_FAN_1_R:
            sysfs_id = 4;
            break;
        case ONLP_FAN_2_R:
            sysfs_id = 3;
            break;
        case ONLP_FAN_3_R:
            sysfs_id = 2;
            break;
        case ONLP_FAN_4_R:
            sysfs_id = 1;
            break;
        default:
            sysfs_id = 0;
    }
    return sysfs_id;
}