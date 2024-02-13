#include <smbus.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define SMBUS_I2C_DEVICE_FORMAT "/dev/i2c-%u"
#define SMBUS_I2C_DEVICE_NAME_LEN 20

static int smbus_rw_access(
    uint8_t i2c_bus,
    unsigned command_type, 
    uint8_t read_write, 
    uint8_t reg,
    union i2c_smbus_data* data
);

int smbus_open(
    unsigned i2c_bus_number
)
{
    char device_path[SMBUS_I2C_DEVICE_NAME_LEN + 1];
    snprintf(device_path, SMBUS_I2C_DEVICE_NAME_LEN, SMBUS_I2C_DEVICE_FORMAT, i2c_bus_number);

    int i2c_bus = open(device_path, O_RDWR);

    return i2c_bus;
}

bool smbus_close(
    int i2c_bus
)
{
    if(close(i2c_bus) < 0)
    {
        return false;
    }

    return true;
}

bool smbus_use_slave(
    int i2c_bus,
    uint8_t address
)
{
    if(ioctl(i2c_bus, I2C_SLAVE, address) < 0)
    {
        return false;
    }

    return true;
}

int smbus_rw_access(
    uint8_t i2c_bus,
    unsigned command_type, 
    uint8_t read_write, 
    uint8_t reg,
    union i2c_smbus_data* data
)
{
    int res = 0;
    struct i2c_smbus_ioctl_data args = {
        .command = reg,
        .read_write = read_write,
        .size = command_type,
        .data = data,
    };

    res = ioctl(i2c_bus, I2C_SMBUS, &args);

    return res;
}

bool smbus_quick_command(
    int i2c_bus,
    bool bit
)
{
    int res = 0;

    uint8_t read_write = bit ? I2C_SMBUS_READ : I2C_SMBUS_WRITE;

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_QUICK, read_write, 0x00, NULL)) < 0)
    {
        return false;
    }

    return true;
}

bool smbus_read_reg(
    int i2c_bus,
    uint8_t* reg
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BYTE, I2C_SMBUS_READ, 0x00, &data)) < 0)
    {
        return false;
    }

    *reg = data.byte;

    return true;
}

bool smbus_write_reg(
    int i2c_bus,
    uint8_t reg
)
{
    int res = 0;

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BYTE, I2C_SMBUS_WRITE, reg, NULL)) < 0)
    {
        return false;
    }

    return true;
}


bool smbus_read_byte_data(
    int i2c_bus,
    uint8_t command,
    uint8_t* byte
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BYTE_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    *byte = data.byte;

    return true;
}

bool smbus_write_byte_data(
    int i2c_bus,
    uint8_t command,
    uint8_t byte
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.byte = byte;

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BYTE_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_word_data(
    int i2c_bus,
    uint8_t command,
    uint16_t* word
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_WORD_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    *word = data.word;
    
    return true;
}

bool smbus_write_word_data(
    int i2c_bus,
    uint8_t command,
    uint16_t word
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.word = word;

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_WORD_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}


bool smbus_read_dword_data(
    int i2c_bus,
    uint8_t command,
    uint32_t* dword
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint32_t);

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    memcpy(dword, &data.block[1], data.block[0]);
    
    return true;
}

bool smbus_write_dword_data(
    int i2c_bus,
    uint8_t command,
    uint32_t dword
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint32_t);
    memcpy(&data.block[1], &dword, data.block[0]);

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_qword_data(
    int i2c_bus,
    uint8_t command,
    uint64_t* qword
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint64_t);

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    memcpy(qword, &data.block[1], data.block[0]);
    
    return true;
}

bool smbus_write_qword_data(
    int i2c_bus,
    uint8_t command,
    uint64_t qword
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint64_t);
    memcpy(&data.block[1], &qword, data.block[0]);

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_block_data(
    int i2c_bus,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    if(data.block[0] > SMBUS_BLOCK_MAX)
    {
        data.block[0] = SMBUS_BLOCK_MAX;
    }

    *length = data.block[0];
    memcpy(block, &data.block[1], data.block[0]);
    
    return true;
}

bool smbus_write_block_data(
    int i2c_bus,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if(*length > SMBUS_BLOCK_MAX)
    {
        *length = SMBUS_BLOCK_MAX;
    }

    data.block[0] = *length;
    memcpy(&data.block[1], block, data.block[0]);

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_proc_call(
    int i2c_bus,
    uint8_t command,
    uint16_t request,
    uint16_t* response
)
{
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.word = request;

    if((res = smbus_rw_access(i2c_bus, I2C_SMBUS_PROC_CALL, 0, command, &data)) < 0)
    {
        return false;
    }

    *response = data.word;
    
    return true;
}