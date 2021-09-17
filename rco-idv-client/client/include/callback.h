/*
 * callback.h
 *
 *  Created on: Feb 17, 2017
 *      Author: zhf
 */
#ifndef __CALLBACK_H__
#define __CALLBACK_H__

class CallBack {
public:
	CallBack(){};
	~CallBack(){};
	virtual int callback(int ret, void *obj, void *data) = 0;
};

int c_callback(int ret, const void *obj,const void *data);
#endif
