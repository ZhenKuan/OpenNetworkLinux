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
 *      Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include "platform_lib.h"

#define MILLI(cel) (cel * 1000)

/* ufispace APIs */
// Extern ufisapce APIs
extern int get_adv_psu_present(int local_id, float *pw_present);
extern int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask);
extern int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask);
// s9180-32x-adv/std
static int get_thermal_local_id(int id, int *local_id);
// s9180-32x-adv
static int get_adv_bmc_thermal_info(int local_id, onlp_thermal_info_t* info);
// s9180-32x-std
static int get_std_thermal_info(int local_id, onlp_thermal_info_t* info);
static int get_std_psu_thermal_info(int local_id, onlp_thermal_info_t* info);

 //FIXME threshold
static onlp_thermal_info_t thermal_bmc_info[] = {
    {/* Not used */}, 
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_BMC),
            .description = BMC_ATTR_NAME_Temp_BMC,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC),
            .description = BMC_ATTR_NAME_Temp_MAC,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC_Front),
            .description = BMC_ATTR_NAME_Temp_MAC_Front,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_MAC_Rear),
            .description = BMC_ATTR_NAME_Temp_MAC_Rear,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_Battery),
            .description = BMC_ATTR_NAME_Temp_Battery,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_OOB_conn),
            .description = BMC_ATTR_NAME_Temp_OOB_conn,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_SFP_conn),
            .description = BMC_ATTR_NAME_Temp_SFP_conn,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PCIE_conn),
            .description = BMC_ATTR_NAME_Temp_PCIE_conn,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PSU1_AMB),
            .description = BMC_ATTR_NAME_Temp_PSU1_AMB,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PSU2_AMB),
            .description = BMC_ATTR_NAME_Temp_PSU2_AMB,
            .poid = POID_0,
        },
        .status = ONLP_THERMAL_STATUS_PRESENT,
        .caps = (ONLP_THERMAL_CAPS_ALL)
    },
};

static onlp_thermal_info_t thermal_std_info[] = {
    {/* Not used */},
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_FRONT_MAC), "FRONT MAC", 0},  /* hdr */
                ONLP_THERMAL_STATUS_PRESENT,  /* status */
                ONLP_THERMAL_CAPS_ALL, 0, {80000,  85000, 90000}  /* caps, mcelsius, thresholds */
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_ASIC), "ASIC CORE TEMP", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {84787,  89250, 93712}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU1), "CPU THERMAL 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU2), "CPU THERMAL 2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU3), "CPU THERMAL 3", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU4), "CPU THERMAL 4", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU1_1), "PSU 1 THERMAL 1", ONLP_PSU_ID_CREATE(ONLP_PSU_1)},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {70000,  75000, 75000}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU2_1), "PSU 2 THERMAL 1", ONLP_PSU_ID_CREATE(ONLP_PSU_2)},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {70000,  75000, 75000}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_CPU_BOARD), "CPU BOARD", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {78000,  80000, 82000}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU1_NEAR), "NEAR PSU 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {61132,  64350, 67567}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU2_NEAR), "NEAR PSU 2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {61132,  64350, 67567}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_MAC_REAR), "REAR MAC", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000,  85000, 90000}
    },
    { { ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_QSFP_NEAR), "NEAR QSFP PORT", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {58900,  62000, 65100}
    }
};

typedef enum thrm_attr_type_e {
    TYPE_THRM_ATTR_UNNKOW = 0,
    // s9180-adv
    TYPE_THRM_ATTR_BMC,
    TYPE_THRM_ATTR_MAX,
} thrm_type_t;

typedef struct
{
    int type;
    int attr;
} thrm_attr_t;

static const thrm_attr_t thrm_bmc_attr[] = {
    //bmc
    [ONLP_THERMAL_ID_Temp_BMC]             =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_BMC},
    [ONLP_THERMAL_ID_Temp_MAC]             =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_MAC},
    [ONLP_THERMAL_ID_Temp_MAC_Front]       =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_MAC_Front},
    [ONLP_THERMAL_ID_Temp_MAC_Rear]        =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_MAC_Rear},
    [ONLP_THERMAL_ID_Temp_Battery]         =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_Battery},
    [ONLP_THERMAL_ID_Temp_OOB_conn]        =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_OOB_conn},
    [ONLP_THERMAL_ID_Temp_SFP_conn]        =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_SFP_conn},
    [ONLP_THERMAL_ID_Temp_PCIE_conn]       =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_PCIE_conn},
    [ONLP_THERMAL_ID_Temp_PSU1_AMB]        =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_PSU1_AMB},
    [ONLP_THERMAL_ID_Temp_PSU2_AMB]        =   {TYPE_THRM_ATTR_BMC      ,BMC_ATTR_ID_Temp_PSU2_AMB},
};

/**
 * @brief Initialize the thermal subsystem.
 */
