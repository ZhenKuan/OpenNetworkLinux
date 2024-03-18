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
#include <unistd.h>
#include <sys/io.h>
#include <onlplib/shlocks.h>
#include <onlp/oids.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "platform_lib.h"

bmc_info_t bmc_cache[] =
{
    [BMC_ATTR_ID_Temp_BMC]           = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_BMC             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_Temp_MAC]           = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_MAC             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_Temp_MAC_Front]     = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_MAC_Front       ,BMC_DATA_FLOAT         ,0},        
    [BMC_ATTR_ID_Temp_MAC_Rear]      = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_MAC_Rear        ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_Battery]       = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_Battery         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_OOB_conn]      = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_OOB_conn        ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_SFP_conn]      = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_SFP_conn        ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_PCIE_conn]     = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_PCIE_conn       ,BMC_DATA_FLOAT         ,0},        
    [BMC_ATTR_ID_Temp_PSU1_AMB]      = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_PSU1_AMB        ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_PSU1_HS]       = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_PSU1_HS         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_PSU2_AMB]      = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_PSU2_AMB        ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_Temp_PSU2_HS]       = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Temp_PSU2_HS         ,BMC_DATA_FLOAT         ,0},    
    //-------------------------------------------------------------------------------------------------------------------------------    
    [BMC_ATTR_ID_FAN_1_F]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_1_F              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_2_F]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_2_F              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_3_F]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_3_F              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_4_F]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_4_F              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_1_R]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_1_R              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_2_R]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_2_R              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_3_R]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_3_R              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_FAN_4_R]            = {HW_PLAT_ALL         ,BMC_ATTR_NAME_FAN_4_R              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_Fan_PSU1]           = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Fan_PSU1             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_Fan_PSU2]           = {HW_PLAT_ALL         ,BMC_ATTR_NAME_Fan_PSU2             ,BMC_DATA_FLOAT         ,0},
    //-------------------------------------------------------------------------------------------------------------------------------
    [BMC_ATTR_ID_Fan1_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_Fan1_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_Fan2_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_Fan2_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_Fan3_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_Fan3_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_Fan4_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_Fan4_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_PSU1_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU1_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_PSU2_PRSNT]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU2_PRSNT            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_CPU_PRSNT]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_CPU_PRSNT             ,BMC_DATA_BOOL          ,0},
    //-------------------------------------------------------------------------------------------------------------------------------   
    [BMC_ATTR_ID_ALL_PWRGOOD]        = {HW_PLAT_ALL        ,BMC_ATTR_NAME_ALL_PWRGOOD           ,BMC_DATA_FLOAT         ,0},    
    //-------------------------------------------------------------------------------------------------------------------------------
    [BMC_ATTR_ID_THERMAL_ALERT]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_THERMAL_ALERT         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_HWM_P0V9_VSEN]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_HWM_P0V9_VSEN         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_HWM_VDDC_VSEN]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_HWM_VDDC_VSEN         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_HWM_P2V5_VSEN]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_HWM_P2V5_VSEN         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_P3V3_VSEN]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P3V3_VSEN             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_BMC_P1V26_VSEN]     = {HW_PLAT_ALL        ,BMC_ATTR_NAME_BMC_P1V26_VSEN        ,BMC_DATA_FLOAT         ,0},        
    [BMC_ATTR_ID_BMC_P1V538_VSEN]    = {HW_PLAT_ALL        ,BMC_ATTR_NAME_BMC_P1V538_VSEN       ,BMC_DATA_FLOAT         ,0},       
    [BMC_ATTR_ID_BMC_P3V3_VSEN]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_BMC_P3V3_VSEN         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_BMC_P5V_VSEN]       = {HW_PLAT_ALL        ,BMC_ATTR_NAME_BMC_P5V_VSEN          ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_P5V_SB_MON]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P5V_SB_MON            ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_P3V3_SB_MON]        = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P3V3_SB_MON           ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_PERI_P3V3_MON]      = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PERI_P3V3_MON         ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_P3V3_MON]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P3V3_MON              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_P2V5_MON]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P2V5_MON              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_P1V2_MON]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P1V2_MON              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_P0V9_MON]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_P0V9_MON              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_VDD_CORE_MON]       = {HW_PLAT_ALL        ,BMC_ATTR_NAME_VDD_CORE_MON          ,BMC_DATA_FLOAT         ,0},    
    [BMC_ATTR_ID_PSU1_INT]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU1_INT              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_PSU2_INT]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU2_INT              ,BMC_DATA_FLOAT         ,0},
    //-------------------------------------------------------------------------------------------------------------------------------
    [BMC_ATTR_ID_PSU1_PWROK]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU1_PWROK            ,BMC_DATA_BOOL          ,0},    
    [BMC_ATTR_ID_PSU2_PWROK]         = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PSU2_PWROK            ,BMC_DATA_BOOL          ,0},    
    //-------------------------------------------------------------------------------------------------------------------------------
    [BMC_ATTR_ID_PIN_PSU1]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PIN_PSU1              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_PIN_PSU2]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_PIN_PSU2              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_POUT_PSU1]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_POUT_PSU1             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_POUT_PSU2]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_POUT_PSU2             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_VIN_PSU1]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_VIN_PSU1              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_VIN_PSU2]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_VIN_PSU2              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_VOUT_PSU1]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_VOUT_PSU1             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_VOUT_PSU2]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_VOUT_PSU2             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_IIN_PSU1]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_IIN_PSU1              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_IIN_PSU2]           = {HW_PLAT_ALL        ,BMC_ATTR_NAME_IIN_PSU2              ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_IOUT_PSU1]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_IOUT_PSU1             ,BMC_DATA_FLOAT         ,0},
    [BMC_ATTR_ID_IOUT_PSU2]          = {HW_PLAT_ALL        ,BMC_ATTR_NAME_IOUT_PSU2             ,BMC_DATA_FLOAT         ,0},
};

