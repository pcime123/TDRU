
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <jni.h>
#include <math.h>
#include <stdio.h>

#include "SpiPort.h"
#include "gp22.h"

#include "common.h"
#include "termios.h"

#define Write                    0x80
#define Read                    0xB0
#define Init                    0x70
#define Power_On_Reset            0x50
#define Start_TOF                0x01
#define Start_Temp                0x02
#define Start_Cal_Resonator        0x03
#define Start_Cal_TDC            0x04
#define Start_TOF_Restart        0x05
#define Start_Temp_Restart        0x06

#define gp22_read_status_bytes()    gp22_rd_read_reg(0x04)

float CLKHS_freq = 1.000;           // Clock frequency in MHz
float CLKHS_freq_cal = 1.000;       // Calibrated Clock frequency in MHz
float CLKHS_freq_corr_fact = 1.000; // Correction factor for Clock frequency
float CLKHS_Tref = 1.000;                      //

float avg_diff_Cal2_Cal1 = 0;
float diff_Cal2_Cal1_old = 0;
float diff_Cal2_Cal1_new = 0;

float average_RAW_Result[4] = {0,};
float RAW_Result[4] = {0,};
float Time_Result[4] = {0,};
float Distance_Result[6] = {0,};
float Distance_Result_Avg[5] = {0,};

int N_Measure_Cycles;
int fd;
int i;
int j;
int n;
float bnc_length = 0.f;
unsigned int MeasurementMode = 1;
unsigned int AutoCalibration = 1;
unsigned int Dummy_var = 0;

int cable_stat;
float cable_len;

gp22_mode gp22;
gp22_result result;

unsigned char check_step = 0;
const unsigned char *stat_str[] = {
        (const unsigned char *) "Cable open",
        (const unsigned char *) "Cable short",
        (const unsigned char *) "matching or not connecting",
};

#define CONF_REG_MAX 7
const unsigned int Config_Tbl_mode_a[2][CONF_REG_MAX] = {
        {0x00276400, 0x55440002, 0xE0000001, 0x00000002, 0x20008000, 0x00000004, 0x40000000},
        {0x00076800, 0x21430002, 0xE0000001, 0x10000002, 0x10008000, 0x50000000, 0x40004000}
};

void gp22_ctl_pin(char mode) {
    int ret;
    ret = ioctl(fd, GP22_CTL_PIN_SET, &mode);
    if (ret < 0) {
        LOGW("ERROR 4");
    }
}

void gp22_reset() {
    unsigned char cmd = 0;
    int ret;
    ret = ioctl(fd, GP22_RESET, &cmd);
    if (ret < 0) {
        LOGE("GP22 RESET ERROR");
    }
}

void gp22_start_toggle() {
    unsigned char cmd = 0;
    int ret;
    ret = ioctl(fd, GP22_START_PULSE, &cmd);
    if (ret < 0) {
        LOGE("GP22 START ERROR");
    }
}

void gp22_send_1byte(unsigned char gp22_opcode_byte) {
    unsigned char cmd = gp22_opcode_byte;
    ioctl(fd, GP22_WRITE_CMD, &cmd);
}

void gp22_wr_config_reg(unsigned char config_reg, unsigned int config_reg_data) {
    gp22_reg dev_write;
    dev_write.reg_addr = config_reg;
    dev_write.value = config_reg_data;

    ioctl(fd, GP22_WRITE_REG, &dev_write);

}

unsigned int gp22_rd_read_reg(unsigned char rd_reg) {
    unsigned int Result_read = 0;
    gp22_reg dev_read;
    dev_read.reg_addr = rd_reg;
    Result_read = ioctl(fd, GP22_READ_REG, &dev_read);

    return Result_read;
}


float gp22_read_n_bytes(unsigned char read_addr, unsigned char fractional_bits) {

    gp22_reg dev_read_byte;
    float Result;
    unsigned int ret;

    dev_read_byte.reg_addr = read_addr;
    ret = ioctl(fd, GP22_READ_REG, &dev_read_byte);

//    LOGE("[GP22_DEV] read_addr: %x\n", ret);

    Result = (float) (ret / pow(2, fractional_bits));
//    LOGD("[GP22_DEV] Result: %.f\n", Result);
    return Result;
}

