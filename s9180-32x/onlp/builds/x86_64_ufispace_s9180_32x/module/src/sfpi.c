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
 *      SFP Platform Implementation Interface.
 *
 ***********************************************************/
#include "platform_lib.h"

#define MGMT_NUM              0
#define SFP_NUM               2
#define QSFP_NUM              32
#define QSFPDD_NUM            0
#define QSFPX_NUM             (QSFP_NUM+QSFPDD_NUM)
#define PORT_NUM              (SFP_NUM+QSFPX_NUM+MGMT_NUM)

#define SYSFS_EEPROM         "eeprom"
#define EEPROM_ADDR (0x50)

/* ufispace APIs */
static int get_qsfp_present(int port, int *pres_val);
static int port_to_gpio_num(int port, onlp_sfp_control_t control);
static int xfr_port_to_eeprom_bus(int port);

typedef enum port_type_e {
    TYPE_SFP = 0,
    TYPE_QSFP,
    TYPE_QSFPDD,
    TYPE_MGMT,
    TYPE_UNNKOW,
    TYPE_MAX,
} port_type_t;

typedef struct
{
    int abs;
    int lpmode;
    int reset;  
    int rxlos;
    int txfault;
    int txdis;
    int eeprom_bus;
    int port_type;
    unsigned int cpld_bit;
} port_attr_t;

static const port_attr_t port_attr[] = {
/* port       abs    lpmode   reset    rxlos    txfault   txdis   eeprom_bus   type */
    [1]   =  {14     ,94      ,126      ,-1      ,-1      ,-1      ,10          ,TYPE_QSFP},
    [2]   =  {15     ,95      ,127      ,-1      ,-1      ,-1      ,9           ,TYPE_QSFP},
    [3]   =  {12     ,92      ,124      ,-1      ,-1      ,-1      ,12          ,TYPE_QSFP},
    [4]   =  {13     ,93      ,125      ,-1      ,-1      ,-1      ,11          ,TYPE_QSFP},
    [5]   =  {10     ,90      ,122      ,-1      ,-1      ,-1      ,14          ,TYPE_QSFP},
    [6]   =  {11     ,91      ,123      ,-1      ,-1      ,-1      ,13          ,TYPE_QSFP},
    [7]   =  {8      ,88      ,130      ,-1      ,-1      ,-1      ,16          ,TYPE_QSFP},
    [8]   =  {9      ,89      ,129      ,-1      ,-1      ,-1      ,15          ,TYPE_QSFP},
    [9]   =  {6      ,86      ,126      ,-1      ,-1      ,-1      ,18          ,TYPE_QSFP},
    [10]  =  {7      ,87      ,127      ,-1      ,-1      ,-1      ,17          ,TYPE_QSFP},
    [11]  =  {4      ,84      ,116      ,-1      ,-1      ,-1      ,20          ,TYPE_QSFP},
    [12]  =  {5      ,85      ,117      ,-1      ,-1      ,-1      ,19          ,TYPE_QSFP},
    [13]  =  {2      ,82      ,114      ,-1      ,-1      ,-1      ,22          ,TYPE_QSFP},
    [14]  =  {3      ,83      ,115      ,-1      ,-1      ,-1      ,21          ,TYPE_QSFP},
    [15]  =  {0      ,80      ,112      ,-1      ,-1      ,-1      ,24          ,TYPE_QSFP},
    [16]  =  {1      ,81      ,113      ,-1      ,-1      ,-1      ,23          ,TYPE_QSFP},
    [17]  =  {30     ,110     ,142      ,-1      ,-1      ,-1      ,26          ,TYPE_QSFP},
    [18]  =  {31     ,111     ,143      ,-1      ,-1      ,-1      ,25          ,TYPE_QSFP},
    [19]  =  {28     ,108     ,140      ,-1      ,-1      ,-1      ,28          ,TYPE_QSFP},
    [20]  =  {29     ,109     ,141      ,-1      ,-1      ,-1      ,27          ,TYPE_QSFP},
    [21]  =  {26     ,106     ,138      ,-1      ,-1      ,-1      ,30          ,TYPE_QSFP},
    [22]  =  {27     ,107     ,139      ,-1      ,-1      ,-1      ,29          ,TYPE_QSFP},
    [23]  =  {24     ,104     ,136      ,-1      ,-1      ,-1      ,32          ,TYPE_QSFP},
    [24]  =  {25     ,105     ,137      ,-1      ,-1      ,-1      ,31          ,TYPE_QSFP},
    [25]  =  {22     ,102     ,134      ,-1      ,-1      ,-1      ,34          ,TYPE_QSFP},
    [26]  =  {23     ,103     ,135      ,-1      ,-1      ,-1      ,33          ,TYPE_QSFP},
    [27]  =  {20     ,100     ,132      ,-1      ,-1      ,-1      ,36          ,TYPE_QSFP},
    [28]  =  {21     ,101     ,133      ,-1      ,-1      ,-1      ,35          ,TYPE_QSFP},
    [29]  =  {18     ,98      ,130      ,-1      ,-1      ,-1      ,38          ,TYPE_QSFP},
    [30]  =  {19     ,99      ,131      ,-1      ,-1      ,-1      ,37          ,TYPE_QSFP},
    [31]  =  {16     ,96      ,128      ,-1      ,-1      ,-1      ,40          ,TYPE_QSFP},
    [32]  =  {17     ,97      ,129      ,-1      ,-1      ,-1      ,39          ,TYPE_QSFP},
    //SFP  
    [33]  =  {79     ,-1      ,-1       ,73      ,77      ,75      ,45          ,TYPE_SFP },
    [34]  =  {78     ,-1      ,-1       ,72      ,76      ,74      ,46          ,TYPE_SFP },
};

