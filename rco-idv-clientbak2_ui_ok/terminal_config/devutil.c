/*
 * devutil.c
 *
 *  Created on: 2017-1-24
 *      Author: chenw
 */

#include "common.h"
#include "rc_log.h"

display_info_t local_disply;

#define	LOGIN_CONFIG "/opt/lessons/RCC_Client/logic.ini"
#define	HOST_NAME_CONFIG "/opt/lessons/RCC_Client/logic_configured.ini"
#define DISPLAY_IFNO	"/tmp/xrandr_outfile"
#define DISPLAY_IFNO2	"/tmp/xrandr.out"

int get_disply_info(int *width, int *height, int *refresh)
{
    FILE *fp = NULL;
    char buf[128] = {0,};
    char tmp_display[32] = {0};
    char str1[32] = {0,};
    char str_w[32] = {0,};
    char str_h[32] = {0,};
    int width_len = 0;
    int num = 0;

    if (access(DISPLAY_IFNO, F_OK) == 0) {
        fp = fopen(DISPLAY_IFNO, "r");
    } else if (access(DISPLAY_IFNO2, F_OK) == 0) {
        fp = fopen(DISPLAY_IFNO2, "r");
    } else {
        return -1;
    }

    if (fp == NULL) {
        LOG_ERR("fopen error:%s", strerror(errno));
        return -1;
    }
    while (fgets(buf,128,fp))
    {
        num = sscanf(buf,"%s %s %*s",tmp_display,str1);
        if (num == 0) {
            break;
        }
        if (!isdigit(tmp_display[0])) {
            continue;
        }


        if (strchr(buf,'*') != NULL) {
            break;
        }
        memset(tmp_display,0,32);
        memset(str1,0,32);
        //memset(str2,0,128);
    }
    pclose(fp);
    fp = NULL;
	if((0 == strlen(str1)) || (0 == strlen(tmp_display)) || (!isdigit(tmp_display[0]))){
        return -1;
    }
    *refresh = 60;

    width_len = strlen(tmp_display) - strlen(strchr(tmp_display,'x'));
    strncpy(str_w, tmp_display, width_len);
    strcpy(str_h,strchr(tmp_display,'x')+1);

    *width = atoi(str_w);
    *height = atoi(str_h);
    return 0;
}


int get_serialnum(char * sn, int size)
{
	FILE* fp = NULL;
    char result[128];
	char* tmp;
    char command[128];
    int ret = 0;

    sprintf(command, "dmidecode -t system | grep 'Serial Number'|cut -d : -f 2|sed s/[[:space:]]//g" );

    if((fp = popen(command,"r")) == NULL) {
    	LOG_ERR("popen error:%s", strerror(errno));
        return -1;
    }

	if((fgets(result,sizeof(result),fp)) == NULL) {
		ret = -1;
		goto end;
		//strncpy(serialNumber, result, size-1);
	}

	tmp = strchr(result, '\n');
    if(tmp) {
        *tmp = '\0';
    }

	if((strstr(result, "To be filled by O.E.M") != NULL) || (strstr(result, "empty") != NULL)) {
		ret = -1;
		goto end;
	}
	strncpy(sn, result, size);
end:
	if (fp) {
		pclose(fp);
	}
    return ret;
}

int get_dev_mode(int * mode)
{
	IniFile* ini;
	int tmp;

	if(access(LOGIN_CONFIG, F_OK) != 0) {
		return -1;
	}
	ini = ini_file_open_file(LOGIN_CONFIG);
	if (ini == NULL ) {
		return -1;
	}

	if (ini_file_read_int(ini,"default","mode", &tmp) == true) {
		//* mode = tmp;
		switch(tmp) {
			case 0:
				*mode = PRI_MODE;
				break;
			case 1:
				*mode = MUTI_MODE;
				break;
			case 2:
				*mode = CMN_MODE;
				break;
			default:
				break;
		}
	}
	ini_file_free(ini);

	return 0;
}

int get_hostname(char * buf, int size)
{
	IniFile* ini;
	char * tmp;

	if(access(HOST_NAME_CONFIG, F_OK) != 0) {
		return -1;
	}
	ini = ini_file_open_file(HOST_NAME_CONFIG);
	if (ini == NULL ) {
		return -1;
	}

	ini_file_read_string(ini,"default","hostname", &tmp);
	if (tmp) {
		strncpy(buf, tmp, size - 1);
		g_free(tmp);
	}
	ini_file_free(ini);

	return 0;
}

/*
void update_display_info(display_info_t *disply)
{
    FILE *fp;
    char buf[128];
    char tmp_display[20];
    char flags[128];
    char buf1[128];
    char buf2[128];
    char buf3[128];
    char buf4[128];
    char buf5[128];
    char width[6];
    char height[6];
    int num;
    int len;

	if(access("/usr/setdisplay.sh", F_OK) == -1){
		return;
	}
	//log_info("update resolution\n");

    system("sort /usr/setdisplay.sh | grep xrandr | uniq > /tmp/display2");
    fp = fopen("/tmp/display2","r");
    if (fp == NULL) {
    	log_err("%s", strerror(errno));
        return;
    }

    memset(buf,0,128);
    memset(tmp_display,0,20);
    memset(flags,0,128);
    memset(buf1,0,128);
    memset(buf2,0,128);
    memset(buf3,0,128);
    memset(buf4,0,128);
    memset(buf5,0,128);
    while (fgets(buf,128,fp))
    {
        num = sscanf(buf,"%s %s %s %s %s %s %s",buf1,buf2,buf4,buf5,tmp_display,buf3,flags);

        if (num == 0) {
            break;
        }

        if (!isdigit(tmp_display[0])) {
            continue;
        }

        len =strlen(tmp_display) - strlen(strchr(tmp_display,'x'));
        memset(width,'\0',sizeof(width));
        memset(height,'\0',sizeof(height));
        strncpy(width,tmp_display,len);
        strcpy(height,strchr(tmp_display,'x')+1);
        disply->height = atoi(height);
        disply->width = atoi(width);
        disply->refresh = atoi(flags);
        break;

        memset(buf,0,128);
    }
    fclose(fp);
}
*/
