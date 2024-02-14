# Block comands issue

If i2c-detect tells that SMBus Block Read not supported (Linux kernel issue) specify dto overlay for IC2 in the device boot config file

```bash
ssh USER@XXX.XXX.XXX.XXX
sudo nano /boot/config.txt
```

Insert following line

```bash
dtoverlay=i2c-gpio,bus=BUS_NUMBER,i2c_gpio_delay_us=GPIO_DELAY_US,i2c_gpio_sda=SDA_PIN,i2c_gpio_scl=SCL_PIN
```

For example (replaces default i2c-0 device on RPI 1 Model B rev1)
```bash
dtoverlay=i2c-gpio,bus=0,i2c_gpio_delay_us=2,i2c_gpio_sda=23,i2c_gpio_scl=24
```

* `dtoverlay=i2c-gpio` - Specifies I2C GPIO device tree overlay (DON'T CHANGE)(REQUIRED).
* `bus=0` - Specifies bus number, /dev/i2c-N device will appear after reboot. Official guide says that it is not recommended to set 0 bus number but actually it works fine, overlay will override default functionality of the existing bus if you specify correct pin numbers. If you just want to make predefined I2C bus working as it should, you don't need to change anything except the boot config. (OPTIONAL, available on kernel version 4.14 and higher)
*  `i2c_gpio_sda` - Data line pin number. (REQUIRED)
*  `i2c_gpio_scl` - Clock line pin number. (REQUIRED)
*  `i2c_gpio_delay_us` GPIO delay in microseconds. 2 (default) means approximately 100kHZ. The resulting frequency is about `1000000 / (4 * delay_us)`. General speed also can be specified by setting `dtparam=i2c_arm_baudrate=BAUDRATE_IN_HZ` as a new config line. (OPTIONAL)

After any boot config modification reboot your device.

# Environment setup

## Step 1 - Mount sshfs

```bash
sshfs USER@XXX.XXX.XXX.XXX:/TARGET_LOCATION /HOST_LOCATION -o allow_other,default_permissions
```

## Step 2 - Connect by ssh

```bash
ssh USER@XXX.XXX.XXX.XXX
```

## Step 3 - Set target related vars

```cmake
#set dir which will follow to the root of the running target device
set(RASPBIAN_TARGET_ROOT SSHFS_MOUNT_LOCATION)

#set sysroot dir (not the sysroot from your toolchain)
set(CMAKE_SYSROOT SYSROOT_LOCATION)

#set installation prefix, binaries will be moved to the target device
set(RASPBIAN_INSTALL_PREFIX SSHFS_MOUNT_LOCATION/LOCATION_ON_TARGET_DEVICE")
```

## Step 4 - Modify settings.json
Set valid path to the toolchain file

## Step 5 - Modify launch.json
Set valid path gdb path, binary path and target IP:PORT

## Step 6 - Configure and set "install" as CMake current target
Binaries will be moved to the target device right after the successfull build
If copy operation failes it means that the sshfs is unmounted (repeat step 1)

## Step 7 Run build
Run build task in VSCode

## Step 8 - Run gdb server on rpi
```bash
gdbserver --multi :PORT /LOCATION_ON_TARGET_DEVICE/BINARY_DIR/BIN_NAME
```
## Step 9 - Run debug task in VSCode
Set some breakpoints and start debugging

## Example

```cmake
#optional
set(RASPBIAN_DIR "$ENV{HOME}/raspbian")

#required
set(RASPBIAN_TARGET_ROOT "${RASPBIAN_DIR}/sshfs")
set(CMAKE_SYSROOT "${RASPBIAN_DIR}/cross-compile/chroots/rpi-bullseye-armhf")
set(RASPBIAN_INSTALL_PREFIX "${RASPBIAN_TARGET_ROOT}/home/pi")

#install call
install(
    TARGETS ${PROJECT_CMD}
    RUNTIME
    DESTINATION "${RASPBIAN_INSTALL_PREFIX}/${PROJECT_NAME}/"
)
```

```bash
# HOST SHELL
sudo sshfs pi@192.168.0.107:/ /home/maksym_trehubenko/raspbian/sshfs/ -o allow_other,default_permissions

sshfs pi@192.168.0.107

# TARGET SHELL
gdbserver --multi :2345 ~/raspbian-hello/raspbian-hello
```
