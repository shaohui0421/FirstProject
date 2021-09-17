#include "dhcp_option.h"
#include <sys/inotify.h>  
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef  IDV_CLIENT
#include "rc/rc_log.h"
#define DHCP_OPTION_LOG(format, args...) LOG_INFO(format, ##args)
#else
#ifdef  VDI_CLIENT
static char log_buf[512];
#define DHCP_OPTION_LOG(format, args...) do { sprintf(log_buf, format, ## args); \
        write_log(log_buf, 777); } while(0)

#else
#define DHCP_OPTION_LOG(str, args...) printf(str, ## args)
#endif
#endif


typedef struct DhcpOptionThreadParament
{
    char netcard_name[64];
    int lock;
    DhcpOptionCallbackHandler handle;
    void* handle_data;
} DhcpOptionThreadParament;

static void* dhcp_option_listening_thread(void* data);

pthread_t init_dhcp_option_thread(const char* netcard_name, DhcpOptionCallbackHandler handle, void* handle_data)
{
    pthread_t thread_id;
    if(netcard_name == NULL)
    {
        DHCP_OPTION_LOG("init_dhcp_option_thread netcard_name in NULL, so return \n");
        return 0;
    }
    
    DhcpOptionThreadParament buf;
    strcpy(buf.netcard_name, netcard_name);
    buf.lock = 1;
    buf.handle = handle;
    buf.handle_data = handle_data;
    
    pthread_create(&thread_id, NULL, dhcp_option_listening_thread, &buf);

    while(buf.lock)
    {
        usleep(100);
    }
    return thread_id;
}

//you should guarantee s and len is correct
static int __hex_to_int(const char* s, int len)
{
    int i = 0, m = 0, temp=0, n = 0;
    m = len;
    for(i = 0; i < m; i++)
    {
        if(s[i] >= 'A' && s[i] <= 'F')
            n = s[i] - 'A' + 10;
        else if(s[i] >= 'a' &&s [i] <= 'f')
            n = s[i] - 'a' + 10;
        else
            n = s[i] - '0';

        temp = temp * 16 + n;
    }
    return temp;
}

//always success, you should guarantee output space is enough
static void __get_server_ip(const char* input, char* output)
{
    //input is "ac:15:6f:d8" for example
    const char* tmp_head = input;
    const char* tmp_tail;
    int ip[4];
    int i = 0;

    for(i = 0; i < 4; i++)
    {
        tmp_tail = strchr(tmp_head, ((i == 3) ? ';' : ':'));
        if(tmp_tail ==NULL)
        {
            *output = 0;
            return;
        }
        ip[i] = __hex_to_int(tmp_head, tmp_tail-tmp_head);

        tmp_head = tmp_tail + 1;
    }
    
    sprintf(output, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return;
}

int handle_dhcp_option(const char* netcard_name, DhcpOptionCallbackHandler handle, void* handle_data)
{

#define __LINE_BUF_LENGTH       1024
#define __LEASE_STRING          "lease {"
#define __ENCAPSULATED_STRING   "vendor-encapsulated-options"

    FILE* fp;
    char line_buf[__LINE_BUF_LENGTH];
    char* buf;
    char encapsulated_buf[__LINE_BUF_LENGTH];
    char path_buf[128];
    char server_ip[32];
    memset(encapsulated_buf, 0, __LINE_BUF_LENGTH);
    
    if(netcard_name == NULL)
    {
        DHCP_OPTION_LOG("handle_dhcp_option netcard_name in NULL, so return \n");
        return -1;
    }
    sprintf(path_buf, "/var/lib/dhcp/dhclient.%s.leases", netcard_name);
    fp = fopen(path_buf, "r");
    if (fp == NULL)
    {
        DHCP_OPTION_LOG("open file sinks failed errno = %d\n", errno);
        return -1;
    }

    //analyze the file
    while((fgets(line_buf, __LINE_BUF_LENGTH, fp))!= NULL)
    {
        //get a new lease
        if(strstr(line_buf, __LEASE_STRING) != NULL)
        {
            //clear encapsulated buf and server_ip for a new lease
            memset(encapsulated_buf, 0, __LINE_BUF_LENGTH);
            memset(server_ip, 0, sizeof(server_ip));
        }
        
        //get a new encapsulated-option
        if((buf = strstr(line_buf, __ENCAPSULATED_STRING)) != NULL)
        {
            strcpy(encapsulated_buf, buf + strlen(__ENCAPSULATED_STRING) + strlen(" "));

            __get_server_ip(encapsulated_buf, server_ip);
        }
    }

    if(strlen(server_ip) == 0)
    {
        fclose(fp);
        return -2;
    }
    else
    {
        DHCP_OPTION_LOG("get server_ip %s \n", server_ip);
        handle(server_ip, handle_data);
        fclose(fp);
        return 0;
    }

#undef __LINE_BUF_LENGTH
#undef __LEASE_STRING
#undef __ENCAPSULATED_STRING

}


static void* dhcp_option_listening_thread(void* data)
{

#define __INOTIFY_BUF_LENGTH 1024

    pthread_detach(pthread_self());
    int inotify_fd = 0;
    int watch_fd = 0;
    DhcpOptionThreadParament parament;
    char path_buf[128];
    //struct inotify_event *event;
    //char *p;
    ssize_t read_size;
    char read_buf[__INOTIFY_BUF_LENGTH];
    memcpy(&parament, data, sizeof(DhcpOptionThreadParament));
    ((DhcpOptionThreadParament*)data)->lock = 0;
    
    //init inotify
    sprintf(path_buf, "/var/lib/dhcp/dhclient.%s.leases", parament.netcard_name);
    inotify_fd = inotify_init(); 
    if(inotify_fd == -1)
    {
        DHCP_OPTION_LOG("inotify_init fail, errno = %d\n", errno);
        return NULL;
    }
    watch_fd = inotify_add_watch(inotify_fd, path_buf, IN_MODIFY);
    if(watch_fd == -1)
    {
        DHCP_OPTION_LOG("inotify_add_watch fail, errno = %d\n", errno);
        close(inotify_fd);
        return NULL;
    }

    //listening /var/lib/dhcp/dhclient.leases
    while(1)
    {
        read_size = read(inotify_fd, read_buf, __INOTIFY_BUF_LENGTH);  
        if(read_size == -1)  
        {  
            DHCP_OPTION_LOG("read error, errno = %d\n", errno);  
        }  
  
        DHCP_OPTION_LOG("Read %ld bytes from inotify fd\n", read_size);  
        handle_dhcp_option(parament.netcard_name, parament.handle, parament.handle_data);
        /*
        //mask these code for we only listenning one fd
        for(p = read_buf; p < read_buf+read_size; )  
        {  
            event = (struct inotify_event *)p;  
            DHCP_OPTION_LOG("a new operation\n");
            p += sizeof(struct inotify_event) + event->len;  
        }  
        */
    }
    
#undef __INOTIFY_BUF_LENGTH

    close(inotify_fd);
}

/*
static void test(cJSON* pMsg, void* data)
{
    return;
}
int main(int argc, char* argv[])
{
    init_dhcp_option_thread(argv[1], test, NULL);
    while(1)
        sleep(100);
    return 0;
}
*/