static bmc_fru_t bmc_fru_cache[] =
{
    [BMC_FRU_IDX_ONLP_PSU_1] = {
        .bmc_fru_id = 1,
        .init_done = 0,
        .cache_files = "/tmp/bmc_fru_cache_1",
        .vendor   = {BMC_FRU_KEY_MANUFACTURER ,""},
        .name     = {BMC_FRU_KEY_NAME         ,""},
        .part_num = {BMC_FRU_KEY_PART_NUMBER  ,""},
        .serial   = {BMC_FRU_KEY_SERIAL       ,""},
    },
    [BMC_FRU_IDX_ONLP_PSU_2] = {
        .bmc_fru_id = 2,
        .init_done = 0,
        .cache_files = "/tmp/bmc_fru_cache_2",
        .vendor   = {BMC_FRU_KEY_MANUFACTURER ,""},
        .name     = {BMC_FRU_KEY_NAME         ,""},
        .part_num = {BMC_FRU_KEY_PART_NUMBER  ,""},
        .serial   = {BMC_FRU_KEY_SERIAL       ,""},
    },
};

static onlp_shlock_t* onlp_lock = NULL;

#define ONLP_LOCK() \
    do { \
        onlp_shlock_take(onlp_lock); \
    } while(0)

#define ONLP_UNLOCK() \
    do { \
        onlp_shlock_give(onlp_lock); \
    } while(0)

#define LOCK_MAGIC 0xA2B4C6D8

void init_lock()
{
    static int sem_inited = 0;
    if(!sem_inited) {
        onlp_shlock_create(LOCK_MAGIC, &onlp_lock, "bmc-file-lock");
        sem_inited = 1;
        check_and_do_i2c_mux_reset(-1);
    }
}

/**
 * @brief Get board version
 * @param board [out] board data struct
 */
int get_board_version(board_t *board)
{
    int rv = ONLP_STATUS_OK;

    if(board == NULL) {
        return ONLP_STATUS_E_INVALID;
    }

    //Get HW Version
    if(read_file_hex(&board->hw_rev, PATH_MB_CPLD "cpld_hw_rev") != ONLP_STATUS_OK ||
        read_file_hex(&board->board_id, PATH_MB_CPLD "cpld_board_id") != ONLP_STATUS_OK ||
        read_file_hex(&board->hw_build, PATH_MB_CPLD "cpld_build_rev") != ONLP_STATUS_OK ||
        read_file_hex(&board->model_id, PATH_MB_CPLD "cpld_model_id") != ONLP_STATUS_OK)
    {
        board->hw_rev = 0;
        board->board_id = 0;
        board->hw_build = 0;
        board->model_id = 0;
        rv = ONLP_STATUS_E_INVALID;
    }

    return rv;
}

