#ifndef __DHCP_OPTION_H__
#define __DHCP_OPTION_H__

#include "cJSON.h"
#include <pthread.h>

typedef void (*DhcpOptionCallbackHandler)(const char* pMsg, void* data);

/**
 * Function:    init_dhcp_option_thread
 * Description: create a thread listenning to dhcp-options received from dhcp-server
 *              we only seperate the latest lease and parse the dhcp-option-value from it
 * Input:       netcard_name:   string of netcard_name to get dhcp
 *              handle:         callback when we get dhcp-options
 *              handle_data:    input data of callback handle's parament
 * return:      0   fail in creating the thread
 * 		        >=0 dhcp_option_thread_id
 */
pthread_t init_dhcp_option_thread(const char* netcard_name, DhcpOptionCallbackHandler handle, void* handle_data);

/**
 * Function:    handle_dhcp_option
 * Description: do callback handle if we find dhcp-option-value in the latest lease
 *              we only seperate the latest lease and parse the dhcp-option-value from it
 * Input:       netcard_name:   string of netcard_name to get dhcp
 *              handle:         callback when we get dhcp-options
 *              handle_data:    input data of callback handle's parament
 * return:      0   success
 * 		        -1  fail if error happen
 *              -2  fail if not found dhcp-option-value
 */
int handle_dhcp_option(const char* netcard_name, DhcpOptionCallbackHandler handle, void* handle_data);


#endif//__DHCP_OPTION_H__
