#ifndef SMBUS_H
#define SMBUS_H

#include <stdint.h>
#include <stdbool.h>

#define SMBUS_BLOCK_MAX 32


typedef void* smbus_handle_t;


smbus_handle_t smbus_open(
    unsigned i2c_bus_number
);
bool smbus_close(
    smbus_handle_t smbus_handle
);
bool smbus_use_slave(
    smbus_handle_t smbus_handle,
    uint8_t address
);
bool smbus_set_pec(
    smbus_handle_t smbus_handle,
    bool is_enabled
);
bool smbus_get_pec(
    smbus_handle_t smbus_handle
);

bool smbus_quick_command(
    smbus_handle_t smbus_handle,
    bool bit
);
bool smbus_read_reg(
    smbus_handle_t smbus_handle,
    uint8_t* reg
);
bool smbus_write_reg(
    smbus_handle_t smbus_handle,
    uint8_t reg
);
bool smbus_read_byte_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* byte
);
bool smbus_write_byte_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t byte
);
bool smbus_read_word_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t* word
);
bool smbus_write_word_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t word
);
bool smbus_read_dword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint32_t* dword
);
bool smbus_write_dword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint32_t dword
);
bool smbus_read_qword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint64_t* qword
);
bool smbus_write_qword_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint64_t qword
);
bool smbus_read_block_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
);
bool smbus_write_block_data(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint8_t* block,
    uint8_t* length
);
bool smbus_proc_call(
    smbus_handle_t smbus_handle,
    uint8_t command,
    uint16_t request,
    uint16_t* response
);

#endif // SMBUS_H