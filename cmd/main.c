#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <smbus.h>
#include "commands.h"

#define PICO_I2C_BUS_NUMBER 0
#define PICO_I2C_SLAVE_ADDRESS 0x17


int main(int argc, char* argv[]) 
{
    int i2c_bus = smbus_open(PICO_I2C_BUS_NUMBER);

    if(i2c_bus < 0)
    {
        perror("Error opening I2C bus");
        return -1;
    }

    printf("Opened bus i2c-%u\n", PICO_I2C_BUS_NUMBER);
    

    if(!smbus_use_slave(i2c_bus, PICO_I2C_SLAVE_ADDRESS))
    {
        perror("Error setting slave address");
        return -1;
    }

    printf("Set slave address: 0x%02X\n", PICO_I2C_SLAVE_ADDRESS);


    // QUICK CMD
    {
        if(smbus_quick_command(i2c_bus, true))
        {
            printf("[OK] QUICK ON\n");
        }
        else
        {
            perror("[ER] QUICK ON");
        }
        

        if(smbus_quick_command(i2c_bus, false))
        {
            printf("[OK] QUICK OFF\n");
        }
        else
        {
            perror("[ER] QUICK OFF");
        }        
    }

    // R/W REG 
    {
        uint8_t reg = 0;

        if(smbus_read_reg(i2c_bus, &reg))
        {
            printf("[OK] READ REG: [%02X]\n", reg);
        }
        else
        {
            perror("[ER] READ REG");
        }
        

        if(smbus_write_reg(i2c_bus, SMBUS_CMD_REG))
        {
            printf("[OK] WRITE REG: [%02X]\n", SMBUS_CMD_REG);
        }
        else
        {
            perror("[ER] WRITE REG");
        }
    }

    // R/W BYTE DATA 
    {
        uint8_t byte = 0;

        if(smbus_read_byte_data(i2c_bus, SMBUS_CMD_BYTE_DATA, &byte))
        {
            printf("[OK] READ BYTE DATA: 0x%02X\n", byte);
        }
        else
        {
            perror("[ER] READ BYTE DATA");
        }

        if(smbus_write_byte_data(i2c_bus, SMBUS_CMD_BYTE_DATA, byte))
        {
            printf("[OK] WRITE BYTE DATA: 0x%02X\n", byte);
        }
        else
        {
            perror("[ER] WRITE BYTE DATA");
        }
    }

    // R/W WORD DATA 
    {
        uint16_t word = 0;

        if(smbus_read_word_data(i2c_bus, SMBUS_CMD_WORD_DATA, &word))
        {
            printf("[OK] READ WORD DATA: 0x%04X\n", word);
        }
        else
        {
            perror("[ER] READ WORD DATA");
        }

        if(smbus_write_word_data(i2c_bus, SMBUS_CMD_WORD_DATA, word))
        {
            printf("[OK] WRITE WORD DATA: 0x%04X\n", word);
        }
        else
        {
            perror("[ER] WRITE WORD DATA");
        }
    }

    // R/W DWORD DATA 
    {
        uint32_t dword = 0;

        if(smbus_read_dword_data(i2c_bus, SMBUS_CMD_DWORD_DATA, &dword))
        {
            printf("[OK] READ DWORD DATA: 0x%08X\n", dword);
        }
        else
        {
            perror("[ER] READ DWORD DATA");
        }

        if(smbus_write_dword_data(i2c_bus, SMBUS_CMD_DWORD_DATA, dword))
        {
            printf("[OK] WRITE DWORD DATA: 0x%08X\n", dword);
        }
        else
        {
            perror("[ER] WRITE DWORD DATA");
        }
    }

    // R/W QWORD DATA 
    {
        uint64_t qword = 0;

        if(smbus_read_qword_data(i2c_bus, SMBUS_CMD_QWORD_DATA, &qword))
        {
            printf("[OK] READ QWORD DATA: 0x%016llX\n", qword);
        }
        else
        {
            perror("[ER] READ QWORD DATA");
        }

        if(smbus_write_qword_data(i2c_bus, SMBUS_CMD_QWORD_DATA, qword))
        {
            printf("[OK] WRITE QWORD DATA: 0x%016llX\n", qword);
        }
        else
        {
            perror("[ER] WRITE QWORD DATA");
        }
    }

    // R/W BLOCK DATA 
    {
        uint8_t block_len = 0;
        uint8_t block[SMBUS_BLOCK_MAX];
        memset(block, 0, SMBUS_BLOCK_MAX);

        if(smbus_read_block_data(i2c_bus, SMBUS_CMD_BLOCK_DATA, block, &block_len))
        {
            printf("[OK] READ BLOCK DATA (%u):", block_len);

            for(uint8_t i = 0; i < block_len; ++i)
            {
                printf(" %02X", block[i]);
            }

            printf("\n");
        }
        else
        {
            perror("[ER] READ BLOCK DATA");
        }

        if(smbus_write_block_data(i2c_bus, SMBUS_CMD_BLOCK_DATA, block, &block_len))
        {
            printf("[OK] READ BLOCK DATA (%u):", block_len);

            for(uint8_t i = 0; i < block_len; ++i)
            {
                printf(" %02X", block[i]);
            }

            printf("\n");
        }
        else
        {
            perror("[ER] WRITE BLOCK DATA");
        }
    }

    // R/W BLOCK DATA 
    {
        uint16_t request = 0xFACE;
        uint16_t response = 0;

        if(smbus_proc_call(i2c_bus, SMBUS_CMD_PROC_CALL, request, &response))
        {
            printf("[OK] PROC CALL (%04X) -> (%04X)\n", request, response);
        }
        else
        {
            perror("[ER] PROC CALL");
        }

    }

    if(!smbus_close(i2c_bus))
    {
        perror("Error closing I2C bus");
        return -1;
    }

    printf("Closed bus i2c-%u\n", PICO_I2C_BUS_NUMBER);


    return 0;
}