void gp22_analyse_error_bit(void) {
    unsigned int STAT_REG = 0x0000;
    STAT_REG = gp22_read_status_bytes();

    LOGD("STAT_REG = %04x - ", STAT_REG);
    //Bit9: Timeout_TDC
    if ((STAT_REG & 0x0200) == 0x0200) LOGD("Indicates an overflow of the TDC unit");
    //Bit10: Timeout_Precounter
    if ((STAT_REG & 0x0400) == 0x0400)
        LOGD("Indicates an overflow of the 14 bit precounter in MR 2");
    //Bit11: Error_open
    if ((STAT_REG & 0x0800) == 0x0800) LOGD("Indicates an open sensor at temperature measurement");
    //Bit12: Error_short
    if ((STAT_REG & 0x1000) == 0x1000)
        LOGD("Indicates a shorted sensor at temperature measurement");
    //Bit13: EEPROM_Error
    if ((STAT_REG & 0x2000) == 0x2000) LOGD("Single error in EEPROM which has been corrected");
    //Bit14: EEPROM_DED
    if ((STAT_REG & 0x4000) == 0x4000)
        LOGD("Double error detection. A multiple error has been detected whcich can not be corrected.");
    //Bit15: EEPROM_eq_CREG
    if ((STAT_REG & 0x8000) == 0x8000)
        LOGD("Indicates whether the content of the configuration registers equals the EEPROM");
    LOGD("\n");
}

void gp22_wait_interrupt(void) {
    n = 0;
    char data = 0;
    ioctl(fd, GP22_GET_INT_PIN, &data);
//    LOGE("Interrupt: %d\n", data);
    while (data) {
        usleep(10);
        if (++n == 420) {        // Max. measurement range = 4ms, ALU timing = max 7.26us(3V, div4)
            //if(++n == 600){
//			#ifdef _DEBUG_
//			printf("No interrupt signal!!!\n");
//			#endif
            break;
        }
    }

}


void gp22_calibrate_high_speed_clock_cycle(void) {

    gp22_send_1byte(Init);
    gp22_send_1byte(Start_Cal_Resonator);
    gp22_wait_interrupt();

    float bytes = gp22_read_n_bytes(0x00, 16);

    CLKHS_freq_corr_fact = ((float) 61.03515625 / bytes) * CLKHS_freq;
    CLKHS_freq_cal = CLKHS_freq * CLKHS_freq_corr_fact;
    CLKHS_Tref = 1000 / CLKHS_freq_cal;

#ifdef _DEBUG_
//    LOGE("[GP22_DEV] Correction factor for clock = %f\n", CLKHS_freq_corr_fact);
//    LOGE("[GP22_DEV] CLKHS_freq_cal = %f\n", CLKHS_freq_cal);
//    LOGE("[GP22_DEV] CLKHS_Tref = %f\n", CLKHS_Tref);
#endif

}

void gp22_seperate_calibration_measurement_cycle(void) {
    gp22_send_1byte(Init);
    gp22_send_1byte(Start_Cal_TDC); // update calibration data

    // Note:
    // The calibration data are not addressed directly after the calibration
    // measurement but after the next regular measurement, before the next INIT.
    gp22_wait_interrupt();
}

unsigned char gp22_caluculate_distance() {


//
//    LOGE("\n --------- Measure Mode %d Result ----------\n", gp22.measuremode);
//    LOGE("| hit | Res. Avg |  Time(ns)  | Distanc(m)   |\n");
//    LOGE(" ---------------------------------------------\n");


    for (j = 0; j < 4; j++) {

        result.average_value[j] /= gp22.measurecycles; //N_Measure_Cycles;

        result.calc_time[j] = result.average_value[j] * CLKHS_Tref;        //time [ns]

        //if(MeasurementMode == 1)	Time_Result[j] *= 4;			//time correction
        //if(MeasurementMode == 2)	Time_Result[j] -= 16;			//time correction

        // ---------------------------------------------------
        //                   velocity_of_light
        // Distance_Value = ------------------- * Time_Value
        //                           2
        // ---------------------------------------------------
        result.calc_distance[j] = (float) (result.calc_time[j] /
                                           6.671281904);    //distance [m]	// 2000000 kmps = 6.671281904 speed of light in vacuum

        //Distance_Result[j] = Distance_Result[j] - (Distance_Result[j] * 0.2);

//        LOGW("|  %d  |    %d.%03d | %5d.%03d  | %5d.%03d  | \n", j + 1,
//             result.average_value[j] / 1000, result.average_value[j] % 1000,
//             (unsigned int) result.calc_time[j], (unsigned int) (result.calc_time[j] * 1000) % 1000,
//             (unsigned int) result.calc_distance[j],
//             (unsigned int) (result.calc_distance[j] * 1000) % 1000);

    }


//    LOGE(" ------------------------------------------\n");


}

void gp22_measure_start(void) {
    gp22_send_1byte(Init);

    gp22_start_toggle();

    gp22_wait_interrupt();
}