#define IS_SFP(_port)         (port_attr[_port].port_type == TYPE_SFP || port_attr[_port].port_type == TYPE_MGMT)
#define IS_QSFPX(_port)       (port_attr[_port].port_type == TYPE_QSFPDD || port_attr[_port].port_type == TYPE_QSFP)
#define IS_QSFP(_port)        (port_attr[_port].port_type == TYPE_QSFP)
#define IS_QSFPDD(_port)      (port_attr[_port].port_type == TYPE_QSFPDD)

#define VALIDATE_PORT(p) { if ((p <= 0) || (p > PORT_NUM)) return ONLP_STATUS_E_PARAM; }
#define VALIDATE_SFP_PORT(p) { if (!IS_SFP(p)) return ONLP_STATUS_E_PARAM; }

/**
 * @brief Initialize the SFPI subsystem.
 */
int onlp_sfpi_init(void)
{
    init_lock();
    return ONLP_STATUS_OK;
}

/**
 * @brief Get the bitmap of SFP-capable port numbers.
 * @param bmap [out] Receives the bitmap.
 */
int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    for(p = 1; p <= PORT_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

/**
 * @brief Determine if an SFP is present.
 * @param port The port number.
 * @returns 1 if present
 * @returns 0 if absent
 * @returns An error condition.
 */
int onlp_sfpi_is_present(int port)
{
    int status;

    if (get_qsfp_present(port, &status) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    return status;

}

/**
 * @brief Return the presence bitmap for all SFP ports.
 * @param dst Receives the presence bitmap.
 */
int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p = 1;
    int rc = 0;

    for (p = 1; p <= PORT_NUM; p++) {
        if ((rc = onlp_sfpi_is_present(p)) < 0) {
            return rc;
        }
        AIM_BITMAP_MOD(dst, p, rc);
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Return the RX_LOS bitmap for all SFP ports.
 * @param dst Receives the RX_LOS bitmap.
 */
int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p=0, value=0;

    for(p = 1; p <= PORT_NUM; p++) {
        if(IS_SFP(p)) {
            if(onlp_sfpi_control_get(p, ONLP_SFP_CONTROL_RX_LOS, &value) != ONLP_STATUS_OK) {
                AIM_BITMAP_MOD(dst, p, 0); 
            } 
            else {
                AIM_BITMAP_MOD(dst, p, value); 
            }
        } 
        else {
            AIM_BITMAP_MOD(dst, p, 0);  //set default value for port which has no cap
        }
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Read the SFP EEPROM.
 * @param port The port number.
 * @param data Receives the SFP data.
 */
int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int size = 0, bus = 0, rc = 0;

    VALIDATE_PORT(port);

    memset(data, 0, 256);
    bus = xfr_port_to_eeprom_bus(port);

    if((rc = onlp_file_read(data, 256, &size, SYS_FMT, bus, EEPROM_ADDR, SYSFS_EEPROM)) < 0) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)", port);
        AIM_LOG_ERROR(SYS_FMT, bus, EEPROM_ADDR, SYSFS_EEPROM);

        check_and_do_i2c_mux_reset(port);
        return rc;
    }

    if (size != 256) {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d), size is different!", port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Read a byte from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 */
int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int rc = 0;
    int bus = xfr_port_to_eeprom_bus(port);

    if (onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if ((rc = onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }
    return rc;
}

/**
 * @brief Write a byte to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    VALIDATE_PORT(port);
    int rc = 0;
    int bus = xfr_port_to_eeprom_bus(port);

    if (onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if ((rc = onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }
    return rc;
}

/**
 * @brief Read a word from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 * @returns The word if successful, error otherwise.
 */
int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    VALIDATE_PORT(port);
    int rc = 0;
    int bus = xfr_port_to_eeprom_bus(port);

    if(onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if((rc = onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }
    return rc;
}

/**
 * @brief Write a word to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    VALIDATE_PORT(port);
    int rc = 0;
    int bus = xfr_port_to_eeprom_bus(port);

    if(onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if((rc = onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }
    return rc;
}

/**
 * @brief Read from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 * @returns The data if successful, error otherwise.
 */
int onlp_sfpi_dev_read(int port, uint8_t devaddr, uint8_t addr, uint8_t* rdata, int size)
{
    VALIDATE_PORT(port);
    int bus = xfr_port_to_eeprom_bus(port);

    if (onlp_sfpi_is_present(port) != 1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if (onlp_i2c_block_read(bus, devaddr, addr, size, rdata, ONLP_I2C_F_FORCE) < 0) {
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Write to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_write(int port, uint8_t devaddr, uint8_t addr, uint8_t* data, int size)
{
    VALIDATE_PORT(port);
    int rc = 0;
    int bus = xfr_port_to_eeprom_bus(port);

    if (onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    if ((rc = onlp_i2c_write(bus, devaddr, addr, size, data, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }

    return rc;
}

/**
 * @brief Read the SFP DOM EEPROM.
 * @param port The port number.
 * @param data Receives the SFP data.
 */
int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    char eeprom_path[512];
    FILE* fp;
    int bus = 0;

    /* 
    ** sfp dom is on 0x51 (2nd 256 bytes)
    ** qsfp dom is on lower page 0x00
    ** qsfpdd 2.0 dom is on lower page 0x00
    ** qsfpdd 3.0 and later dom and above is on lower page 0x00 and higher page 0x17 
    */
    VALIDATE_SFP_PORT(port);

    if (onlp_sfpi_is_present(port) !=  1) {
        AIM_LOG_INFO("sfp module (port=%d) is absent.\n", port);
        return ONLP_STATUS_OK;
    }

    memset(data, 0, 256);
    memset(eeprom_path, 0, sizeof(eeprom_path));

    /* set eeprom_path */
    bus = xfr_port_to_eeprom_bus(port);
    snprintf(eeprom_path, sizeof(eeprom_path), SYS_FMT, bus, EEPROM_ADDR, SYSFS_EEPROM);

    /* read eeprom */
    fp = fopen(eeprom_path, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, 256, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Returns whether or not the given control is suppport on the given port.
 * @param port The port number.
 * @param control The control.
 * @param rv [out] Receives 1 if supported, 0 if not supported.
 * @note This provided for convenience and is optional.
 * If you implement this function your control_set and control_get APIs
 * will not be called on unsupported ports.
 */
int onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv)
{
    VALIDATE_PORT(port);

    /* set unsupported as default value */
    *rv=0;

    switch (control) {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
        case ONLP_SFP_CONTROL_LP_MODE:
            if (IS_QSFPX(port)) {
                *rv = 1;
            }
            break;
        case ONLP_SFP_CONTROL_RX_LOS:
        case ONLP_SFP_CONTROL_TX_FAULT:
        case ONLP_SFP_CONTROL_TX_DISABLE:
            if (IS_SFP(port)) {
                *rv = 1;
            }
            break;
        default:
            *rv = 0;
            break;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Set an SFP control.
 * @param port The port.
 * @param control The control.
 * @param value The value.
 */
int onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    int rc = 0;
    int gpio_num = 0;

    VALIDATE_PORT(port);

    /* check control is valid for this port */
    switch(control)
    {
        case ONLP_SFP_CONTROL_RESET:
            {
                if (IS_QSFPX(port)) {
                    break;
                } else {
                    return ONLP_STATUS_E_UNSUPPORTED;
                }
            }
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                if (IS_SFP(port)) {
                    break;
                } else {
                    return ONLP_STATUS_E_UNSUPPORTED;
                }
            }
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                if (IS_QSFPX(port)) {
                    break;
                } else {
                    return ONLP_STATUS_E_UNSUPPORTED;
                }
            }
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    /* get gpio_num */
    if ((rc = port_to_gpio_num(port, control)) < 0) {
        AIM_LOG_ERROR("port_to_gpio_num() failed, error=%d, port=%d, control=%d", rc, port, control);
        return rc;
    } 
    else {
        gpio_num = rc;
    }

    /* write gpio value */
    if ((rc = onlp_file_write_int(value, SYS_GPIO_FMT, gpio_num)) < 0) {
        AIM_LOG_ERROR("onlp_sfpi_control_set() failed, error=%d, sysfs=%s, gpio_num=%d", rc, SYS_GPIO_FMT, gpio_num);
        check_and_do_i2c_mux_reset (port);
        return rc;
    }

    return rc;
}

/**
 * @brief Get an SFP control.
 * @param port The port.
 * @param control The control
 * @param [out] value Receives the current value.
 */
int onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    int rc;
    int gpio_num = 0;

    VALIDATE_PORT(port);

    /* get gpio_num */
    if ((rc = port_to_gpio_num(port, control)) < 0) {
        AIM_LOG_ERROR("port_to_gpio_num() failed, error=%d, port=%d, control=%d", rc, port, control);
        return rc;
    } 
    else {
        gpio_num = rc;
    }

    //read gpio value
    if ((rc = read_file_hex(value, SYS_GPIO_FMT, gpio_num)) < 0) {
        AIM_LOG_ERROR("onlp_sfpi_control_get() failed, error=%d, sysfs=%s, gpio_num=%d", rc, SYS_GPIO_FMT, gpio_num);
        check_and_do_i2c_mux_reset(port);
        return rc;
    }

    /* reverse bit */
    if (control == ONLP_SFP_CONTROL_RESET_STATE || control == ONLP_SFP_CONTROL_RX_LOS) {
        *value = !(*value);
    }

    return rc;
}

static int get_qsfp_present(int port, int *pres_val)
{
    int status, rc, gpio_num;

    if (port >= 1 && port <= 16) {
        gpio_num = 496 + ((port - 1) ^ 1);
    } 
    else if (port >= 17 && port <= 32) {
        gpio_num = 464 + ((port - 1) ^ 1);
    } 
    else if (port == 33) {
        gpio_num = 432;
    } 
    else if (port == 34) {
        gpio_num = 433;
    }
    else {
        return ONLP_STATUS_E_INVALID;
    }

    if ((rc = onlp_file_read_int(&status, "/sys/class/gpio/gpio%d/value", gpio_num)) != ONLP_STATUS_OK) {
        return ONLP_STATUS_E_INTERNAL;
    }

    *pres_val = status;

    /* reverse bit */
    //*pres_val = !(*pres_val);

    return ONLP_STATUS_OK;
}

static int port_to_gpio_num(int port, onlp_sfp_control_t control)
{
    int gpio_max = 0;
    int gpio_num = -1;

    ONLP_TRY(get_gpio_max(&gpio_max));

    switch(control)
    {
        case ONLP_SFP_CONTROL_RESET:
        case ONLP_SFP_CONTROL_RESET_STATE:
            {
                gpio_num = gpio_max - port_attr[port].reset;
                break;
            }
        case ONLP_SFP_CONTROL_RX_LOS:
            {
                gpio_num = gpio_max - port_attr[port].rxlos;
                break;
            }
        case ONLP_SFP_CONTROL_TX_FAULT:
            {
                gpio_num = gpio_max - port_attr[port].txfault;
                break;
            }
        case ONLP_SFP_CONTROL_TX_DISABLE:
            {
                gpio_num = gpio_max - port_attr[port].txdis;
                break;
            }
        case ONLP_SFP_CONTROL_LP_MODE:
            {
                gpio_num = gpio_max - port_attr[port].lpmode;
                break;
            }
        default:
            gpio_num=-1;
    }

    if (gpio_num<0 || gpio_num>gpio_max) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    return gpio_num;
}

static int xfr_port_to_eeprom_bus(int port)
{
    int bus = -1;

    bus=port_attr[port].eeprom_bus;
    return bus;
}
