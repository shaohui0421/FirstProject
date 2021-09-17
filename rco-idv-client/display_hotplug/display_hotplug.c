#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#define UEVENT_BUFFER_SIZE      2048
#define MAX_RES_NUM             16
#define MAX_DIS_NUM             2
#define MAX_TYPE_LEN            16
#define MAX_RES_LEN             16
#define MAX_CMD_LEN             512
#define MAX_BUF_LEN             256

typedef struct display_info {
    int exist;
    int primary;
    int res_num;
    char type[MAX_TYPE_LEN];
    char resolution[MAX_RES_NUM][MAX_RES_LEN];
} display_info_t;

enum MonitorStatus{
    CONNECTED,
    DISCONNECTED,
};

static void set_same_resolution()
{
    FILE *fp = NULL;
    char buf[MAX_BUF_LEN] = { 0 };
    char str1[MAX_BUF_LEN] = { 0 };
    char str2[MAX_BUF_LEN] = { 0 };
    char str3[MAX_BUF_LEN] = { 0 };
    char cmd[MAX_CMD_LEN] = { 0 };
    display_info_t display[MAX_DIS_NUM + 1] = { 0 };
    int width = 0;
    int heigth = 0;    
    int dis_index = -1;
    int res_index = -1;
    int num;
    int i, j;
    
    fp = popen("xrandr -q", "r");
    if (fp == NULL) {
        return;
    }
        
    while (fgets(buf, sizeof(buf), fp)) {       
        num = sscanf(buf, "%s %s %s", str1, str2, str3);
        if (num == 0) {
            goto buf_clear;
        }
        
        if (!strcmp(str1, "VIRTUAL1")) {
            break;
        }

        if(!strcmp(str1, "Screen") || !strcmp(str2, "disconnected")) {
            goto buf_clear;
        }
        
        if ( (!strncmp(str1, "eDP", 3) && !strcmp(str2, "connected"))  
                || (!strncmp(str1, "DP", 2) && !strcmp(str2, "connected"))   
                || (!strncmp(str1, "HDMI1", 5) && !strcmp(str2, "connected"))   
                || (!strncmp(str1, "HDMI2", 5) && !strcmp(str2, "connected"))   
                || (!strncmp(str1, "HDMI", 4) && !strcmp(str2, "connected")) ) {
  
            dis_index++; 
            if(dis_index >= MAX_DIS_NUM) {
                break;
            }
            
            res_index = -1;
            display[dis_index].exist = 1;
            strcpy(display[dis_index].type, str1); 
            if (!strncmp(str3, "primary", 7)) {
                display[dis_index].primary = 1;
            }  
            goto buf_clear;
        }
        
        num = sscanf(str1, "%dx%d", &width, &heigth);
        if (num != 2) {
            goto buf_clear;
        }
        
        if (res_index < (MAX_RES_NUM - 1)) {
            display[dis_index].res_num = display[dis_index].res_num + 1;
            strcpy(display[dis_index].resolution[++res_index], str1);
        }

buf_clear:
        memset(str1, 0, MAX_BUF_LEN);
        memset(str2, 0, MAX_BUF_LEN);
        memset(str3, 0, MAX_BUF_LEN);
        memset(buf, 0, MAX_BUF_LEN);
    }
    
    pclose(fp);
    fp = NULL;
        
    if (display[0].exist && display[1].exist) {
        for (i = 0; i < display[0].res_num; i++) {
            for (j = 0; j < display[1].res_num; j++) {
                if (strcmp(display[0].resolution[i], display[1].resolution[j])) {
                    continue;
                }
                                
                if (display[0].primary) {
                    sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                        display[1].type, display[0].type, display[0].resolution[i], display[0].type, display[0].resolution[i]);
                    if (system(cmd) < 0) {
                        printf("exec cmd failed: %s", cmd);
                    }
                    return;
                } else if (display[1].primary) {
                    sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                        display[0].type, display[1].type, display[1].resolution[j], display[1].type, display[1].resolution[j]);
                    if (system(cmd) < 0) {
                        printf("exec cmd failed: %s", cmd);
                    }
                    return;
                } else {
                    if (!strncmp(display[0].type, "HDMI2", 5)) {
                        sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                            display[0].type, display[1].type, display[1].resolution[j], display[1].type, display[1].resolution[j]);
                        if (system(cmd) < 0) {
                            printf("exec cmd failed: %s", cmd);
                        }
                        return;
                    } else if (!strncmp(display[1].type, "HDMI2", 5)) {
                        sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                            display[1].type, display[0].type, display[0].resolution[i], display[0].type, display[0].resolution[i]);
                        if (system(cmd) < 0) {
                            printf("exec cmd failed: %s", cmd);
                        }
                        return;
                    } else if (!strncmp(display[0].type, "HDMI", 4)) {
                        sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                            display[0].type, display[1].type, display[1].resolution[j], display[1].type, display[1].resolution[j]);
                        if (system(cmd) < 0) {
                            printf("exec cmd failed: %s", cmd);
                        }
                        return;
                    } else if (!strncmp(display[1].type, "HDMI", 4)) {
                        sprintf(cmd, "xrandr --output %s --same-as %s --mode %s --output %s --primary --mode %s", 
                            display[1].type, display[0].type, display[0].resolution[i], display[0].type, display[0].resolution[i]);
                        if (system(cmd) < 0) {
                            printf("exec cmd failed: %s", cmd);
                        }
                        return;
                    }
                }
            }
        }
    } else if (display[0].exist) {
        sprintf(cmd, "xrandr --output %s --mode %s --primary", display[0].type, display[0].resolution[0]);
        if (system(cmd) < 0) {
            printf("exec cmd failed: %s", cmd);
        }
    } else if (display[1].exist) {
        sprintf(cmd, "xrandr --output %s --mode %s --primary", display[1].type, display[1].resolution[0]);
        if (system(cmd) < 0) {
            printf("exec cmd failed: %s", cmd);
        }
    }
    
    return;
}

