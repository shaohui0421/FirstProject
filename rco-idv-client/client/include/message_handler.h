#ifndef _MESSAGE_HANDLER_H
#define _MESSAGE_HANDLER_H

#include <map>
#include <string>
#include "rc/rc_public.h"
using std::string;

template <class HandlerClass>
class MessageHandler
{
    typedef void (HandlerClass::*Handler)(string data);
    typedef std::map<int, Handler> Handlers;
public:
    MessageHandler(HandlerClass& obj) :_obj (obj) {}
    virtual ~MessageHandler() {}

protected:
    void set_handler(int id, Handler handle)
    {
        _handlers[id] = handle;
    }

    //handle a readable fd,return false if it comes into an error while receiving
    bool handle_message(int fd)
    {
        int id;
        string buf;
        if(false == recv_message(fd, id, buf))
        {
            return false;
        }
        else
        {
            if(_handlers.count(id) > 0)
            {
                (_obj.*_handlers[id])(buf);
                return true;
            }
            else
            {
                LOG_ERR("miss handle a unset handler id = %d", id);
                return false;
            }
        }
    }

    virtual bool recv_message(int fd, int& id, string& buf){return false;}
private:
    HandlerClass& _obj;
    Handlers _handlers;

};


#endif//_MESSAGE_HANDLER_H

