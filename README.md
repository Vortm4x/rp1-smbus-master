# Step 1 - Build the app

# Step 2 - Mount sshfs

```bash
sshfs USER@XXX.XXX.XXX.XXX:/TARGET_LOCATION /HOST_LOCATION -o allow_other,default_permissions
```

# Step 3 - Setup CMake install command
```cmake
set(RASPBIAN_INSTALL_PREFIX "/HOST_LOCATION")
```

# Step 5 - Run CMake build target

# Step 6 - Run CMake install target

# Step 7 - run gdb server on rpi

```bash
gdbserver --multi :PORT /TARGET_LOCATION/PROJECT_NAME/BIN_NAME
```

# Example

```cmake
set(RASPBIAN_INSTALL_PREFIX "~/raspbian-root")
```

```bash
# HOST
sudo sshfs pi@192.168.0.106:/home/pi /home/maksym_trehubenko/raspbian/sshfs/ -o allow_other,default_permissions

# TARGET
gdbserver --multi :2345 ~/raspbian-hello/raspbian-hello
```
