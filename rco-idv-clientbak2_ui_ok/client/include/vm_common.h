#ifndef CLIENT_VM_VM_COMMON_H_
#define CLIENT_VM_VM_COMMON_H_

#include "event_source.h"

class VM_Common:public EventSource
{
public:
	static string execute_command(string cmd);
};


#endif /* CLIENT_VM_VM_COMMON_H_ */
