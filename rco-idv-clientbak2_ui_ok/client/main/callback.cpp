/*
 * callback.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: zhf
 */

#include "callback.h"

int c_callback(int ret, const void *obj,const void *data){
	//we must be sure the obj can be use, how to do
	CallBack *cb = (CallBack *)obj;
	return cb->callback(ret, cb, (void *)data);
}
