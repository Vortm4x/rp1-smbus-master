# Step 1 - Mount sshfs

```bash
sshfs USER@XXX.XXX.XXX.XXX:/TARGET_LOCATION /HOST_LOCATION -o allow_other,default_permissions
```

# Step 2 - Connect by ssh

```bash
ssh USER@XXX.XXX.XXX.XXX
```

# Step 3 - Set target related vars

```cmake
#set dir which will follow to the root of the running target device
set(RASPBIAN_TARGET_ROOT SSHFS_MOUNT_LOCATION)

#set sysroot dir (not the sysroot from your toolchain)
set(CMAKE_SYSROOT SYSROOT_LOCATION)

#set installation prefix, binaries will be moved to the target device
set(RASPBIAN_INSTALL_PREFIX SSHFS_MOUNT_LOCATION/LOCATION_ON_TARGET_DEVICE")
```

# Step 4 - Modify settings.json
Set valid path to the toolchain file

# Step 5 - Modify launch.json
Set valid path gdb path, binary path and target IP:PORT

# Step 6 - Configure and set "install" as CMake current target
Binaries will be moved to the target device right after the successfull build
If copy operation failes it means that the sshfs is unmounted (repeat step 1)

# Step 7 Run build
Run build task in VSCode

# Step 8 - Run gdb server on rpi
```bash
gdbserver --multi :PORT /LOCATION_ON_TARGET_DEVICE/BINARY_DIR/BIN_NAME
```
# Step 9 - Run debug task in VSCode
Set some breakpoints and start debugging

# Example

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