float gp22_measure_read(void) {
    unsigned int read_stat = 0;
    unsigned char read_addr = 0;
    float Result = 0;

    //-------------------------------------------------------------------
    // read ALU operation pointer. Pointer to the result register.
    read_stat = gp22_read_status_bytes();
//    LOGE("gp22_measure_read: stat = %04x\n", read_stat);

//	if(read_stat&0x0600){			// timeout error
//		#ifdef _DEBUG_
//		printf("timeout error(reg 4 = %04x)\n", read_stat);
//		#endif
//		return 0;
//	}
    //-------------------------------------------------------------------
    // read result register
    read_addr = (read_stat & 0x07) - 1;
//    LOGE("GP22 Measure addr: %x\n", read_addr);

    Result = gp22_read_n_bytes(read_addr, 16);
//    LOGE("GP22 Measure result: %f\n", Result);
    return Result;
}


void gp22_init(unsigned int mode) {

    unsigned int reg_data;
    gp22_reg config;
    unsigned char i, n;

    gp22_reset();

    gp22.measuremode = mode;
    gp22.measurecycles = 10;

    for (n = 0; n < CONF_REG_MAX; n++) {
        gp22.config_data[n] = Config_Tbl_mode_a[gp22.measuremode - 1][n];
    }

//    LOGD("[GP22_DEV] init [mode %d]...\n", gp22.measuremode);

    gp22_send_1byte(Power_On_Reset);

    usleep(500);

    for (i = 0; i < CONF_REG_MAX; i++) {
        config.reg_addr = i;
        config.value = gp22.config_data[i];

//        LOGD("config data: %d", gp22.config_data[i]);

        gp22_wr_config_reg(config.reg_addr, config.value);

    }

    usleep(500);

    if (gp22.measuremode == 1) CLKHS_freq = 1.000;
    if (gp22.measuremode == 2) CLKHS_freq = 4.000;

    gp22_calibrate_high_speed_clock_cycle();

    if ((gp22.measuremode == 1) && (AutoCalibration == 0)) {
        gp22_seperate_calibration_measurement_cycle();
    }

    usleep(500);

    gp22_send_1byte(Init);

//    usleep(500);

    gp22_ctl_pin(0);

//    reg_data = gp22_rd_read_reg(0x7);
//    LOGD("[GP22_DEV] reg 0x07 = %08x", reg_data);
//    if (reg_data == 0x00020102) LOGD("...done\n");
//    else
//        LOGE("...fail\n");
//
//    Dummy_var = 0;

}

void gp22_re_init(unsigned int mode) {

    gp22_reset();


    //gp22_send_1byte(Power_On_Reset);


    gp22_init(mode);

}

