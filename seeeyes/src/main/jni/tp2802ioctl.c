#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "tp2802.h"
#include "tp28xx_ptz.h"

#define TEST 0

// Function prototypes
int convert_string_to_hex(char *string, unsigned int *value);

PTZ_packet ptz;
unsigned char tilt_speed=0x01;
unsigned char pan_speed=0x01;
unsigned char presetNUM=0x00;

void PacketPTZData( PTZ_packet *ptz)
{
  ptz->header = 0xb5;
  ptz->addr = 0x00;
  ptz->sum = ptz->header + ptz->addr + ptz->cmd + ptz->data[0] + ptz->data[1] + ptz->data[2] + ptz->data[3];

}

void HandleCommand( unsigned cmd)
{
    switch (cmd)
    {
        case KEY_TILT_UP:
        case KEY_TILT_DOWN:
                            ptz.cmd = cmd;
                            ptz.data[0] = tilt_speed;
                            ptz.data[1] = 0x00;
                            ptz.data[2] = 0x00;
                            ptz.data[3] = 0x00;
                            break;
        case KEY_PAN_RIGHT:
        case KEY_PAN_LEFT:
                            ptz.cmd = cmd;
                            ptz.data[0] = 0x00;
                            ptz.data[1] = pan_speed;
                            ptz.data[2] = 0x00;
                            ptz.data[3] = 0x00;
                            break;
        case KEY_LEFT_UP:
        case KEY_LEFT_DOWN:
        case KEY_RIGHT_UP:
        case KEY_RIGHT_DOWN:
                            ptz.cmd = cmd;
                            ptz.data[0] = tilt_speed;
                            ptz.data[1] = pan_speed;
                            ptz.data[2] = 0x00;
                            ptz.data[3] = 0x00;
                            break;

        case KEY_PRESET_SET:
        case KEY_PRESET_CLR:
        case KEY_PRESET_CALL:
                            ptz.cmd = cmd;
                            ptz.data[0] = presetNUM;
                            ptz.data[1] = 0x00;
                            ptz.data[2] = 0x00;
                            ptz.data[3] = 0x00;
                            break;
        case KEY_IRIS_CLOSE:
        case KEY_IRIS_OPEN:
        case KEY_FOCUS_NEAR:
        case KEY_FOCUS_FAR:
        case KEY_ZOOM_WIDE:
        case KEY_ZOOM_TELE:
        case KEY_MOTOR_STOP:
                            ptz.cmd = cmd;
                            ptz.data[0] = 0x00;
                            ptz.data[1] = 0x00;
                            ptz.data[2] = 0x00;
                            ptz.data[3] = 0x00;
                            break;
        default:
                    printf("ERROR: Invalid command detected!\n");
                    break;

    }

    PacketPTZData(&ptz);

}