int check_file_exist(char *file_path, long *file_time)
{
    struct stat file_info;

    if(stat(file_path, &file_info) == 0) {
        if(file_info.st_size == 0) {
            return 0;
        } 
        else {
            *file_time = file_info.st_mtime;
            return 1;
        }
    } 
    else {
        return 0;
    }
}

/**
 * @brief check bmc still alive
 * @returns ONLP_STATUS_OK         : bmc still alive
 *          ONLP_STATUS_E_INTERNAL : bmc not respond
 */
int check_bmc_alive(void)
{
    /**
     *   BMC detect timeout get from "ipmitool mc info" test.
     *   Test Case: Run 100 times of "ipmitool mc info" command and get the execution times.
     *              We take 3s as The detect timeout value,
     *              since the execution times value is between 0.015s - 0.062s.
     */
    char* bmc_dect = "timeout 3s ipmitool mc info > /dev/null 2>&1";

    int retry = 0, retry_max = 2;
    for (retry = 0; retry < retry_max; ++retry) {
        int ret = 0;
        if((ret=system(bmc_dect)) != 0) {
            if (retry == retry_max-1) {
                AIM_LOG_ERROR("%s() bmc detecting fail, retry=%d, ret=%d\r\n",
                    __func__, retry, ret);
                return ONLP_STATUS_E_INTERNAL;
            } 
            else {
                continue;
            }
        } 
        else {
            break;
        }
    }

    return ONLP_STATUS_OK;
}

int bmc_cache_expired_check(long last_time, long new_time, int cache_time)
{
    int bmc_cache_expired = 0;

    if(last_time == 0) {
        bmc_cache_expired = 1;
    } 
    else {
        if(new_time > last_time) {
            if((new_time - last_time) > cache_time) {
                bmc_cache_expired = 1;
            } 
            else {
                bmc_cache_expired = 0;
            }
        } 
        else if(new_time == last_time) {
            bmc_cache_expired = 0;
        } 
        else {
            bmc_cache_expired = 1;
        }
    }

    return bmc_cache_expired;
}

