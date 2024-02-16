#include <smbus/smbus.h>
#include <smbus_pec.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define SMBUS_I2C_DEVICE_FORMAT "/dev/i2c-%u"
#define SMBUS_I2C_DEVICE_NAME_LEN 20

#define SMBUS_HANDLE_CHECK(smbus_handle)    \
    {                                       \
        if(smbus_handle == NULL)            \
        {                                   \
            errno = EINVAL;                 \
            return false;                   \
        }                                   \
    }                                       \
    while(0)

typedef struct smbus_inst_t
{
    int i2c_bus;
    uint8_t is_pec_enabled : 1;
    uint8_t slave_address : 7;
}
smbus_inst_t;

static int smbus_rw_access(
    uint8_t i2c_bus,
    unsigned command_type, 
    uint8_t read_write, 
    uint8_t reg,
    union i2c_smbus_data* data
);

static uint8_t smbus_calc_i2c_read_block_pec(
    uint8_t slave_address,
    uint8_t command,
    uint8_t* block 
);

static uint8_t smbus_calc_i2c_write_block_pec(
    uint8_t slave_address,
    uint8_t command,
    uint8_t* block 
);

smbus_handle_t smbus_open(
    unsigned bus_index
)
{
    char device_path[SMBUS_I2C_DEVICE_NAME_LEN + 1];
    snprintf(device_path, SMBUS_I2C_DEVICE_NAME_LEN, SMBUS_I2C_DEVICE_FORMAT, bus_index);

    int i2c_bus = open(device_path, O_RDWR);

    smbus_inst_t* smbus_inst = NULL;

    if(i2c_bus >= 0)
    {
        smbus_inst = calloc(1, sizeof(smbus_inst_t));
        smbus_inst->i2c_bus = i2c_bus;
    }

    return smbus_inst;
}

bool smbus_close(
    smbus_handle_t smbus_handle
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;

    int res = close(smbus_inst->i2c_bus);
    free(smbus_inst);

    return (res >= 0);
}

bool smbus_use_slave(
    smbus_handle_t smbus_handle,
    uint8_t address
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;

    if(ioctl(smbus_inst->i2c_bus, I2C_SLAVE, address) < 0)
    {
        return false;
    }

    smbus_inst->slave_address = address;

    return true;
}

bool smbus_set_pec(
    smbus_handle_t smbus_handle,
    bool is_enabled
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;

    if(is_enabled)
    {
        unsigned long func_flags = 0;

        ioctl(smbus_inst->i2c_bus, I2C_FUNCS, &func_flags);

        if((func_flags & I2C_FUNC_SMBUS_PEC) == 0)
        {
            errno = ENOTSUP;
            return false;
        }
    }

    ioctl(smbus_inst->i2c_bus, I2C_PEC, is_enabled);
    smbus_inst->is_pec_enabled = is_enabled;

    return true;
}

bool smbus_get_pec(
    smbus_handle_t smbus_handle
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;

    return smbus_inst->is_pec_enabled;
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

uint8_t smbus_calc_i2c_read_block_pec(
    uint8_t slave_address,
    uint8_t command,
    uint8_t* block 
)
{
    uint8_t write_address = (slave_address << 1) | I2C_SMBUS_WRITE;
    uint8_t read_address = (slave_address << 1) | I2C_SMBUS_READ;
    uint8_t crc = 0;

    crc = smbus_pec_single(crc, write_address);
    crc = smbus_pec_single(crc, command);
    crc = smbus_pec_single(crc, read_address);
    crc = smbus_pec_block(crc, &block[1], block[0]);

    return crc;
}

uint8_t smbus_calc_i2c_write_block_pec(
    uint8_t slave_address,
    uint8_t command,
    uint8_t* block 
)
{
    uint8_t write_address = (slave_address << 1) | I2C_SMBUS_WRITE;
    uint8_t crc = 0;

    crc = smbus_pec_single(crc, write_address);
    crc = smbus_pec_single(crc, command);
    crc = smbus_pec_block(crc, &block[1], block[0]);

    return crc;
}

bool smbus_quick_command(
    smbus_handle_t smbus_handle,
    bool bit
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    uint8_t read_write = bit ? I2C_SMBUS_READ : I2C_SMBUS_WRITE;

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_QUICK, read_write, 0x00, NULL)) < 0)
    {
        return false;
    }

    return true;
}