static int init_hotplug_sock() 
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1) {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    (void)setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    
    return hotplug_sock;
}

static int get_monitor_connect_status(const char* path)
{
    FILE *fp = NULL;
    char buf[64] = {0, };
    int status = DISCONNECTED;

    fp = fopen(path, "r");
    if (fp == NULL) {
        return DISCONNECTED;
    }

    if (fgets(buf, 64, fp)) {
        if(!strncmp(buf, "connected", 9)){
            status = CONNECTED;
        } else {
            status = DISCONNECTED;
        }
    }
    
    fclose(fp);

    return status;
}

static int get_monitor_devices(char *device)
{
    strcpy(device, "change@/devices/pci0000:00/0000:00:02.0/drm/card0");
    return 0;
}

void *hotplug_event_monitor(void *data)
{
    char buf[UEVENT_BUFFER_SIZE * 2] = {0,};
    char monitor_name[128] = {0,};
    char vga_status_file[128] = {0,};
    char hdmi_status_file[128] = {0,};
    int hotplug_sock;

    get_monitor_devices(monitor_name);
    snprintf(vga_status_file, 128, "/sys/class/drm/card0-DP-1/status");
    snprintf(hdmi_status_file, 128, "/sys/class/drm/card0-HDMI-A-1/status");

    hotplug_sock = init_hotplug_sock();
    if (hotplug_sock == -1) {
        return NULL;
    }
    
    while (1) {
        memset(buf, 0, UEVENT_BUFFER_SIZE * 2);
        (void)recv(hotplug_sock, &buf, sizeof(buf), 0);
        if (strstr(buf, monitor_name)) {
            if (get_monitor_connect_status(vga_status_file) == CONNECTED
                    || get_monitor_connect_status(hdmi_status_file) == CONNECTED) {
                set_same_resolution();
            }
        }
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    //int pthread_ret = 0;
    //pthread_t pid = -1;
    //int err = 0;

    if (!access("/usr/local/lib/libdevice_manage.so", F_OK)) {
        printf("libdevice_manage.so exists, display_hotplug return\n");
        return -1;
    }

    if (!access("/usr/local/lib/libsysabslayer_linux.so", F_OK)) {
        printf("libsysabslayer_linux.so, display_hotplug return\n");
        return -1;
    }

    set_same_resolution();
    
    //pthread_ret = pthread_create(&pid, NULL, (void *)hotplug_event_monitor, NULL);
    //if(pthread_ret != 0) {
    //     printf("Create hotplug event monitor pthread error\n");
    //      return -1;
    // }
    //(void)pthread_join(pid, NULL);
    
    return 0;
}
