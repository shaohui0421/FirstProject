#ifndef _APPLICATION_DATA_H
#define _APPLICATION_DATA_H

#include <sstream>
#include <iostream>
#include "common.h"
#include "local_network_data.h"
#include "user_db.h"

#define FLIE_LEN              256

class BasicData
{
public:
    BasicData()
    {
        int ret = 0;
        rc_systeminfo_t system_info;
        VersioninfoDB version_data;
        rc_netifdev_t dev;
        rc_netifdev_t dev2;
        std::stringstream software_stream;
        
        memset(&system_info, 0, sizeof(rc_systeminfo_t ));
        ret = rc_getsysteminfo(&system_info);
        if(ret != 0)
        {
            LOG_ERR("rc_getsysteminfo error, ret = %d", ret);
        }
    
        _basic_info.serial_number   = system_info.serial_number;
        _basic_info.product_id      = system_info.product_id;
        _basic_info.product_name      = system_info.product_name;
        _basic_info.hardware_version= system_info.hardware_version;
        _basic_info.os_version      = system_info.os_version;
        _basic_info.bios_version      = system_info.bios_version;
        _basic_info.cpu             = system_info.cpu;
        _basic_info.memory          = system_info.memory;
        _basic_info.storage         = system_info.storage;

        software_stream << "RCO_V" << version_data.get_version_info().main_version << "." << version_data.get_version_info().minor_version\
            << version_data.get_version_info().extra_first << "." <<version_data.get_version_info().fourth_version << version_data.get_version_info().extra_second;
        software_stream >> _basic_info.software_version;
        
        LOG_DEBUG("the software_version is %s", _basic_info.software_version.c_str());
        strcpy(dev.dev, RCC_DEFAULT_IFNAME);
        ret = rc_getnetifdevinfo(&dev);
        if(ret != 0)
        {
            LOG_ERR("rc_getnetifdevinfo error, ret = %d", ret);
        }
        _basic_info.mac = dev.mac;
        strcpy(dev2.dev, RCC_DEFAULT_IFNAME2);
        ret = rc_getnetifdevinfo(&dev2);
        if(ret != 0)
        {
            LOG_ERR("rc_getnetifdevinfo error, ret = %d", ret);
        }
        _basic_info.mac2 = dev2.mac;
    }

    const BasicInfo& get(){return _basic_info;}

private:
    BasicInfo _basic_info;
};

class RccUsbFliterData:public InitFile
{
public:
    RccUsbFliterData(string write_mode = "wb+", string read_mode = "rb"):InitFile (RCC_USB_FLITER, write_mode, read_mode) {}
    ~RccUsbFliterData(){};
protected:
    virtual int on_wite_file(const void *data) 
    {
        int nwrite = 0;

        if (get_file_fd()) {
            if (data) {
                _usbinfo = (UsbConfigInfo *)data;
                nwrite = fwrite(_usbinfo, 1, sizeof(_usbinfo->head), get_file_fd());
                if (nwrite <= 0) {
                    LOG_ERR("RccUsbFliterData head nwrite: %d, errno: %d, strerror: %s", nwrite, errno, strerror(errno));
                    return nwrite;
                }

                if (_usbinfo->allow_len) {
                    nwrite = fwrite(_usbinfo->allow_data.c_str(), 1, _usbinfo->allow_len, get_file_fd());
                    if (nwrite <= 0) {
                        LOG_ERR("RccUsbFliterData allow nwrite: %d, errno: %d, strerror: %s", nwrite, errno, strerror(errno));
                        return nwrite;
                    }
                }

                if (_usbinfo->unallow_len) {
                    nwrite = fwrite(_usbinfo->unallow_data.c_str(), 1,_usbinfo->unallow_len, get_file_fd());
                    if (nwrite <= 0) {
                        LOG_ERR("RccUsbFliterData unallow nwrite: %d, errno: %d, strerror: %s", nwrite, errno, strerror(errno));
                        return nwrite;
                    }
                }

                fflush(get_file_fd());
                fsync(fileno(get_file_fd()));
            }
        } 
        return nwrite;
    }

    virtual int on_read_file(const void *buf, int len) {
        int nread = 0;

        if (get_file_fd()) {
            if (!buf) {
                return nread;
            }

            _usbinfo = (UsbConfigInfo *)buf;
            nread = fread(_usbinfo, 1, sizeof(UsbConfigInfo), get_file_fd());
        }
        return nread;
    }

private:
    UsbConfigInfo *_usbinfo;
};

class RcdUsbConf:public InitFile
{
    public:
        RcdUsbConf(string write_mode = "w", string read_mode = "r"):InitFile (RCC_USB_CONF, write_mode, read_mode) {}
        ~RcdUsbConf(){};
    protected:
        virtual int on_wite_file(const void *data)
        {
            int nwrite = 0;

            if (get_file_fd()) {
                if (data) {
                    _buf = (char *)data;
                    nwrite = fwrite(_buf, 1, strlen(_buf), get_file_fd());
                    if (nwrite <= 0) {
                        LOG_DEBUG("RcdUsbConf nwrite: %d errno: %d, strerror: %s", nwrite, errno, strerror(errno));
                        return nwrite;
                    }

                    fflush(get_file_fd());
                    fsync(fileno(get_file_fd()));
                }
            }
            return nwrite;
        }

        virtual int on_read_file(const void *buf, int len) {
            int nread = 0;

            if (get_file_fd()) {
               if (!buf) {
                    return nread;
                }

               _buf = (char *)buf;
               nread = fread(_buf, 1, sizeof(_buf) - 1, get_file_fd());
            }
            return nread;
        }
private:
    // point func
    char *_buf;
};

class UsbPolicy:public InitFile
{
    public:
        UsbPolicy(string file, string write_mode = "w", string read_mode = "r"):InitFile (file, write_mode, read_mode) {}
        ~UsbPolicy(){};

    protected:
        virtual int on_wite_file(const void *data)
        {
            int nwrite = 0;

            if (get_file_fd()) {
                if (data) {
                    _buf = (char *)data;
                    nwrite = fwrite(_buf, 1, strlen(_buf), get_file_fd());
                    if (nwrite <= 0) {
                        LOG_DEBUG("UsbPolicy nwrite: %d errno: %d, strerror: %s", nwrite, errno, strerror(errno));
                        return nwrite;
                    }

                    fflush(get_file_fd());
                    fsync(fileno(get_file_fd()));
                }
            }
            return nwrite;
        }

        virtual int on_read_file(const void *buf, int len) {
            int nread = 0;

            if (get_file_fd()) {
               if (!buf) {
                    return nread;
                }

               _buf = (char *)buf;
               nread = fread(_buf, 1, len, get_file_fd());
            }
            return nread;
        }
private:
    // point func
    char *_buf;
};

#endif//_APPLICATION_DATA_H