int read_bmc_sensor(int bmc_cache_index, int sensor_type, float *data)
{
    int cache_time = 0;
    int bmc_cache_expired = 0;
    int bmc_cache_change = 0;
    static long file_pre_time = 0;
    long file_last_time = 0;
    static int init_cache = 1;
    int rv = ONLP_STATUS_OK;

    switch(sensor_type) {
        case FAN_SENSOR:
            cache_time = FAN_CACHE_TIME;
            break;
        case PSU_SENSOR:
            cache_time = PSU_CACHE_TIME;
            break;
        case THERMAL_SENSOR:
            cache_time = THERMAL_CACHE_TIME;
            break;
    }

    board_t board = {0};
    ONLP_TRY(get_board_version(&board));

    ONLP_LOCK();

    if(check_file_exist(BMC_SENSOR_CACHE, &file_last_time)) {
        struct timeval new_tv = {0};
        gettimeofday(&new_tv, NULL);
        if(bmc_cache_expired_check(file_last_time, new_tv.tv_sec, cache_time)) {
            bmc_cache_expired = 1;
        } 
        else {
            if(file_pre_time != file_last_time) {
                file_pre_time = file_last_time;
                bmc_cache_change = 1;
            }
            bmc_cache_expired = 0;
        }
    } 
    else {
        bmc_cache_expired = 1;
    }

    //update cache
    if(bmc_cache_expired == 1 || init_cache == 1 || bmc_cache_change == 1) {
        if(bmc_cache_expired == 1) {
            // detect bmc status
            if(check_bmc_alive() != ONLP_STATUS_OK) {
                rv = ONLP_STATUS_E_INTERNAL;
                goto done;
            }

            // get bmc data
            char ipmi_cmd[1536] = {0};
            char bmc_token[1024] = {0};
            int i = 0;
            for(i = BMC_ATTR_ID_START; i <= BMC_ATTR_ID_LAST; i++) {
                int plat = 0;
                if(board.hw_rev == BRD_PROTO) {
                    plat = HW_PLAT_PROTO;
                }
                else if(board.hw_rev == BRD_ALPHA) {
                    plat = HW_PLAT_ALPHA;
                }
                else if(board.hw_rev == BRD_BETA) {
                    plat = HW_PLAT_BETA;
                }
                else if(board.hw_rev == BRD_PVT) {
                    plat = HW_PLAT_PVT;
                }
                else {
                    plat = HW_PLAT_PVT;
                }

                if(bmc_cache[i].plat & plat) {
                    char tmp_str[1024] = {0};
                    int copy_size = (sizeof(bmc_token) - strlen(bmc_token) - 1) >= 0? (sizeof(bmc_token) - strlen(bmc_token) - 1):0;
                    snprintf(tmp_str, sizeof(tmp_str), " %s", bmc_cache[i].name);
                    strncat(bmc_token, tmp_str, copy_size);
                }
            }

            snprintf(ipmi_cmd, sizeof(ipmi_cmd), CMD_BMC_SENSOR_CACHE, IPMITOOL_CMD_TIMEOUT, bmc_token);
            int retry = 0, retry_max = 2;
            for (retry = 0; retry < retry_max; ++retry) {
                int ret = 0;
                if((ret=system(ipmi_cmd)) != 0) {
                    if (retry == retry_max-1) {
                        AIM_LOG_ERROR("%s() write bmc sensor cache failed, retry=%d, cmd=%s, ret=%d\r\n",
                            __func__, retry, ipmi_cmd, ret);
                        rv = ONLP_STATUS_E_INTERNAL;
                        goto done;
                    } 
                    else {
                        continue;
                    }
                } 
                else {
                    break;
                }
            }
        }

        //read sensor from cache file and save to bmc_cache
        FILE *fp = NULL;
        fp = fopen (BMC_SENSOR_CACHE, "r");
        if(fp == NULL) {
            AIM_LOG_ERROR("%s() open file failed, file=%s\r\n", __func__, BMC_SENSOR_CACHE);
            rv = ONLP_STATUS_E_INTERNAL;
            goto done;
        }

        //read file line by line
        char line[BMC_FRU_LINE_SIZE] = {'\0'};
        while(fgets(line,BMC_FRU_LINE_SIZE, fp) != NULL) {
            int i = 0;
            char *line_ptr = line;
            char *token = NULL;

            //parse line into fields. fields[0]: fields name, fields[1]: fields value
            char line_fields[BMC_FIELDS_MAX][BMC_FRU_ATTR_KEY_VALUE_SIZE] = {{0}};
            while ((token = strsep (&line_ptr, ",")) != NULL) {
                sscanf (token, "%[^\n]", line_fields[i++]);
            }

            //save bmc_cache from fields
            for(i=BMC_ATTR_ID_START; i<=BMC_ATTR_ID_LAST; ++i) {
                if(strcmp(line_fields[0], bmc_cache[i].name) == 0) {
                    if(bmc_cache[i].data_type == BMC_DATA_BOOL) {
                        /* fan/psu present, got from bmc */
                        if( strstr(line_fields[4], "Present") != NULL ) {
                            bmc_cache[i].data = BMC_ATTR_STATUS_PRES;
                        } 
                        /* power good status, got from bmc */
                        else if( strstr(line_fields[4], "Enabled") != NULL ) {
                            bmc_cache[i].data = PSU_STATUS_POWER_GOOD;
                        } 
                        /* power good failed, got from bmc */
                        else if( strstr(line_fields[4], "Disabled") != NULL )  {
                            bmc_cache[i].data = PSU_STATUS_POWER_FAILED;
                        }
                        /* fan/psu absent, got from bmc */
                        else {
                            bmc_cache[i].data = BMC_ATTR_STATUS_ABS;
                        }
                    } 
                    else {
                        /* other attribute, got from bmc */
                        if(strcmp(line_fields[1], "") == 0) {
                            bmc_cache[i].data = BMC_ATTR_INVALID_VAL;
                        } 
                        else {
                            bmc_cache[i].data = atof(line_fields[1]);
                        }
                    }
                    break;
                }
            }
        }
        fclose(fp);
        init_cache = 0;
    }

    /* read from cache */
    *data = bmc_cache[bmc_cache_index].data;

done:
    ONLP_UNLOCK();
    return rv;
}