int main(int argc , char* argv[])
{
	int fd = -1;
	int ret = 0;
	int i;

    tp2802_register		   dev_register;
    tp2802_image_adjust    image_adjust;
    tp2802_work_mode       work_mode;
    tp2802_video_mode	   video_mode;
    tp2802_video_loss      video_loss;
    tp2802_PTZ_data         PTZ_data;
    tp2802_audio_playback  audio_playback ;
    tp2802_audio_da_volume audio_da_volume;
    tp2802_audio_da_mute audio_da_mute;
    tp2802_audio_samplerate samplerate;

	if( argv[1] == NULL )
    {
    	printf("Usage:\n");
    	printf("./tp2802ioctl <CMD ID> <CMD ARGS>\n\n");
    	printf("\t<CMD ID> = 1 (TP2802_READ_REG)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  reg_addr(hex)  channel(0-3, is reg_addr<0x40)\n\n");
    	printf("\t<CMD ID> = 2 (TP2802_WRITE_REG)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  reg_addr(hex)  value(hex)  channel(0-3, None=ALL)\n\n");
    	printf("\t<CMD ID> = 3 (TP2802_SET_VIDEO_MODE)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  mode(3=1080P25,2=1080P30,5=720P25,4=720P30,1=720P50,"
    			"0=720P60)  channel(0-3, None=ALL)\n\n");
    	printf("\t<CMD ID> = 8 (TP2802_SET_PTZ_DATA)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  PTZ_CMD(hex)  PTZ_ARG1(hex)  PTZ_ARG2(hex)  PTZ_ARG3(hex)"
    			"  PTZ_ARG4(hex)  channel(0-3)\n\n");
    	printf("\t<CMD ID> = 10 (TP2802_SCAN_MODE)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  SCAN_MODE(hex)  channel(0-3, is reg_addr<0x40)\n\n");
        printf("\t<CMD ID> = 11 (TP2802_DUMP_REG)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  \n\n");
    	printf("\t<CMD ID> = 12 (TP2802_FORCE_DETECT)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  mode  channel(0-3, None=ALL) \n\n");
        printf("\t<CMD ID> = 13 (TP2802_SET_SAMPLE_RATE)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  samplerate(0->8K,1->16K) \n\n");
     	printf("\t<CMD ID> = 14 (TP2802_SET_AUDIO_PLAYBACK)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  audio_chn \n\n");
        printf("\t<CMD ID> = 15 (TP2802_SET_AUDIO_DA_VOLUME)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  audio_volume \n\n");
        printf("\t<CMD ID> = 15 (TP2802_SET_AUDIO_DA_MUTE)\n");
    	printf("\t\t\t<CMD ARGS> = chip(dec)  flag  \n\n");
    	return -1;
    }

#if(!TEST)
	// Open file
	fd = open("/dev/tp2802dev", 0);
	if (fd < 0)
	{
		printf("Can't open /dev/tp2802\n");
		return -1;
	}
#endif

	switch(atoi(argv[1]))
	{
	case 1: // TP2802_READ_REG
		dev_register.chip = atoi(argv[2]);
		convert_string_to_hex(argv[3], &dev_register.reg_addr);

		if( argv[4] == NULL )
			dev_register.ch = 4;
		else
			dev_register.ch = atoi(argv[4]);

	    ret = ioctl(fd, TP2802_READ_REG, &dev_register);

		printf("Sent command:\t\tioctl(fd, TP2802_READ_REG, &dev_register)\n");
		printf("Register Read:\n\n");
		printf("dev_register.chip:"
					   "\t%d\n", dev_register.chip);
		printf("dev_register.ch:\t%d\n", dev_register.ch);
		printf("dev_register.reg_addr:\t0x%02x\n", dev_register.reg_addr);
		printf("dev_register.value:\t0x%02x\n\n", dev_register.value);

	    break;
	case 2: // TP2802_WRITE_REG
		dev_register.chip = atoi(argv[2]);
		convert_string_to_hex(argv[3], &dev_register.reg_addr);
		convert_string_to_hex(argv[4], &dev_register.value);

		if( argv[5] == NULL )
			dev_register.ch = 4;
		else
			dev_register.ch = atoi(argv[5]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_WRITE_REG, &dev_register);
#else
		printf("Send command:\t\tioctl(fd, TP2802_WRITE_REG, &dev_register)\n");
		printf("dev_register.chip:\t%d\n", dev_register.chip);
		printf("dev_register.ch:\t%d\n", dev_register.ch);
		printf("dev_register.reg_addr:\t0x%02x\n", dev_register.reg_addr);
		printf("dev_register.value:\t0x%02x\n", dev_register.value);
#endif
	    break;

	case 3: // TP2802_SET_VIDEO_MODE
		work_mode.chip = atoi(argv[2]);
		work_mode.mode = atoi(argv[3]);
		/*if( (work_mode.mode < 0) || (work_mode.mode > 6))
		{
			printf("ERROR: Invalid mode, must be 0-5.\n");
			return -1;
			break;
		}*/

		if( argv[4] == NULL )
			work_mode.ch = 4;
		else
			work_mode.ch = atoi(argv[4]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_VIDEO_MODE, &work_mode);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_VIDEO_MODE, &work_mode)\n");
		printf("work_mode.chip:\t\t%d\n", work_mode.chip);
		printf("work_mode.ch:\t\t%d\n", work_mode.ch);
		printf("work_mode.mode:\t\t%d\n", work_mode.mode);
#endif
		break;

	case 6: // TP2802_SET_IMAGE_ADJUST
		image_adjust.chip = atoi(argv[2]);
		convert_string_to_hex(argv[3], &ptz.cmd);
		convert_string_to_hex(argv[4], &image_adjust.contrast);
		convert_string_to_hex(argv[5], &image_adjust.saturation);
		convert_string_to_hex(argv[6], &image_adjust.hue);
		convert_string_to_hex(argv[7], &image_adjust.sharpness);

		if( argv[8] == NULL )
			image_adjust.ch = 4;
		else
			image_adjust.ch = atoi(argv[8]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_IMAGE_ADJUST, &image_adjust);
#else
		printf("Send command:\t\t\tioctl(fd, TP2802_SET_IMAGE_ADJUST, &image_adjust)\n");
		printf("image_adjust.chip:\t\t%d\n", image_adjust.chip);
		printf("image_adjust.ch:\t\t%d\n", image_adjust.ch);
		printf("image_adjust.brightness:\t0x%02x\n", image_adjust.brightness);
		printf("image_adjust.contrast:\t\t0x%02x\n", image_adjust.contrast);
		printf("image_adjust.saturation:\t0x%02x\n", image_adjust.saturation);
		printf("image_adjust.hue:\t\t0x%02x\n", image_adjust.hue);
		printf("image_adjust.sharpness:\t\t0x%02x\n", image_adjust.sharpness);
#endif
		break;
	case 8: // TP2802_SET_PTZ_DATA

		convert_string_to_hex(argv[3], &ptz.cmd);
		convert_string_to_hex(argv[4], &ptz.data[0]);
		convert_string_to_hex(argv[5], &ptz.data[1]);
		convert_string_to_hex(argv[6], &ptz.data[2]);
		convert_string_to_hex(argv[7], &ptz.data[3]);
		if(0xff == ptz.cmd)
		{
            PTZ_data.data[0] = ptz.data[0];
            PTZ_data.data[1] = ptz.data[1];
            PTZ_data.data[2] = ptz.data[2];
            PTZ_data.data[3] = ptz.data[3];
            PTZ_data.mode = 1;
		}
		else
		{
            PacketPTZData(&ptz);

            PTZ_data.data[0] = ptz.header;
            PTZ_data.data[1] = ptz.addr;
            PTZ_data.data[2] = ptz.cmd;
            PTZ_data.data[3] = ptz.data[0];
            PTZ_data.data[4] = ptz.data[1];
            PTZ_data.data[5] = ptz.data[2];
            PTZ_data.data[6] = ptz.data[3];
            PTZ_data.data[7] = ptz.sum;
            PTZ_data.mode = 0;
		}

        PTZ_data.chip = atoi(argv[2]);
        PTZ_data.ch = atoi(argv[8]);
		if( argv[8] == NULL )
			PTZ_data.ch = 0;
		else
			PTZ_data.ch = atoi(argv[8]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_PTZ_DATA, &PTZ_data);
#else
		printf("Send command:\t\t\tioctl(fd, TP2802_SET_PTZ_DATA, &PTZ_data)\n");
		printf("PTZ_data.chip:\t\t%d\n", PTZ_data.chip);
		printf("PTZ_data.ch:\t\t%d\n", PTZ_data.ch);
		printf("PTZ_data.command:\t0x%02x\n", PTZ_data.data[2]);
		printf("PTZ_data.parameter1:\t0x%02x\n", PTZ_data.data[3]);
		printf("PTZ_data.parameter2:\t0x%02x\n", PTZ_data.data[4]);
		printf("PTZ_data.parameter3:\t0x%02x\n", PTZ_data.data[5]);
		printf("PTZ_data.parameter4:\t0x%02x\n", PTZ_data.data[6]);
#endif
		break;
	case 10: // TP2802_SCAN_MODE
		work_mode.chip = atoi(argv[2]);
		work_mode.mode = atoi(argv[3]);
		if( argv[4] == NULL )
			work_mode.ch = 4;
		else
			work_mode.ch = atoi(argv[4]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_SCAN_MODE, &work_mode);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_SCAN_MODE, &work_mode)\n");
		printf("scan_mode.chip:\t%d\n", work_mode.chip);
		printf("scan_mode.ch:\t%d\n", work_mode.ch);
		printf("scan_mode.mode:\t%d\n", work_mode.mode);