bool smbus_read_reg(
    smbus_handle_t smbus_handle,
    uint8_t* reg
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BYTE, I2C_SMBUS_READ, 0x00, &data)) < 0)
    {
        return false;
    }

    *reg = data.byte;

    return true;
}

bool smbus_write_reg(
    smbus_handle_t smbus_handle,
    uint8_t reg
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BYTE, I2C_SMBUS_WRITE, reg, NULL)) < 0)
    {
        return false;
    }

    return true;
}


bool smbus_read_byte_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* byte
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BYTE_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    *byte = data.byte;

    return true;
}

bool smbus_write_byte_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t byte
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.byte = byte;

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BYTE_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_word_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t* word
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_WORD_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    *word = data.word;
    
    return true;
}

bool smbus_write_word_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t word
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.word = word;

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_WORD_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}


bool smbus_read_dword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint32_t* dword
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint32_t);

    if(smbus_inst->is_pec_enabled)
    {
        ++data.block[0];
    }

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    if(smbus_inst->is_pec_enabled)
    {
        uint8_t received_crc = data.block[data.block[0]];
        --data.block[0];
        
        uint8_t calculated_crc = smbus_calc_i2c_read_block_pec(
            smbus_inst->slave_address, 
            command, 
            data.block
        );

        if(calculated_crc != received_crc)
        {
            errno = EBADMSG;
            return false;
        }
    }    

    memcpy(dword, &data.block[1], data.block[0]);
    
    return true;
}

bool smbus_write_dword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint32_t dword
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint32_t);
    memcpy(&data.block[1], &dword, data.block[0]);

    if(smbus_inst->is_pec_enabled)
    {
        uint8_t crc = smbus_calc_i2c_write_block_pec(
            smbus_inst->slave_address, 
            command, 
            data.block
        );

        ++data.block[0];
        data.block[data.block[0]] = crc;
    }

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_qword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint64_t* qword
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint64_t);

    if(smbus_inst->is_pec_enabled)
    {
        ++data.block[0];
    }

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
    {
        return false;
    }

    if(smbus_inst->is_pec_enabled)
    {
        uint8_t received_crc = data.block[data.block[0]];
        --data.block[0];
        
        uint8_t calculated_crc = smbus_calc_i2c_read_block_pec(
            smbus_inst->slave_address, 
            command, 
            data.block
        );

        if(calculated_crc != received_crc)
        {
            errno = EBADMSG;
            return false;
        }
    }

    memcpy(qword, &data.block[1], data.block[0]);
    
    return true;
}

bool smbus_write_qword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint64_t qword
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.block[0] = sizeof(uint64_t);
    memcpy(&data.block[1], &qword, data.block[0]);

    if(smbus_inst->is_pec_enabled)
    {
        uint8_t crc = smbus_calc_i2c_write_block_pec(
            smbus_inst->slave_address, 
            command, 
            data.block
        );

        ++data.block[0];
        data.block[data.block[0]] = crc;
    }

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_read_block_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BLOCK_DATA, I2C_SMBUS_READ, command, &data)) < 0)
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
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    if(*length > SMBUS_BLOCK_MAX)
    {
        *length = SMBUS_BLOCK_MAX;
    }

    data.block[0] = *length;
    memcpy(&data.block[1], block, data.block[0]);

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_BLOCK_DATA, I2C_SMBUS_WRITE, command, &data)) < 0)
    {
        return false;
    }
    
    return true;
}

bool smbus_proc_call(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t request,
    uint16_t* response
)
{
    SMBUS_HANDLE_CHECK(smbus_handle);
    smbus_inst_t* smbus_inst = (smbus_inst_t*)smbus_handle;
    int res = 0;

    union i2c_smbus_data data;
    memset(&data, 0, sizeof(union i2c_smbus_data));

    data.word = request;

    if((res = smbus_rw_access(smbus_inst->i2c_bus, I2C_SMBUS_PROC_CALL, 0, command, &data)) < 0)
    {
        return false;
    }

    *response = data.word;
    
    return true;
}