/**
 * @brief bmc fru read
 * @param fru_id The fru id
 * @param data [out] The fru information.
 */
int read_bmc_fru(int fru_id, bmc_fru_t *data)
{
    struct timeval new_tv;
    int cache_time = PSU_CACHE_TIME;
    int bmc_cache_expired = 0;
    int bmc_cache_change = 0;
    static long file_pre_time = 0;
    long file_last_time = 0;
    int rv = ONLP_STATUS_OK;

    if((data == NULL)) {
        return ONLP_STATUS_E_INTERNAL;
    }

    switch(fru_id){
        case BMC_FRU_IDX_ONLP_PSU_1:
        case BMC_FRU_IDX_ONLP_PSU_2:
            break;
        default:
            return ONLP_STATUS_E_INTERNAL;
    }

    bmc_fru_t *fru = &bmc_fru_cache[fru_id];

    ONLP_LOCK();

    if(check_file_exist(fru->cache_files, &file_last_time)) {
        gettimeofday(&new_tv, NULL);
        if(bmc_cache_expired_check(file_last_time, new_tv.tv_sec, cache_time)) {
            bmc_cache_expired = 1;
        } 
        else {
            if(file_pre_time != file_last_time) {
                file_pre_time = file_last_time;
                bmc_cache_change = 1;
            }
            bmc_cache_expired = 0;
        }
    } 
    else {
        bmc_cache_expired = 1;
    }

    /* update cache */
    if(bmc_cache_expired == 1 || fru->init_done == 0 || bmc_cache_change == 1) {
        /* get fru from ipmitool and save to cache file */
        if(bmc_cache_expired == 1) {

            memset(fru->vendor.val, '\0', sizeof(fru->vendor.val));
            memset(fru->name.val, '\0', sizeof(fru->name.val));
            memset(fru->part_num.val, '\0', sizeof(fru->part_num.val));
            memset(fru->serial.val, '\0', sizeof(fru->serial.val));

            /* detect bmc status */
            if(check_bmc_alive() != ONLP_STATUS_OK) {
                rv = ONLP_STATUS_E_INTERNAL;
                goto done;
            }

            /* get bmc data */
            char ipmi_cmd[1536] = {0};
            char fields[256]="";
            snprintf(fields, sizeof(fields), "-e '%s' -e '%s' -e '%s' -e '%s'",
                        BMC_FRU_KEY_MANUFACTURER, BMC_FRU_KEY_NAME ,BMC_FRU_KEY_PART_NUMBER, BMC_FRU_KEY_SERIAL);

            snprintf(ipmi_cmd, sizeof(ipmi_cmd), CMD_FRU_CACHE_SET, IPMITOOL_CMD_TIMEOUT, fru->bmc_fru_id, fields, fru->cache_files);
            int retry = 0, retry_max = 2;
            for (retry = 0; retry < retry_max; ++retry) {
                int ret = 0;
                if ((ret = system(ipmi_cmd)) != 0) {
                    if (retry == retry_max-1) {
                        AIM_LOG_ERROR("%s() write bmc fru cache failed, retry=%d, cmd=%s, ret=%d\r\n",
                            __func__, retry, ipmi_cmd, ret);
                        rv = ONLP_STATUS_E_INTERNAL;
                        goto done;
                    } 
                    else {
                        continue;
                    }
                } 
                else {
                    break;
                }
            }
        }

        /* read fru from cache file and save to bmc_fru_cache */
        FILE *fp = NULL;
        fp = fopen (fru->cache_files, "r");
        while(1) {
            char key[BMC_FRU_ATTR_KEY_VALUE_SIZE] = {'\0'};
            char val[BMC_FRU_ATTR_KEY_VALUE_SIZE] = {'\0'};
            if(fscanf(fp ,"%[^:]:%s\n", key, val) != 2) {
                break;
            }

            if(strcmp(key, BMC_FRU_KEY_MANUFACTURER) == 0) {
                memset(fru->vendor.val, '\0', sizeof(fru->vendor.val));
                strncpy(fru->vendor.val, val, strnlen(val, BMC_FRU_ATTR_KEY_VALUE_LEN));
            }

            if(strcmp(key, BMC_FRU_KEY_NAME) == 0) {
                memset(fru->name.val, '\0', sizeof(fru->name.val));
                strncpy(fru->name.val, val, strnlen(val, BMC_FRU_ATTR_KEY_VALUE_LEN));

            }

            if(strcmp(key, BMC_FRU_KEY_PART_NUMBER) == 0) {
                memset(fru->part_num.val, '\0', sizeof(fru->part_num.val));
                strncpy(fru->part_num.val, val, strnlen(val, BMC_FRU_ATTR_KEY_VALUE_LEN));
            }

            if(strcmp(key, BMC_FRU_KEY_SERIAL) == 0) {
                memset(fru->serial.val, '\0', sizeof(fru->serial.val));
                strncpy(fru->serial.val, val, strnlen(val, BMC_FRU_ATTR_KEY_VALUE_LEN));
            }

        }

        fclose(fp);

        fru->init_done = 1;

        /* Check output is correct */
        if (strnlen(fru->vendor.val, BMC_FRU_ATTR_KEY_VALUE_LEN) == 0 ) {
                strncpy(fru->vendor.val, COMM_STR_NOT_AVAILABLE, strnlen(COMM_STR_NOT_AVAILABLE, BMC_FRU_ATTR_KEY_VALUE_LEN));
        }


        if (strnlen(fru->name.val, BMC_FRU_ATTR_KEY_VALUE_LEN) == 0) {
                strncpy(fru->name.val, COMM_STR_NOT_AVAILABLE, strnlen(COMM_STR_NOT_AVAILABLE, BMC_FRU_ATTR_KEY_VALUE_LEN));
        }


        if (strnlen(fru->part_num.val, BMC_FRU_ATTR_KEY_VALUE_LEN) == 0) {
                strncpy(fru->part_num.val, COMM_STR_NOT_AVAILABLE, strnlen(COMM_STR_NOT_AVAILABLE, BMC_FRU_ATTR_KEY_VALUE_LEN));
        }


        if (strnlen(fru->serial.val, BMC_FRU_ATTR_KEY_VALUE_LEN) == 0) {
                strncpy(fru->serial.val, COMM_STR_NOT_AVAILABLE, strnlen(COMM_STR_NOT_AVAILABLE, BMC_FRU_ATTR_KEY_VALUE_LEN));
        }
    }

    /* read from cache */
    *data = *fru;

done:
    ONLP_UNLOCK();
    return rv;
}