int onlp_thermali_init(void)
{
    init_lock();
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information for the given thermal OID.
 * @param id The Thermal OID
 * @param rv [out] Receives the thermal information.
 */
int onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* rv)
{
    int local_id;

    if(rv == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    ONLP_TRY(get_thermal_local_id(id, &local_id));

    /* Clean rv */ 
    memset(rv, 0, sizeof(onlp_thermal_info_t));

    /* update info  */
    if(bmc_enable){
        if(thrm_bmc_attr[local_id].type == TYPE_THRM_ATTR_BMC && thrm_bmc_attr[local_id].type != TYPE_THRM_ATTR_UNNKOW) {
            ONLP_TRY(get_adv_bmc_thermal_info(local_id, rv));
        }
        else {
            return ONLP_STATUS_E_INVALID;
        }
    }
    else {
        ONLP_TRY(get_std_thermal_info(local_id, rv));
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the thermal's operational status.
 * @param id The thermal oid.
 * @param rv [out] Receives the operational status.
 */
int onlp_thermali_status_get(onlp_oid_t id, uint32_t* rv)
{
    int local_id;
    onlp_thermal_info_t info ={0};

    ONLP_TRY(get_thermal_local_id(id, &local_id));

    if (bmc_enable) {
        ONLP_TRY(get_adv_bmc_thermal_info(local_id, &info));
    }
    else {
        ONLP_TRY(onlp_thermal_info_get(local_id, &info));
    }

    *rv = info.status;

    return ONLP_STATUS_OK;
}

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param rv [out] Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    int local_id;

    ONLP_TRY(get_thermal_local_id(id, &local_id));
    *rv = thermal_std_info[local_id].hdr;

    return ONLP_STATUS_OK;
}

/**
 * @brief Get and check thermal local ID
 * @param id [in] OID
 * @param local_id [out] The thermal local id
 */
static int get_thermal_local_id(int id, int *local_id)
{
    int tmp_id;
    if(local_id == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    if(!ONLP_OID_IS_THERMAL(id)) {
        return ONLP_STATUS_E_INVALID;
    }

    tmp_id = ONLP_OID_ID_GET(id);

    if(bmc_enable) {
        switch (tmp_id) {
            /* bmc */
            case ONLP_THERMAL_ID_Temp_BMC:
            case ONLP_THERMAL_ID_Temp_MAC:
            case ONLP_THERMAL_ID_Temp_MAC_Front:
            case ONLP_THERMAL_ID_Temp_MAC_Rear:
            case ONLP_THERMAL_ID_Temp_Battery:
            case ONLP_THERMAL_ID_Temp_OOB_conn:
            case ONLP_THERMAL_ID_Temp_SFP_conn:
            case ONLP_THERMAL_ID_Temp_PCIE_conn:
            case ONLP_THERMAL_ID_Temp_PSU1_AMB:
            case ONLP_THERMAL_ID_Temp_PSU2_AMB:
                *local_id = tmp_id;
                return ONLP_STATUS_OK;
            default:
                return ONLP_STATUS_E_INVALID;
        }
    }
    else {
        switch (tmp_id) {
            /* sys */
            case ONLP_THERMAL_ID_FRONT_MAC:
            case ONLP_THERMAL_ID_ASIC:
            case ONLP_THERMAL_ID_CPU1:
            case ONLP_THERMAL_ID_CPU2:
            case ONLP_THERMAL_ID_CPU3:
            case ONLP_THERMAL_ID_CPU4:
            case ONLP_THERMAL_ID_PSU1_1:
            case ONLP_THERMAL_ID_PSU2_1:
            case ONLP_THERMAL_ID_CPU_BOARD:
            case ONLP_THERMAL_ID_PSU1_NEAR:
            case ONLP_THERMAL_ID_PSU2_NEAR:
            case ONLP_THERMAL_ID_MAC_REAR:
            case ONLP_THERMAL_ID_QSFP_NEAR:
                *local_id = tmp_id;
                return ONLP_STATUS_OK;
            default:
                return ONLP_STATUS_E_INVALID;
        }
    }
    
    return ONLP_STATUS_E_INVALID;
}

static int get_std_thermal_info(int local_id, onlp_thermal_info_t* info)
{
    int rv = 0;

    *info = thermal_std_info[local_id];

    switch (local_id) {
            case ONLP_THERMAL_ID_FRONT_MAC:
            case ONLP_THERMAL_ID_ASIC:
                rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CORE_TEMP_PREFIX "temp%d_input", local_id);
                break;
            case ONLP_THERMAL_ID_CPU1:
            case ONLP_THERMAL_ID_CPU2:
            case ONLP_THERMAL_ID_CPU3:
            case ONLP_THERMAL_ID_CPU4:
                rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_TEMP_PREFIX "temp%d_input", (local_id-1));
                break;
            case ONLP_THERMAL_ID_PSU1_1:
            case ONLP_THERMAL_ID_PSU2_1:
                rv = get_std_psu_thermal_info(local_id, info);
                break;
            case ONLP_THERMAL_ID_CPU_BOARD:
                rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_BOARD_TEMP_PREFIX "temp1_input");
                break;
            case ONLP_THERMAL_ID_PSU1_NEAR:
            case ONLP_THERMAL_ID_PSU2_NEAR:
                if (local_id == ONLP_THERMAL_ID_PSU1_NEAR) {
                    rv = onlp_file_read_int(&info->mcelsius,
                                        SYS_PSU1_NEAR_TEMP_PREFIX "temp1_input");
                } 
                else if (local_id == ONLP_THERMAL_ID_PSU2_NEAR) {
                    rv = onlp_file_read_int(&info->mcelsius,
                                        SYS_PSU2_NEAR_TEMP_PREFIX "temp1_input");
                } 
                else {
                    return ONLP_STATUS_E_INTERNAL;
                }
                break;
            case ONLP_THERMAL_ID_MAC_REAR:
                rv = onlp_file_read_int(&info->mcelsius,
                            SYS_MAC_REAR_TEMP_PREFIX "temp1_input");
                break;
            case ONLP_THERMAL_ID_QSFP_NEAR:
                rv = onlp_file_read_int(&info->mcelsius,
                            SYS_QSFP_NEAR_TEMP_PREFIX "temp1_input");
                break;
            default:
                return ONLP_STATUS_E_INTERNAL;
                break;
    }

    if(rv == ONLP_STATUS_E_INTERNAL) {
        return rv;
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_thermal_info(int local_id, onlp_thermal_info_t* info) 
{
    int pw_exist, pw_good, exist_offset, good_offset;
    int offset, i2c_bus, rc;
    int value, buf, psu_mask;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];

    if (local_id == ONLP_THERMAL_ID_PSU1_1) {
        i2c_bus = I2C_BUS_PSU1;
        offset = PSU_THERMAL1_OFFSET;
        exist_offset = PSU1_PRESENT_OFFSET;
        good_offset = PSU1_PWGOOD_OFFSET;
    }
    else if (local_id == ONLP_THERMAL_ID_PSU2_1) {
        i2c_bus = I2C_BUS_PSU2;
        offset = PSU_THERMAL1_OFFSET;
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
        info->mcelsius = 0;
        info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
        return ONLP_STATUS_OK;
    } 
    else {
        info->status |= ONLP_THERMAL_STATUS_PRESENT;
    }

    if ((rc = get_std_psu_pwgood(&pw_good, good_offset, I2C_BUS_0, psu_mask))
            != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (pw_good != PSU_STATUS_POWER_GOOD) {
        info->mcelsius = 0;
        return ONLP_STATUS_OK;
    }

    value = onlp_i2c_readw(i2c_bus, PSU_REG, offset, ONLP_I2C_F_FORCE);

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp)
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    buf = atof((const char *)result);
    info->mcelsius = (int)(buf * 1000);

    return ONLP_STATUS_OK;

}

static int get_adv_bmc_thermal_info(int local_id, onlp_thermal_info_t* info)
{
    float data = 0;

    *info = thermal_bmc_info[local_id];
    temp_thld_t temp_thld = {0};
    
    ONLP_TRY(get_thermal_thld(local_id, &temp_thld));
    info->thresholds.warning = MILLI(temp_thld.warning);
    info->thresholds.error = MILLI(temp_thld.error);
    info->thresholds.shutdown = MILLI(temp_thld.shutdown);

    /* present */
    if(local_id == ONLP_THERMAL_ID_Temp_PSU1_AMB || local_id == ONLP_THERMAL_ID_Temp_PSU2_AMB) {
        /* When the PSU module is unplugged, the psu thermal does not exist. */
        int psu_local_id = ONLP_PSU_MAX;

        if(local_id == ONLP_THERMAL_ID_Temp_PSU1_AMB) {
            psu_local_id = ONLP_PSU_1;
        } 
        else {
            psu_local_id = ONLP_PSU_2;
        }

        float psu_present = 0;
        ONLP_TRY(get_adv_psu_present(psu_local_id, &psu_present));
        if (psu_present == PSU_STATUS_PRSNT) {
            info->status |= ONLP_THERMAL_STATUS_PRESENT;
        } 
        else {
            info->status &= ~ONLP_THERMAL_STATUS_PRESENT;
        }
    }
    else {
        info->status |= ONLP_THERMAL_STATUS_PRESENT;
    }

    /* contents */
    if(info->status & ONLP_THERMAL_STATUS_PRESENT) {
        int bmc_attr = thrm_bmc_attr[local_id].attr;
        ONLP_TRY(read_bmc_sensor(bmc_attr, THERMAL_SENSOR, &data));

        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->status &= ~ONLP_THERMAL_STATUS_FAILED;
            info->mcelsius = (int) (data*1000);
        }
        else {
            info->status |= ONLP_THERMAL_STATUS_FAILED;
            info->mcelsius = 0;
        }
    }
    else {
        info->status |= ONLP_THERMAL_STATUS_FAILED;
    }

    return ONLP_STATUS_OK;
}
