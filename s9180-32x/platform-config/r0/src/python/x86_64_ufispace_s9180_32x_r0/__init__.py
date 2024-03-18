from onl.platform.base import *
from onl.platform.ufispace import *
from struct import *
from ctypes import c_int, sizeof
import os
import sys
import commands
import subprocess
import time
import fcntl
import yaml

def msg(s, fatal=False):
    sys.stderr.write(s)
    sys.stderr.flush()
    if fatal:
        sys.exit(1)

class IPMI_Ioctl(object):
    _IONONE = 0
    _IOWRITE = 1
    _IOREAD = 2

    IPMI_MAINTENANCE_MODE_AUTO = 0
    IPMI_MAINTENANCE_MODE_OFF  = 1
    IPMI_MAINTENANCE_MODE_ON   = 2

    IPMICTL_GET_MAINTENANCE_MODE_CMD = _IOREAD << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 30  # from ipmi.h
    IPMICTL_SET_MAINTENANCE_MODE_CMD = _IOWRITE << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 31  # from ipmi.h

    def __init__(self):
        self.ipmidev = None
        devnodes=["/dev/ipmi0", "/dev/ipmi/0", "/dev/ipmidev/0"]
        for dev in devnodes:
            try:
                self.ipmidev = open(dev, 'rw')
                break
            except Exception as e:
                print("open file {} failed, error: {}".format(dev, e))

    def __del__(self):
        if self.ipmidev is not None:
            self.ipmidev.close()

    def get_ipmi_maintenance_mode(self):
        input_buffer=pack('i',0)
        out_buffer=fcntl.ioctl(self.ipmidev, self.IPMICTL_GET_MAINTENANCE_MODE_CMD, input_buffer)
        maintanence_mode=unpack('i',out_buffer)[0]

        return maintanence_mode

    def set_ipmi_maintenance_mode(self, mode):
        fcntl.ioctl(self.ipmidev, self.IPMICTL_SET_MAINTENANCE_MODE_CMD, c_int(mode))