int exec_cmd(char *cmd, char* out, int size) {
    FILE *fp;

    /* Open the command for reading. */
    fp = popen(cmd, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Failed to run command %s\r\n", cmd );
        return ONLP_STATUS_E_INTERNAL;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(out, size-1, fp) != NULL) {
    }

    /* close */
    pclose(fp);

    return ONLP_STATUS_OK;
}

int read_file_hex(int* value, const char* fmt, ...)
{
    int rv;
    va_list vargs;
    va_start(vargs, fmt);
    rv = vread_file_hex(value, fmt, vargs);
    va_end(vargs);
    return rv;
}

int vread_file_hex(int* value, const char* fmt, va_list vargs)
{
    int rv;
    uint8_t data[32];
    int len;
    rv = onlp_file_vread(data, sizeof(data), &len, fmt, vargs);
    if(rv < 0) {
        return rv;
    }
    //hex to int
    *value = (int) strtol((char *)data, NULL, 0);
    return 0;
}

/*
 * This function check the I2C bus statuas by using the sysfs of cpld_id,
 * If the I2C Bus is stcuk, do the i2c mux reset.
 */
void check_and_do_i2c_mux_reset(int port)
{
    // only support beta and later
    char cmd_buf[256] = {0};
    int ret = 0;
    
    snprintf(cmd_buf, sizeof(cmd_buf), I2C_STUCK_CHECK_CMD);
    ret = system(cmd_buf);
    if(ret != 0) {
        if(access(MUX_RESET_PATH, F_OK) != -1 ) {
            memset(cmd_buf, 0, sizeof(cmd_buf));
            snprintf(cmd_buf, sizeof(cmd_buf), "echo 0 > %s 2> /dev/null", MUX_RESET_PATH);
            ret = system(cmd_buf);
        }
    }
}

