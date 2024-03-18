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
 *      Power Supply Management Implementation.
 *
 ***********************************************************/
#include "platform_lib.h"

#define PSU_STATUS_PWR_FAIL    (0)
#define PSU_STATUS_PWR_GD      (1)

/* ufispace Specific APIs */
/* functions for s9180-32x-adv/std */
static int get_psu_local_id(int id, int *local_id);
/* functions for s9180-32x-adv */
static int update_adv_psui_fru_info(int local_id, onlp_psu_info_t* info);
static int update_adv_psui_info(int local_id, onlp_psu_info_t *info);
int get_adv_psu_present(int local_id, float *pw_present); 
/* functions for s9180-32x-std */
static int get_std_psu_eeprom(onlp_psu_info_t* info, int id);
static int get_std_psu_iout(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_pin(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_pout(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_vout(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_iin(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_vin(onlp_psu_info_t* info, int i2c_bus);
static int get_std_psu_status_info(int local_id, onlp_psu_info_t *info);
int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask); 
int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask);


/* s9180-adv psu info */
static onlp_psu_info_t adv_psu_info[] =
{
    { }, /* Not used */
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .description = "PSU 1",
            .poid = POID_0,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_PSU_1_FAN),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PSU1_AMB)
            }
        },
        .model = COMM_STR_NOT_AVAILABLE,
        .serial = COMM_STR_NOT_AVAILABLE,
    },
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .description = "PSU 2",
            .poid = POID_0,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_PSU_2_FAN),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_Temp_PSU2_AMB)
            }
        },
        .model = COMM_STR_NOT_AVAILABLE,
        .serial = COMM_STR_NOT_AVAILABLE,
    },
};

/* s9180-std psu info */
static onlp_psu_info_t std_psu_info[] =
{
    {/* Not used */}, 
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .description = "PSU 1",
            .poid = POID_0,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_PSU_1_FAN),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU1_1)
            }
        },
        .model = COMM_STR_NOT_AVAILABLE,
        .serial = COMM_STR_NOT_AVAILABLE,
    },
    {
        .hdr = {
            .id = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .description = "PSU 2",
            .poid = POID_0,
            .coids = {
                ONLP_FAN_ID_CREATE(ONLP_PSU_2_FAN),
                ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ID_PSU2_1)
            }
        },
        .model = COMM_STR_NOT_AVAILABLE,
        .serial = COMM_STR_NOT_AVAILABLE,
    },
};

typedef struct
{
    int abs;
    int fru_id;
} psu_attr_t;

typedef enum cpld_attr_idx_e {
    CPLD_PSU = 0,
} cpld_attr_idx_t;

static const psu_attr_t psu_attr[] = {
    /*              abs   fru_id */
    [ONLP_PSU_1] = {0,    BMC_FRU_IDX_ONLP_PSU_1},
    [ONLP_PSU_2] = {1,    BMC_FRU_IDX_ONLP_PSU_2},
};

/**
 * @brief Initialize the PSU subsystem.
 */
