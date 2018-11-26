//
// Created by JinseopKim on 2018-08-06.
//

#ifndef TDR_GP22_GP22_H
#define TDR_GP22_GP22_H

#include <linux/ioctl.h>

#define HIGH        1
#define LOW        0

#define _DEBUG_;

typedef struct _gp22_reg {
    unsigned char reg_addr;
    unsigned int value;
} gp22_reg;

typedef struct _gp22_config {
    unsigned int addr;
    unsigned int config_data;
} gp22_config;

typedef struct _gp22_mode {
    unsigned int measuremode;
    unsigned char measurecycles;
    unsigned int config_data[7];
} gp22_mode;

typedef struct gp22_result {
    unsigned int average_value[4];
    float read_value[4];
    float calc_time[4];
    float calc_distance[6];
} gp22_result;

#define GP22_IOC_MAGIC        'p'

#define GP22_WRITE_REG        _IOW(GP22_IOC_MAGIC, 1, gp22_reg)
#define GP22_READ_REG        _IOR(GP22_IOC_MAGIC, 2, gp22_reg)
#define GP22_WRITE_CMD        _IOW(GP22_IOC_MAGIC, 3, unsigned char)
#define GP22_SET_MODE        _IOW(GP22_IOC_MAGIC, 4, gp22_mode)
#define GP22_MEASURE        _IOWR(GP22_IOC_MAGIC, 5, gp22_result)
#define GP22_START_PULSE    _IOW(GP22_IOC_MAGIC, 6, unsigned char)
#define GP22_GET_INT_PIN    _IOR(GP22_IOC_MAGIC, 7, unsigned char*)
#define GP22_READ_BYTES        _IOR(GP22_IOC_MAGIC, 8, gp22_bytes)
#define GP22_RESET            _IOW(GP22_IOC_MAGIC, 9, unsigned char)
#define GP22_BNC_PIN_SET    _IOW(GP22_IOC_MAGIC, 10, unsigned char)
#define GP22_CTL_PIN_SET    _IOW(GP22_IOC_MAGIC, 11, unsigned char)

#endif //TDR_GP22_GP22_H