/* reg shift */
uint8_t shift_bit(uint8_t mask)
{
    int i=0, mask_one=1;

    for(i=0; i<8; ++i) {
        if((mask & mask_one) == 1)
            return i;
        else
            mask >>= 1;
    }

    return -1;
}

/* reg mask and shift */
uint8_t shift_bit_mask(uint8_t val, uint8_t mask)
{
    int shift=0;

    shift = shift_bit(mask);

    return (val & mask) >> shift;
}

uint8_t operate_bit(uint8_t reg_val, uint8_t bit, uint8_t bit_val)
{
    if(bit_val == 0)
        reg_val = reg_val & ~(1 << bit);
    else
        reg_val = reg_val | (1 << bit);
    return reg_val;
}

uint8_t get_bit_value(uint8_t reg_val, uint8_t bit)
{
    return (reg_val >> bit) & 0x1;
}

/* This function is for s9180-32x-adv*/
int get_thermal_thld(int thermal_local_id,  temp_thld_t *temp_thld) {

    if(temp_thld == NULL) {
        return ONLP_STATUS_E_INVALID;
    }

    if(bmc_enable){
        switch(thermal_local_id) {
            case ONLP_THERMAL_ID_Temp_BMC:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_ENV_BMC_ERROR;
                temp_thld->shutdown = THERMAL_ENV_BMC_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_MAC:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_MAC_ERROR;
                temp_thld->shutdown = THERMAL_MAC_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_MAC_Front:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_MAC_FRONT_ERROR;
                temp_thld->shutdown = THERMAL_MAC_FRONT_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_MAC_Rear:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_MAC_REAR_ERROR;
                temp_thld->shutdown = THERMAL_MAC_REAR_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_Battery:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_TEMP_BATTERY_ERROR;
                temp_thld->shutdown = THERMAL_TEMP_BATTERY_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_OOB_conn:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_TEMP_OOB_CONN_ERROR;
                temp_thld->shutdown = THERMAL_TEMP_OOB_CONN_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_SFP_conn:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_TEMP_SFP_CONN_ERROR;
                temp_thld->shutdown = THERMAL_TEMP_SFP_CONN_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_PCIE_conn:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_TEMP_PCIE_CONN_ERROR;
                temp_thld->shutdown = THERMAL_TEMP_PCIE_CONN_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_PSU1_AMB:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_PSU_AMB_ERROR;
                temp_thld->shutdown = THERMAL_PSU_AMB_SHUTDOWN;
                return ONLP_STATUS_OK;
            case ONLP_THERMAL_ID_Temp_PSU2_AMB:
                temp_thld->warning = THERMAL_STATE_NOT_SUPPORT;
                temp_thld->error = THERMAL_PSU_AMB_ERROR;
                temp_thld->shutdown = THERMAL_PSU_AMB_SHUTDOWN;
                return ONLP_STATUS_OK;
            default:
                break;
        }
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

/**
 * @brief Get gpio max
 * @param gpio_max [out] GPIO max
 */
int get_gpio_max(int *gpio_max)
{
    int rv = ONLP_STATUS_OK;

    if(gpio_max == NULL) {
        return ONLP_STATUS_E_INVALID;
    }

    if(read_file_hex(gpio_max, LPC_BSP_FMT "bsp_gpio_max") != ONLP_STATUS_OK)
    {
        *gpio_max = 511;
        rv = ONLP_STATUS_E_INVALID;
    }
    return rv;
}

/**
 * @brief Get bmc enable
 */
bool ufi_sysi_bmc_en_get(void)
{
    int value;

    if (onlp_file_read_int(&value, BMC_EN_FILE_PATH) < 0) {
        /* flag file not exist, default to not enable */
        return false;
    }

    /* 1 - enable, 0 - no enable */
    if (value) 
        return true;

    return false;
}