#endif
	    break;
	case 11: // TP2802_DUMP_REG
		dev_register.chip = atoi(argv[2]);
		//printf("Sent command:\t\tioctl(fd, TP2802_DUMP_REG, &dev_register)\n");
		//printf("dev_register.chip:\t%d\n", dev_register.chip);
	    ret = ioctl(fd, TP2802_DUMP_REG, &dev_register);
	    break;
	case 12: // TP2802_FORCE_DETECT
		work_mode.chip = atoi(argv[2]);
		work_mode.mode = atoi(argv[3]);
		if( argv[4] == NULL )
			work_mode.ch = 4;
		else
			work_mode.ch = atoi(argv[4]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_FORCE_DETECT, &work_mode);
#else
		printf("Send command:\t\tioctl(fd, TP2802_FORCE_DETECT, &work_mode)\n");
		printf("reset.chip:\t%d\n", work_mode.chip);
		printf("reset.ch:\t%d\n", work_mode.ch);
		printf("reset.mode:\t%d\n", work_mode.mode);
#endif
        break;
	case 13: // TP2802_SET_SAMPLE_RATE

		samplerate = atoi(argv[2]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_SAMPLE_RATE, &samplerate);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_SAMPLE_RATE, &samplerate)\n");
		printf("samplerate:\t%d\n", samplerate);