class OnlPlatform_x86_64_ufispace_s9180_32x_r0(OnlPlatformUfiSpace):
    PLATFORM='x86-64-ufispace-s9180-32x-r0'
    MODEL="S9180-32X"
    SYS_OBJECT_ID=".9180.32"
    PORT_COUNT=34
    PORT_CONFIG="32x100 + 2x10"
    LEVEL_INFO=1
    LEVEL_ERR=2
    BSP_VERSION='2.0.0'
    PATH_SYS_I2C_DEV_ATTR="/sys/bus/i2c/devices/{}-{:0>4x}/{}"
    PATH_SYS_GPIO = "/sys/class/gpio"
    PATH_LPC="/sys/devices/platform/x86_64_ufispace_s9180_32x_lpc"
    PATH_LPC_GRP_BSP=PATH_LPC+"/bsp"
    PATH_PORT_CONFIG="/lib/platform-config/"+PLATFORM+"/onl/port_config.yml"
    PATH_MB_CPLD="/sys/bus/i2c/devices/44-0033"


    def check_bmc_enable(self):
        retcode = subprocess.call('i2cget -y 0 0x76 0x0 2>/dev/null', shell=True)
        if retcode:
            return 1
        return 0

    def bsp_pr(self, pr_msg, level = LEVEL_INFO):
        if level == self.LEVEL_INFO:
            sysfs_bsp_logging = self.PATH_LPC_GRP_BSP+"/bsp_pr_info"
        elif level == self.LEVEL_ERR:
            sysfs_bsp_logging = self.PATH_LPC_GRP_BSP+"/bsp_pr_err"
        else:
            msg("Warning: BSP pr level is unknown, using LEVEL_INFO.\n")
            sysfs_bsp_logging = self.PATH_LPC_GRP_BSP+"/bsp_pr_info"

        if os.path.exists(sysfs_bsp_logging):
            with open(sysfs_bsp_logging, "w") as f:
                f.write(pr_msg)
        else:
            msg("Warning: bsp logging sys is not exist\n")

    def config_bsp_ver(self, bsp_ver):
        bsp_version_path=self.PATH_LPC_GRP_BSP+"/bsp_version"
        if os.path.exists(bsp_version_path):
            with open(bsp_version_path, "w") as f:
                f.write(bsp_ver)
    
    def get_board_version(self):
        board = {}
        board_attrs = {
            "hw_rev": {"sysfs": self.PATH_MB_CPLD + "/cpld_hw_rev"},
            "board_id": {"sysfs": self.PATH_MB_CPLD + "/cpld_board_id"},
            "hw_build": {"sysfs": self.PATH_MB_CPLD + "/cpld_build_rev"},
            "model_id": {"sysfs": self.PATH_MB_CPLD + "/cpld_model_id"},
        }
    
        for key, val in board_attrs.items():
            cmd = "cat {}".format(val["sysfs"])
            try:
                output = subprocess.check_output(cmd, shell=True, universal_newlines=True).strip()
                board[key] = int(output, 16)
            except subprocess.CalledProcessError as e:
                self.bsp_pr(
                    "Get {} from CPLD failed, status={}, output={}, cmd={}\n".format(
                        key, e.returncode, e.output, cmd
                    ),
                    self.LEVEL_ERR,
                )
                board[key] = 1  # Default value when command fails
            except (ValueError, FileNotFoundError) as e:
                self.bsp_pr("Error: {}. Setting default value for {}.\n".format(str(e), key), self.LEVEL_ERR)
                board[key] = 1  # Default value on error
    
        return board
    
    def get_gpio_max(self):
        cmd = "cat {}/bsp_gpio_max".format(self.PATH_LPC_GRP_BSP)
        try:
            output = subprocess.check_output(cmd, shell=True)
            gpio_max = int(output.strip())
        except subprocess.CalledProcessError as e:
            self.bsp_pr("Get gpio max failed, status={}, output={}, cmd={}\n".format(e.returncode, e.output, cmd), self.LEVEL_ERR)
            gpio_max = 511  # Default value when command fails
        except (ValueError, FileNotFoundError) as e:
            self.bsp_pr("Error: {}. Setting default value for GPIO max.\n".format(str(e)), self.LEVEL_ERR)
            gpio_max = 511  # Default value on error
    
        return gpio_max

    def init_i2c_mux_idle_state(self, muxs):
        IDLE_STATE_DISCONNECT = -2

        for mux in muxs:
            i2c_addr = mux[1]
            i2c_bus = mux[2]
            sysfs_idle_state = self.PATH_SYS_I2C_DEV_ATTR.format(i2c_bus, i2c_addr, "idle_state")
            if os.path.exists(sysfs_idle_state):
                with open(sysfs_idle_state, 'w') as f:
                    f.write(str(IDLE_STATE_DISCONNECT))

    def init_mux(self, bus_i801, bmc_enable):

        if bmc_enable: # s9180-adv
            i2c_muxs = [
                # driver, addr, bus_number
                ('pca9548', 0x70, 0),  #0
                ('pca9548', 0x71, 1),  #1
                ('pca9548', 0x71, 2),  #2
                ('pca9548', 0x71, 3),  #3
                ('pca9548', 0x71, 4),  #4
                ('pca9548', 0x71, 7),  #5
            ]
        else: # s9180-std
            i2c_muxs = [
                # driver, addr, bus_number
                ('pca9548', 0x70, 0),  #0
                ('pca9548', 0x71, 1),  #1
                ('pca9548', 0x71, 2),  #2
                ('pca9548', 0x71, 3),  #3
                ('pca9548', 0x71, 4),  #4
                ('pca9548', 0x71, 7),  #5
                ('pca9548', 0x76, 0),  #6 (Dummy BMC)
                ('pca9545', 0x72, 0),  #7 (Dummy BMC) pca9546
            ]

        self.new_i2c_devices(i2c_muxs)
        
        #init idle state on mux
        self.init_i2c_mux_idle_state(i2c_muxs)

    def init_sys_eeprom(self, bmc_enable):
        if bmc_enable:
            sys_eeprom = [
                ('sys_eeprom', 0x51, 0),
            ]
        else:
            sys_eeprom = [
                ('sys_eeprom', 0x55, 0),
                ('sys_eeprom', 0x51, 0),
            ]

        self.new_i2c_devices(sys_eeprom)

    def init_cpld(self):
        cpld = [
            ('s9180_32x_cpld1', 0x33, 44),
        ]

        self.new_i2c_devices(cpld)
        
    def gpio_settings(self, bmc_enable):
        # Golden Finger to show CPLD
        os.system("i2cget -y 44 0x74 2")
        
        if not bmc_enable:
            # Reset BMC Dummy Board
            os.system("i2cset -y -r 0 0x26 4 0x00")
            os.system("i2cset -y -r 0 0x26 5 0x00")
            os.system("i2cset -y -r 0 0x26 2 0x3F")
            os.system("i2cset -y -r 0 0x26 3 0x1F")
            os.system("i2cset -y -r 0 0x26 6 0xC0")
            os.system("i2cset -y -r 0 0x26 7 0x00")

        # CPU Baord
        os.system("i2cset -y -r 0 0x77 6 0xFF")
        os.system("i2cset -y -r 0 0x77 7 0xFF")

        # init SMBUS1 ABS
        os.system("i2cset -y -r 5 0x20 4 0x00")
        os.system("i2cset -y -r 5 0x20 5 0x00")
        os.system("i2cset -y -r 5 0x20 6 0xFF")
        os.system("i2cset -y -r 5 0x20 7 0xFF")

        # init SMBUS1 ABS
        os.system("i2cset -y -r 5 0x21 4 0x00")
        os.system("i2cset -y -r 5 0x21 5 0x00")
        os.system("i2cset -y -r 5 0x21 6 0xFF")
        os.system("i2cset -y -r 5 0x21 7 0xFF")

        # init SMBUS1 INT
        os.system("i2cset -y -r 5 0x22 4 0x00")
        os.system("i2cset -y -r 5 0x22 5 0x00")
        os.system("i2cset -y -r 5 0x22 6 0xFF")
        os.system("i2cset -y -r 5 0x22 7 0xFF")
        
        # init SMBUS1 INT
        os.system("i2cset -y -r 5 0x23 4 0x00")
        os.system("i2cset -y -r 5 0x23 5 0x00")
        os.system("i2cset -y -r 5 0x23 6 0xFF") 
        os.system("i2cset -y -r 5 0x23 7 0xFF") 
        
        # init SFP
        os.system("i2cset -y -r 5 0x27 4 0x00")
        os.system("i2cset -y -r 5 0x27 5 0x00")
        os.system("i2cset -y -r 5 0x27 2 0x00")
        os.system("i2cset -y -r 5 0x27 3 0x00")
        os.system("i2cset -y -r 5 0x27 6 0xCF")
        os.system("i2cset -y -r 5 0x27 7 0xF0")

        # set ZQSFP LP_MODE = 0
        os.system("i2cset -y -r 6 0x20 4 0x00")
        os.system("i2cset -y -r 6 0x20 5 0x00")
        os.system("i2cset -y -r 6 0x20 2 0x00")
        os.system("i2cset -y -r 6 0x20 3 0x00")
        os.system("i2cset -y -r 6 0x20 6 0x00")
        os.system("i2cset -y -r 6 0x20 7 0x00")

        os.system("i2cset -y -r 6 0x21 4 0x00")
        os.system("i2cset -y -r 6 0x21 5 0x00")
        os.system("i2cset -y -r 6 0x21 2 0x00")
        os.system("i2cset -y -r 6 0x21 3 0x00")
        os.system("i2cset -y -r 6 0x21 6 0x00")
        os.system("i2cset -y -r 6 0x21 7 0x00")

        # set ZQSFP RST = 1 
        os.system("i2cset -y -r 6 0x22 4 0x00")
        os.system("i2cset -y -r 6 0x22 5 0x00")
        os.system("i2cset -y -r 6 0x22 2 0xFF")
        os.system("i2cset -y -r 6 0x22 3 0xFF")
        os.system("i2cset -y -r 6 0x22 6 0x00")
        os.system("i2cset -y -r 6 0x22 7 0x00")

        os.system("i2cset -y -r 6 0x23 4 0x00")
        os.system("i2cset -y -r 6 0x23 5 0x00")
        os.system("i2cset -y -r 6 0x23 2 0xFF")
        os.system("i2cset -y -r 6 0x23 3 0xFF")
        os.system("i2cset -y -r 6 0x23 6 0x00")
        os.system("i2cset -y -r 6 0x23 7 0x00")

        # init Host GPIO
        os.system("i2cset -y -r 0 0x74 4 0x00")
        os.system("i2cset -y -r 0 0x74 5 0x00")
        os.system("i2cset -y -r 0 0x74 2 0x0F")
        os.system("i2cset -y -r 0 0x74 3 0xDF")
        os.system("i2cset -y -r 0 0x74 6 0x08")
        os.system("i2cset -y -r 0 0x74 7 0x1F")
        
        if not bmc_enable:
            # init Board ID
            os.system("i2cset -y -r 51 0x27 4 0x00")
            os.system("i2cset -y -r 51 0x27 5 0x00")
            os.system("i2cset -y -r 51 0x27 6 0xFF")
            os.system("i2cset -y -r 51 0x27 7 0xFF")

            # init Board ID of Dummy BMC Board
            os.system("i2cset -y -r 0 0x24 4 0x00")
            os.system("i2cset -y -r 0 0x24 5 0x00")
            os.system("i2cset -y -r 0 0x24 6 0xFF")
            os.system("i2cset -y -r 0 0x24 7 0xFF")

            # init PSU I/O (BAREFOOT_IO_EXP_PSU_ID)
            os.system("i2cset -y -r 0 0x25 4 0x00")
            os.system("i2cset -y -r 0 0x25 5 0x00")
            os.system("i2cset -y -r 0 0x25 2 0x00")
            os.system("i2cset -y -r 0 0x25 3 0x1D")
            os.system("i2cset -y -r 0 0x25 6 0xDB")
            os.system("i2cset -y -r 0 0x25 7 0x03")

            # init FAN I/O (BAREFOOT_IO_EXP_FAN_ID)
            os.system("i2cset -y -r 59 0x20 4 0x00")
            os.system("i2cset -y -r 59 0x20 5 0x00")
            os.system("i2cset -y -r 59 0x20 2 0x11")
            os.system("i2cset -y -r 59 0x20 3 0x11")
            os.system("i2cset -y -r 59 0x20 6 0xCC")
            os.system("i2cset -y -r 59 0x20 7 0xCC")

            # init Fan
            # select bank 0
            os.system("i2cset -y -r 56 0x2F 0x00 0x80")
            # enable FANIN 1-8
            os.system("i2cset -y -r 56 0x2F 0x06 0xFF")
            # disable FANIN 9-14
            os.system("i2cset -y -r 56 0x2F 0x07 0x00")
            # select bank 2
            os.system("i2cset -y -r 56 0x2F 0x00 0x82")
            # set PWM mode in FOMC
            os.system("i2cset -y -r 56 0x2F 0x0F 0x00")

            # init VOLMON
            os.system("i2cset -y -r 56 0x2F 0x00 0x80")
            os.system("i2cset -y -r 56 0x2F 0x01 0x1C")
            os.system("i2cset -y -r 56 0x2F 0x00 0x80")
            os.system("i2cset -y -r 56 0x2F 0x02 0xFF")
            os.system("i2cset -y -r 56 0x2F 0x03 0x50")
            os.system("i2cset -y -r 56 0x2F 0x04 0x0A")
            os.system("i2cset -y -r 56 0x2F 0x00 0x80")
            os.system("i2cset -y -r 56 0x2F 0x01 0x1D")
            self.new_i2c_device('w83795adg', 0x2F, 56)

            # init Fan Speed
            os.system("echo 120 > /sys/class/hwmon/hwmon1/device/pwm1")
            os.system("echo 120 > /sys/class/hwmon/hwmon1/device/pwm2")
        
    def init_temp(self, bmc_enable):
        if bmc_enable:
            self.new_i2c_devices(
                [
                    # CPU Board
                    ('tmp75', 0x4F, 0),
                ]
            )
        else:
            self.new_i2c_devices(
                [
                    # ASIC Coretemp and Front MAC
                    ('lm86', 0x4C, 53),
                
                    # CPU Board
                    ('tmp75', 0x4F, 0),
                
                    # Near PSU1
                    ('tmp75', 0x48, 53),
                
                    # Rear MAC
                    ('tmp75', 0x4A, 53),
                
                    # Near Port 32
                    ('tmp75', 0x4B, 53),
                
                    # Near PSU2
                    ('tmp75', 0x4D, 53),
                ]
            )

    def init_eeprom(self):
        data = None
        port_eeprom = {
            1:  {"type": "QSFP"  , "bus": 10, "driver": "optoe1"},
            2:  {"type": "QSFP"  , "bus": 9 , "driver": "optoe1"},
            3:  {"type": "QSFP"  , "bus": 12, "driver": "optoe1"},
            4:  {"type": "QSFP"  , "bus": 11, "driver": "optoe1"},
            5:  {"type": "QSFP"  , "bus": 14, "driver": "optoe1"},
            6:  {"type": "QSFP"  , "bus": 13, "driver": "optoe1"},
            7:  {"type": "QSFP"  , "bus": 16, "driver": "optoe1"},
            8:  {"type": "QSFP"  , "bus": 15, "driver": "optoe1"},
            9:  {"type": "QSFP"  , "bus": 18, "driver": "optoe1"},
            10: {"type": "QSFP"  , "bus": 17, "driver": "optoe1"},
            11: {"type": "QSFP"  , "bus": 20, "driver": "optoe1"},
            12: {"type": "QSFP"  , "bus": 19, "driver": "optoe1"},
            13: {"type": "QSFP"  , "bus": 22, "driver": "optoe1"},
            14: {"type": "QSFP"  , "bus": 21, "driver": "optoe1"},
            15: {"type": "QSFP"  , "bus": 24, "driver": "optoe1"},
            16: {"type": "QSFP"  , "bus": 23, "driver": "optoe1"},
            17: {"type": "QSFP"  , "bus": 26, "driver": "optoe1"},
            18: {"type": "QSFP"  , "bus": 25, "driver": "optoe1"},
            19: {"type": "QSFP"  , "bus": 28, "driver": "optoe1"},
            20: {"type": "QSFP"  , "bus": 27, "driver": "optoe1"},
            21: {"type": "QSFP"  , "bus": 30, "driver": "optoe1"},
            22: {"type": "QSFP"  , "bus": 29, "driver": "optoe1"},
            23: {"type": "QSFP"  , "bus": 32, "driver": "optoe1"},
            24: {"type": "QSFP"  , "bus": 31, "driver": "optoe1"},
            25: {"type": "QSFP"  , "bus": 34, "driver": "optoe1"},
            26: {"type": "QSFP"  , "bus": 33, "driver": "optoe1"},
            27: {"type": "QSFP"  , "bus": 36, "driver": "optoe1"},
            28: {"type": "QSFP"  , "bus": 35, "driver": "optoe1"},
            29: {"type": "QSFP"  , "bus": 38, "driver": "optoe1"},
            30: {"type": "QSFP"  , "bus": 37, "driver": "optoe1"},
            31: {"type": "QSFP"  , "bus": 40, "driver": "optoe1"},
            32: {"type": "QSFP"  , "bus": 39, "driver": "optoe1"},
            33: {"type": "SFP"   , "bus": 45, "driver": "optoe1"}, # optoe2
            34: {"type": "SFP"   , "bus": 46, "driver": "optoe1"}, # optoe2
        }

        with open(self.PATH_PORT_CONFIG, 'r') as yaml_file:
            data = yaml.safe_load(yaml_file)

        # config eeprom
        for port, config in port_eeprom.items():
            addr=0x50
            self.new_i2c_device(config["driver"], addr, config["bus"])
            port_name = data[config["type"]][port]["port_name"]
            sysfs=self.PATH_SYS_I2C_DEV_ATTR.format( config["bus"], addr, "port_name")
            subprocess.call("echo {} > {}".format(port_name, sysfs), shell=True)

    def init_gpio(self, bmc_enable):
        # pca9555 gpio register setting
        self.bsp_pr("START GPIO SETTINGS...")
        self.gpio_settings(bmc_enable)
        
        # init GPIO sysfs
        if bmc_enable:
            self.new_i2c_devices(
                [
                    ('pca9555', 0x20, 5), # ABS Port 0-15
                    ('pca9555', 0x21, 5), # ABS Port 16-31
                    ('pca9555', 0x22, 5), # INT Port 0-15
                    ('pca9555', 0x23, 5), # INT Port 16-31
                    ('pca9555', 0x27, 5), # SFP status
                    ('pca9555', 0x20, 6), # LP Mode Port 0-15
                    ('pca9555', 0x21, 6), # LP Mode Port 16-31
                    ('pca9555', 0x22, 6), # RST Port 0-15
                    ('pca9555', 0x23, 6), # RST Port 16-31
                ]
            )
        else:
            self.new_i2c_devices(
                [
                    ('pca9555', 0x20, 5), # ABS Port 0-15
                    ('pca9555', 0x21, 5), # ABS Port 16-31
                    ('pca9555', 0x22, 5), # INT Port 0-15
                    ('pca9555', 0x23, 5), # INT Port 16-31
                    ('pca9555', 0x27, 5), # SFP status
                    ('pca9555', 0x20, 6), # LP Mode Port 0-15
                    ('pca9555', 0x21, 6), # LP Mode Port 16-31
                    ('pca9555', 0x22, 6), # RST Port 0-15
                    ('pca9555', 0x23, 6), # RST Port 16-31
                    ('pca9555', 0x25, 0), # PSU I/O status
                ]
            )

        # get gpio base (max) = 511
        gpio_max = self.get_gpio_max()
        msg("GPIO_MAX = %d\n" % gpio_max)
        
        # gpio total number = 512
        gpio_tnumber = gpio_max + 1 
        
        # Get IO Expander bit length (16 bit)
        gpio_bit = int(subprocess.check_output("find /sys/class/gpio/gpiochip*/ngpio -maxdepth 1 -type f -exec cat {} + | tail -1", shell=True))
        
        # Init GPIO Base Index
        gpio_end_index = gpio_tnumber
        gpio_start_index = gpio_end_index - gpio_bit
        
        # -------------------------------------------------------------------------------------
        # export gpio
        # QSFP Status - ABS Port 0-15
        for i in range(gpio_start_index, gpio_end_index):  #range(496, 512)/ range(1008,1024)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # QSFP Status - ABS Port 16-31
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(480, 496)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # QSFP Status - INT Port 0-15
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(464, 480)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # QSFP Status - INT Port 16-31
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(448, 464)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # SFP status
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(432, 448)
            os.system("echo {} > {}/export".format(i, self.PATH_SYS_GPIO))
            # SFP PRSNT (432, 433) 
            # TX DISABLE/ RS/ TS (432+4/433+4/432+8/432+9/432+10/432+11)
            if i == (gpio_start_index+4) or i == (gpio_start_index+5) or i == (gpio_start_index+8) or \
                i == (gpio_start_index+9) or i == (gpio_start_index+10) or i == (gpio_start_index+11):
                os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            else:
                os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # QSFP Status - LP Mode Port 0-15
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(416, 432)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)

        # QSFP Status - LP Mode Port 16-31
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(400, 416)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)

        # QSFP Status - RST Port 0-15
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(384, 400)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/value" % i) 

        # QSFP Status - RST Port 16-31
        # Modify GPIO Base Index
        gpio_end_index = gpio_start_index
        gpio_start_index = gpio_end_index - gpio_bit
        for i in range(gpio_start_index, gpio_end_index):  #range(368, 384)
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/value" % i)

        if not bmc_enable:
            # Modify GPIO Base Index
            gpio_end_index = gpio_start_index
            gpio_start_index = gpio_end_index - gpio_bit
            for i in range(gpio_start_index, gpio_end_index):  #range(352, 368)
                os.system("echo {} > /sys/class/gpio/export".format(i))
                # PW1 PRSNT EVENT(356)/ PW2 PRSNT EVENT(353)/ 
                # PW1 GOOD STATUS(355)/ PW2 GOOD STATUS(352)/
                # (353, 354, 356, 357, 358, 361, 362, 364)
                if i in [gpio_start_index+1, gpio_start_index+2, gpio_start_index+4, gpio_start_index+5, \
                        gpio_start_index+6, gpio_start_index+9, gpio_start_index+10, gpio_start_index+12]:
                    os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)
        # -------------------------------------------------------------------------------------

    def enable_ipmi_maintenance_mode(self):
        ipmi_ioctl = IPMI_Ioctl()

        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("Current IPMI_MAINTENANCE_MODE=%d\n" % (mode) )

        ipmi_ioctl.set_ipmi_maintenance_mode(IPMI_Ioctl.IPMI_MAINTENANCE_MODE_ON)

        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("After IPMI_IOCTL IPMI_MAINTENANCE_MODE=%d\n" % (mode) )

    def baseconfig(self):
        
        self.bsp_pr("START MODPROBE DRIVERS...")
        os.system("modprobe i2c_i801")
        os.system("modprobe i2c_dev")
        
        os.system("modprobe i2c_mux_pca954x")
        os.system("modprobe coretemp")
        
        os.system("modprobe w83795")
        os.system("modprobe gpio_pca953x")
        
        os.system("modprobe ipmi_devintf")
        os.system("modprobe ipmi_si")
        self.bsp_pr("MODPROBE DRIVERS COMPLETE!")

        bus_i801 = 0
        
        # lpc driver
        self.insmod("x86-64-ufispace-s9180-32x-lpc")
        self.bsp_pr("INSMOD LPC DRIVER COMPLETE!")
        
        # check s9180-32x platform std or adv
        bmc_enable = self.check_bmc_enable()
        msg("bmc enable : %r\n" % (True if bmc_enable else False))
        # record the result for onlp
        os.system("echo %d > /etc/onl/bmc_en" % bmc_enable)
        self.bsp_pr("CHECK BMC DONE! BMC ENABLE = {}".format(bmc_enable))

        # version setting
        self.bsp_pr("BSP VERSION = {}".format(self.BSP_VERSION))
        self.config_bsp_ver(self.BSP_VERSION)

        # get gpio max
        gpio_max = self.get_gpio_max()
        self.bsp_pr("GPIO MAX = {}".format(gpio_max))
        
        # vid to mac vdd value mapping
        vdd_val_array=( 0.85,  0.82,  0.77,  0.87,  0.74,  0.84,  0.79,  0.89 )
        # vid to rov reg value mapping
        rov_reg_array=( 0x24,  0x21,  0x1C,  0x26,  0x19, 0x23, 0x1E, 0x28 )
        
        # init SYS EEPROM devices
        self.bsp_pr("START INIT SYS EEPROM...")
        self.insmod("x86-64-ufispace-sys-eeprom")
        self.init_sys_eeprom(bmc_enable)
        os.system("modprobe eeprom")
        self.bsp_pr("INIT SYS EEPROM DONE!")

        # init MUX sysfs & init idle state on mux
        self.bsp_pr("START INIT I2C MUX...")
        self.init_mux(bus_i801, bmc_enable)
        self.bsp_pr("INIT I2C MUX DONE!")
        
        # init CPLD
        self.bsp_pr("START INIT CPLD...")
        self.insmod("x86-64-ufispace-s9180-32x-cpld")
        self.init_cpld()
        self.bsp_pr("INIT CPLD DONE!")
        
        # init GPIO sysfs and configure settings
        self.bsp_pr("START INIT GPIO...")
        self.init_gpio(bmc_enable)
        self.bsp_pr("INIT GPIO DONE!")
        
        # init temperature
        self.bsp_pr("START INIT TEMPERATURE...")
        self.init_temp(bmc_enable)
        self.bsp_pr("INIT TEMPERATURE DONE!")

        # init Port EEPROM
        self.bsp_pr("START INIT PORT EEPROM...")
        self.bsp_pr("INSMOD OPTOE DRIVER")
        self.insmod("optoe")
        self.bsp_pr("INIT PORT EEPROM")
        self.init_eeprom()
        self.bsp_pr("INIT PORT EEPROM DONE!")
        
        # init PSU(0/1) EEPROM devices
        if not bmc_enable:
            self.bsp_pr("INIT PSU EEPROM (S9180-32X-STD only)")
            self.new_i2c_device('eeprom', 0x50, 57)
            self.new_i2c_device('eeprom', 0x50, 58)
        
        # _mac_vdd_init
        self.bsp_pr("MAC VDD INIT")
        try:
            reg_val_str = subprocess.check_output("""i2cget -y 44 0x33 0x42 2>/dev/null""", shell=True)
            reg_val = int(reg_val_str, 16)
            vid = reg_val & 0x7
            mac_vdd_val = vdd_val_array[vid]
            rov_reg = rov_reg_array[vid]
            subprocess.check_output("""i2cset -y -r 55 0x22 0x21 0x%x w 2>/dev/null""" % rov_reg, shell=True)
            msg("Setting mac vdd %1.2f with rov register value 0x%x\n" % (mac_vdd_val, rov_reg) )
        except subprocess.CalledProcessError:
            pass
        
        if not bmc_enable:
            # init SYS LED
            self.bsp_pr("INIT SYS LED")
            os.system("i2cset -y -r 50 0x75 2 0x01")
            os.system("i2cset -y -r 50 0x75 4 0x00")
            os.system("i2cset -y -r 50 0x75 5 0x00")
            os.system("i2cset -y -r 50 0x75 6 0x00")
            os.system("i2cset -y -r 50 0x75 7 0x00")
            self.bsp_pr("INIT SYS LED DONE!")
            
        # get board version
        board = self.get_board_version()

        # enable ipmi maintenance mode
        if bmc_enable:
            self.bsp_pr("ENABLE IPMI MAINTENANCE MODE!")
            self.enable_ipmi_maintenance_mode()
            
            self.bsp_pr("S9180-32X-ADV INIT DONE!")
        
        else:
            self.bsp_pr("S9180-32X-STD INIT DONE!")
        
        return True