void gp22_measure_proc(void) {
    unsigned int h = 0, buff = 0;
    unsigned int read_stat = 0;
    unsigned int hits_ch1 = 0;
    char high = 1;
    char low = 0;
    cable_stat = -1;

    result.average_value[0] = 0;
    result.average_value[1] = 0;
    result.average_value[2] = 0;
    result.average_value[3] = 0;

    ioctl(fd, GP22_BNC_PIN_SET, &high);

    //--------------------------------------------------------------------------
    for (i = 1; i <= gp22.measurecycles; i++) {
//        LOGD("Measure Start!");
        gp22_measure_start();

        read_stat = gp22_read_status_bytes();

        hits_ch1 = (read_stat >> 3) & 0x7;
//        LOGD("%02d: stat = %04x -> hit ch1 = %d\n", i, read_stat, hits_ch1);

        for (h = 0; h < hits_ch1; h++) {
            buff = (gp22.config_data[1] & 0x00ffffff);
            if (gp22.measuremode == 1) {
//                LOGE("Measure1 !");

                buff |= ((h + 1) << 24);                        // stop1_j-start
            }
            if (gp22.measuremode == 2) {
//                LOGE("Measure2 !");

                buff |= (0x01000000 | ((h + 2) << 28));            // stop1_j-start
            }

            gp22_wr_config_reg(0x1, buff);
//            usleep(8);
            gp22_wait_interrupt();                    // ALU wait
//            LOGE("Measure complete!");

            result.read_value[h] = gp22_measure_read();
//            LOGD(" %02d. Measure RAW Value = %.3f \n", i, result.read_value[h]);    // RAW value
            result.average_value[h] += result.read_value[h];
        }
    }

    //--------------------------------------------------------------------------
    // Caluculate
    gp22_caluculate_distance();

    //--------------------------------------------------------------------------
    buff = (gp22.config_data[1] & 0x00ffffff);
    buff |= 0x55000000;                            // no action
    gp22_wr_config_reg(0x1, buff);

    ioctl(fd, GP22_BNC_PIN_SET, &low);


    int ret;
    switch (check_step) {
        case 0:
            result.calc_distance[4] = result.calc_distance[0];
            result.calc_distance[5] = result.calc_distance[1];
//            LOGE("[GP22_DEV] Step 0. Distance Result[5] = %.f\n", result.calc_distance[5]);
            check_step++;
            gp22_ctl_pin(1);
            usleep(500);
            return;
        case 1:
            if ((result.calc_distance[0] <= 10) || (result.calc_distance[0] >= 300)) {
                if ((result.calc_distance[5] <= 10) || (result.calc_distance[5] >= 300)) {
                    check_step++;
                    gp22_init(2);
                    usleep(500);

                    return;
                } else {
                    check_step = 0;
//                    LOGE("[GP22_DEV] Step 1. Distance Result[5] = %.f\n", result.calc_distance[5]);
                }
            } else if ((result.calc_distance[5] <= 10) || (result.calc_distance[5] >= 300)) {
                result.calc_distance[5] = result.calc_distance[0];
            } else if (result.calc_distance[5] > result.calc_distance[0]) {
                result.calc_distance[5] = result.calc_distance[0];
            } else {
                check_step = 0;
//                LOGE("[GP22_DEV] Step 2. Distance Result[5] = %.f\n", result.calc_distance[5]);
            }
            break;
        case 2:
            if (result.calc_distance[0] <= 75) {
                check_step++;
                gp22_ctl_pin(1);

                usleep(500);

                return;
            } else {
                result.calc_distance[5] = result.calc_distance[0];
                check_step = 0;
            }
            break;
        case 3:
            if (result.calc_distance[0] == 0) {
                check_step = 2;
            } else {
                result.calc_distance[5] = result.calc_distance[0];
                check_step = 1;
            }
            break;

        default:
            break;
    }

    bnc_length = result.calc_distance[5] - result.calc_distance[4];
    LOGW("TDRC Done. stat: %s, Length:%.f\n", stat_str[check_step], bnc_length);

    cable_stat = check_step;
    check_step = 0;
    result.calc_distance[4] = 0;
    result.calc_distance[5] = 0;
    result.calc_distance[5] = 0;


    gp22_init(1);



//    gp22_init(1);

}

JNIEXPORT jobject JNICALL Java_com_sscctv_seeeyes_SpiPort_open
        (JNIEnv *env, jclass thiz, jstring path) {

    jobject mFileDescriptor;

    gp22_reg gp22_register;
    gp22_register.reg_addr;
    gp22_register.value;


    /* Opening device */
    {
        jboolean iscopy;
        const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
        LOGD("Opening serial port %s with flags 0x%x", path_utf, O_RDWR);
        fd = open(path_utf, O_RDWR);
        LOGD("open() fd = %d", fd);
        (*env)->ReleaseStringUTFChars(env, path, path_utf);
        if (fd == -1) {
            /* Throw an exception */
            LOGE("Cannot open port");
            /* TODO: throw an exception */
            return NULL;
        }
    }
    gp22_init(1);


    {
        jclass cFileDescriptor = (*env)->FindClass(env, "java/io/FileDescriptor");
        jmethodID iFileDescriptor = (*env)->GetMethodID(env, cFileDescriptor, "<init>", "()V");
        jfieldID descriptorID = (*env)->GetFieldID(env, cFileDescriptor, "descriptor", "I");
        mFileDescriptor = (*env)->NewObject(env, cFileDescriptor, iFileDescriptor);
        (*env)->SetIntField(env, mFileDescriptor, descriptorID, (jint) fd);
    }

    return mFileDescriptor;
}

JNIEXPORT int JNICALL Java_com_sscctv_seeeyes_SpiPort_start
        (JNIEnv *env, jclass thiz) {
    int ret = 0;
//    gp22_init(1);

    do {
        gp22_measure_proc();
    } while (check_step != 0);

    return ret;
}


JNIEXPORT int JNICALL Java_com_sscctv_seeeyes_SpiPort_setTest
        (JNIEnv *env, jclass thiz) {
    int ret = 0;
    gp22_init(1);


    return ret;
}

JNIEXPORT void JNICALL Java_com_sscctv_seeeyes_SpiPort_printOut(JNIEnv *env, jobject obj) {
    puts("Hello Jni");
}

JNIEXPORT jfloat JNICALL
Java_com_sscctv_seeeyes_SpiPort_getLength(JNIEnv *env, jobject obj) {
    return bnc_length;
}

JNIEXPORT int JNICALL
Java_com_sscctv_seeeyes_SpiPort_getStat(JNIEnv *env, jobject obj) {
    return cable_stat;
}