#endif
        break;
	case 14: // TP2802_SET_AUDIO_PLAYBACK
		audio_playback.chip = atoi(argv[2]);
		audio_playback.chn = atoi(argv[3]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_AUDIO_PLAYBACK, &audio_playback);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_AUDIO_PLAYBACK, &audio_playback)\n");
		printf("audio_playback.chip:\t%d\n", audio_playback.chip);
		printf("audio_playback.chn:\t%d\n", audio_playback.chn);
#endif
        break;
	case 15: // TP2802_SET_AUDIO_DA_VOLUME
		audio_da_volume.chip = atoi(argv[2]);
		audio_da_volume.volume = atoi(argv[3]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_AUDIO_DA_VOLUME, &audio_da_volume);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_AUDIO_DA_VOLUME, &audio_da_volume)\n");
		printf("audio_da_volume.chip:\t%d\n", audio_da_volume.chip);
		printf("audio_da_volume.volume:\t%d\n", audio_da_volume.volume);
#endif
        break;
	case 16: // TP2802_SET_AUDIO_DA_MUTE
		audio_da_mute.chip = atoi(argv[2]);
		audio_da_mute.flag = atoi(argv[3]);

#if(!TEST)
	    ret = ioctl(fd, TP2802_SET_AUDIO_DA_MUTE, &audio_da_mute);
#else
		printf("Send command:\t\tioctl(fd, TP2802_SET_AUDIO_DA_MUTE, &audio_da_mute)\n");
		printf("audio_da_mute.chip:\t%d\n", audio_da_mute.chip);
		printf("audio_da_mute.flag:\t%d\n", audio_da_mute.flag);
#endif
        break;
	default:
		printf("ERROR: Invalid IOCTL command detected!\n");
		ret = -1;
		break;
	}

#if(!TEST)
	close(fd);
#endif
	return ret;
}

int convert_string_to_hex(char *string, unsigned int *value)
{
    int ret = sscanf(string, "%x", value);
    return ret;
}