int onlp_psui_init(void)
{
    init_lock();
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the information structure for the given PSU
 * @param id The PSU OID
 * @param rv [out] Receives the PSU information.
 */
int onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* rv)
{
    int local_id;

    ONLP_TRY(get_psu_local_id(id, &local_id));

    /* Clean rv */ 
    memset(rv, 0, sizeof(onlp_psu_info_t));

    if(bmc_enable) {
        ONLP_TRY(update_adv_psui_info(local_id, rv));
    }
    else {
        switch (local_id) {
        case ONLP_PSU_1:
        case ONLP_PSU_2:
            ONLP_TRY(get_std_psu_status_info(local_id, rv));
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
        }
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the PSU's operational status.
 * @param id The PSU OID.
 * @param rv [out] Receives the operational status.
 */
int onlp_psui_status_get(onlp_oid_t id, uint32_t* rv)
{
    int local_id;
    onlp_psu_info_t info ={0};

    ONLP_TRY(get_psu_local_id(id, &local_id));

    if(bmc_enable) {
        ONLP_TRY(update_adv_psui_info(local_id, &info));
    }
    else {
        switch (local_id) {
        case ONLP_PSU_1:
        case ONLP_PSU_2:
            ONLP_TRY(get_std_psu_status_info(local_id, &info));
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
            break;
        }
    }

    *rv = info.status;

    return ONLP_STATUS_OK;
}

/**
 * @brief Get the PSU's oid header.
 * @param id The PSU OID.
 * @param rv [out] Receives the header.
 */
int onlp_psui_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    int local_id;

    ONLP_TRY(get_psu_local_id(id, &local_id));
    if (bmc_enable) {
        *rv = adv_psu_info[local_id].hdr;
    }
    else {
        *rv = std_psu_info[local_id].hdr;
    }
    

    return ONLP_STATUS_OK;
}

/**
 * @brief Generic PSU ioctl
 * @param id The PSU OID
 * @param vargs The variable argument list for the ioctl call.
 */
int onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/**
 * @brief Get and check psu local ID
 * @param id [in] OID
 * @param local_id [out] The psu local id
 */
static int get_psu_local_id(int id, int *local_id)
{
    int tmp_id;

    if(local_id == NULL) {
        return ONLP_STATUS_E_PARAM;
    }

    if(!ONLP_OID_IS_PSU(id)) {
        return ONLP_STATUS_E_INVALID;
    }

    tmp_id = ONLP_OID_ID_GET(id);
    switch (tmp_id) {
        case ONLP_PSU_1:
        case ONLP_PSU_2:
            *local_id = tmp_id;
            return ONLP_STATUS_OK;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    return ONLP_STATUS_E_INVALID;
}

/**
 * @brief Update the information of Model and Serial from PSU EEPROM
 * @param local_id The PSU Local ID
 * @param[out] info Receives the PSU information (model and serial).
 */
static int update_adv_psui_fru_info(int local_id, onlp_psu_info_t* info)
{
    bmc_fru_t fru = {0};

    /* read fru data */
    ONLP_TRY(read_bmc_fru(psu_attr[local_id].fru_id, &fru));

    /* update FRU model */
    memset(info->model, 0, sizeof(info->model));
    snprintf(info->model, sizeof(info->model), "%s", fru.name.val);

    //update FRU serial
    memset(info->serial, 0, sizeof(info->serial));
    snprintf(info->serial, sizeof(info->serial), "%s", fru.serial.val);

    return ONLP_STATUS_OK;
}

int get_adv_psu_present(int local_id, float *pw_present) {
    int attr_prsnt;

    /* Get power present status */
    if (local_id == ONLP_PSU_1) {
        attr_prsnt = BMC_ATTR_ID_PSU1_PRSNT;
    }
    else if (local_id == ONLP_PSU_2) {
        attr_prsnt = BMC_ATTR_ID_PSU2_PRSNT;
    }
    else {
        AIM_LOG_ERROR("[ERROR] Incorrect PSU local_id(%d), while getting PWR PRSNT ATTR\r\n", local_id); 
        return ONLP_STATUS_E_INTERNAL;
    }
    
    ONLP_TRY(read_bmc_sensor(attr_prsnt , PSU_SENSOR, pw_present));

    if(pw_present == NULL){
        AIM_LOG_ERROR("[ERROR] pw_present = NULL\r\n"); 
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
} 

static int update_adv_psui_info(int local_id, onlp_psu_info_t *info)
{
    float pw_present, pw_good;
    float data;
    int attr_pwrok, attr_vin, attr_vout, attr_iin, attr_iout, attr_pin, attr_pout;

    *info = adv_psu_info[local_id];
    
    /* Get PW Present status */
    ONLP_TRY(get_adv_psu_present(local_id, &pw_present));
    
    if (pw_present != PSU_STATUS_PRSNT) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        info->status |=  ONLP_PSU_STATUS_UNPLUGGED;
    } 
    else {
        info->status |= ONLP_PSU_STATUS_PRESENT;
    }

    /* Get PW Good status */
    if(info->status & ONLP_PSU_STATUS_PRESENT) {
        /* Get BMC PSU attributes */
        if (local_id == ONLP_PSU_1) {
            attr_pwrok = BMC_ATTR_ID_PSU1_PWROK;
            attr_vin = BMC_ATTR_ID_VIN_PSU1;
            attr_vout = BMC_ATTR_ID_VOUT_PSU1;
            attr_iin = BMC_ATTR_ID_IIN_PSU1;
            attr_iout = BMC_ATTR_ID_IOUT_PSU1;
            attr_pin = BMC_ATTR_ID_PIN_PSU1;
            attr_pout = BMC_ATTR_ID_POUT_PSU1;
        } 
        else if (local_id ==ONLP_PSU_2) {
            attr_pwrok = BMC_ATTR_ID_PSU2_PWROK;
            attr_vin = BMC_ATTR_ID_VIN_PSU2;
            attr_vout = BMC_ATTR_ID_VOUT_PSU2;
            attr_iin = BMC_ATTR_ID_IIN_PSU2;
            attr_iout = BMC_ATTR_ID_IOUT_PSU2;
            attr_pin = BMC_ATTR_ID_PIN_PSU2;
            attr_pout = BMC_ATTR_ID_POUT_PSU2;
        }
        else {
            AIM_LOG_ERROR("[ERROR] Incorrect PSU local_id(%d), while getting PWR ATTR\r\n", local_id); 
            return ONLP_STATUS_E_UNSUPPORTED;
        }   

        // Get PW Good status form BMC
        ONLP_TRY(read_bmc_sensor(attr_pwrok , PSU_SENSOR, &pw_good));

        if ((int)(pw_good) != PSU_STATUS_PWR_GD) {
            info->status |= ONLP_PSU_STATUS_FAILED;
        } 
        else {
            info->status &= ~ONLP_PSU_STATUS_FAILED;
        }

        /* Get power vin status */
        ONLP_TRY(read_bmc_sensor(attr_vin , PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->mvin = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_VIN;
        }

        /* Get power vout status */
        ONLP_TRY(read_bmc_sensor(attr_vout, PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->mvout = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_VOUT;
        }

        /* Get power iin status */
        ONLP_TRY(read_bmc_sensor(attr_iin, PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->miin = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_IIN;
        }

        /* Get power iout status */
        ONLP_TRY(read_bmc_sensor(attr_iout, PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->miout = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_IOUT;
        }

        /* Get power in */  
        ONLP_TRY(read_bmc_sensor(attr_pin, PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->mpin = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_PIN;
        }
        
        /* Get power out */ 
        ONLP_TRY(read_bmc_sensor(attr_pout, PSU_SENSOR, &data));
        if(BMC_ATTR_INVALID_VAL != (int)(data)) {
            info->mpout = (int) (data*1000);
            info->caps |= ONLP_PSU_CAPS_POUT;
        }

        /* Get FRU (model/serial) */
        ONLP_TRY(update_adv_psui_fru_info(local_id, info));
    }
    return ONLP_STATUS_OK;
}

int get_std_psu_present(int *pw_exist, int exist_offset, int i2c_bus, int psu_mask)
{
    int psu_pres;

    psu_pres = onlp_i2c_readb(i2c_bus, PSU_STATE_REG, 0x0, ONLP_I2C_F_FORCE);
    if (psu_pres < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    *pw_exist = (((psu_pres >> exist_offset) & psu_mask) ? 0 : 1);
    return ONLP_STATUS_OK;
}

int get_std_psu_pwgood(int *pw_good, int good_offset, int i2c_bus, int psu_mask)
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

static int get_std_psu_eeprom(onlp_psu_info_t* info, int id)
{
    uint8_t data[256];
    char eeprom_path[128];
    int data_len, i, rc;
    memset(data, 0, sizeof(data));
    memset(eeprom_path, 0, sizeof(eeprom_path));

    if (id == ONLP_PSU_1) {
        rc = onlp_file_read(data, sizeof(data), &data_len, PSU1_EEPROM_PATH);
    } 
    else if (id == ONLP_PSU_2) {
        rc = onlp_file_read(data, sizeof(data), &data_len, PSU2_EEPROM_PATH);
    }
    else {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (rc == ONLP_STATUS_OK) {
        i = 11;

        /* Manufacturer Name */
        data_len = (data[i]&0x3f);
        i++;
        i += data_len;

        /* Product Name */
        data_len = (data[i]&0x3f);
        i++;
        memcpy(info->model, (char *) &(data[i]), data_len);
        i += data_len;

        /* Product part,model number */
        data_len = (data[i]&0x3f);
        i++;
        i += data_len;

        /* Product Version */
        data_len = (data[i]&0x3f);
        i++;
        i += data_len;

        /* Product Serial Number */
        data_len = (data[i]&0x3f);
        i++;
        memcpy(info->serial, (char *) &(data[i]), data_len);
    } 
    else {
        strcpy(info->model, "NA");
        strcpy(info->serial, "NA");
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_iout(onlp_psu_info_t* info, int i2c_bus)
{
    int value;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    memset(result, 0, sizeof(result));
    double dvalue;

    value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_IOUT_OFFSET, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp) {
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
        }
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_IOUT;
        info->miout = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_pin(onlp_psu_info_t* info, int i2c_bus)
{
    int value;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    memset(result, 0, sizeof(result));
    double dvalue;

    value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_PIN_OFFSET, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp) {
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
        }
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_PIN;
        info->mpin = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_pout(onlp_psu_info_t* info, int i2c_bus)
{
    int value;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    memset(result, 0, sizeof(result));
    double dvalue;

    value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_POUT_OFFSET, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp) {
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
        }
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_POUT;
        info->mpout = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_vout(onlp_psu_info_t* info, int i2c_bus)
{
    int v_value = 0;
    int n_value = 0;
    unsigned int temp = 0;
    char result[32];
    double dvalue;
    memset(result, 0, sizeof(result));

    n_value = onlp_i2c_readb(i2c_bus, PSU_REG, PSU_VOUT_OFFSET1, ONLP_I2C_F_FORCE);
    if (n_value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    v_value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_VOUT_OFFSET2, ONLP_I2C_F_FORCE);
    if (v_value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    if (n_value & 0x10) {
        n_value = 0xF0 + (n_value & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp)
            snprintf(result, sizeof(result), "%d.%04d", v_value/temp, ((v_value%temp)*10000)/temp);
    } 
    else {
        snprintf(result, sizeof(result), "%d", (v_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_VOUT;
        info->mvout = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_iin(onlp_psu_info_t* info, int i2c_bus)
{
    int value;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    memset(result, 0, sizeof(result));
    double dvalue;

    value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_IIN_OFFSET, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp) {
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
        }
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_IIN;
        info->miin = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_vin(onlp_psu_info_t* info, int i2c_bus)
{
    int value;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    memset(result, 0, sizeof(result));
    double dvalue;

    value = onlp_i2c_readw(i2c_bus, PSU_REG, PSU_VIN_OFFSET, ONLP_I2C_F_FORCE);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (value & 0x07FF);
    if ((value & 0x8000)&&(y_value)) {
        n_value = 0xF0 + (((value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1<<n_value);
        if (temp) {
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
        }
    } 
    else {
        n_value = (((value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    dvalue = atof((const char *)result);
    if (dvalue > 0.0) {
        info->caps |= ONLP_PSU_CAPS_VIN;
        info->mvin = (int)(dvalue * 1000);
    }

    return ONLP_STATUS_OK;
}

static int get_std_psu_status_info(int local_id, onlp_psu_info_t *info)
{   
    int pw_exist, exist_offset;
    int pw_good, good_offset;
    int rc, psu_mask, i2c_bus;

    *info = std_psu_info[local_id];

    if (local_id == ONLP_PSU_1) {
        i2c_bus = I2C_BUS_PSU1;
        exist_offset = PSU1_PRESENT_OFFSET;
        good_offset = PSU1_PWGOOD_OFFSET;
    } 
    else if (local_id == ONLP_PSU_2) {
        i2c_bus = I2C_BUS_PSU2;
        exist_offset = PSU2_PRESENT_OFFSET;
        good_offset = PSU2_PWGOOD_OFFSET;
    } 
    else {
        return ONLP_STATUS_E_INTERNAL;
    }

    psu_mask = PSU_MUX_MASK;
    
     /* Get power present status */
    if ((rc = get_std_psu_present(&pw_exist, exist_offset, I2C_BUS_0, psu_mask))
            != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    if (pw_exist != PSU_STATUS_PRSNT) {
        info->status &= ~ONLP_PSU_STATUS_PRESENT;
        info->status |=  ONLP_PSU_STATUS_UNPLUGGED;
        return ONLP_STATUS_OK;
    }    
    info->status |= ONLP_PSU_STATUS_PRESENT;
    
    /* Get power good status */
    if ((rc = get_std_psu_pwgood(&pw_good, good_offset, I2C_BUS_0, psu_mask)) 
            != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }   
    
    if (pw_good != PSU_STATUS_POWER_GOOD) {        
        info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    } 
    else {
        info->status &= ONLP_PSU_STATUS_PRESENT;
    }
    
    /* Get power eeprom status */
    if ((rc = get_std_psu_eeprom(info, local_id)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    /* Get power iout status */
    if ((rc = get_std_psu_iout(info, i2c_bus)) != ONLP_STATUS_OK) { 
        return ONLP_STATUS_E_INTERNAL;
    }
    
    /* Get power pout status */
    if ((rc = get_std_psu_pout(info, i2c_bus)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    /* Get power pin status */
    if ((rc = get_std_psu_pin(info, i2c_bus)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get power vout status */
    if ((rc = get_std_psu_vout(info, i2c_bus)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get power iin status */
    if ((rc = get_std_psu_iin(info, i2c_bus)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Get power vin status */
    if ((rc = get_std_psu_vin(info, i2c_bus)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }
    
    return ONLP_STATUS_OK;
}
