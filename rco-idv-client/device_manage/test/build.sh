#!/bin/bash

#g++ -I ../include/ -g -Wall -std=c++11 -o test_audio test_audio.cpp -lrc_base -lpthread -ldevice_manage -L/usr/local/lib/ -L../../build/device_manage/libdevice_manage.so
g++ -I ../include/ -g -Wall -std=c++11 -o test_video test_video.cpp -lrc_base -lpthread -ldevice_manage -L/usr/local/lib/ -L../../build/device_manage/libdevice_manage.so
g++ -I ../include/ -g -Wall -std=c++11 -o test_video_1 test_video_1.cpp -lrc_base -lpthread -ldevice_manage -L/usr/local/lib/ -L../../build/device_manage/libdevice_manage.so
# hotplug
#g++ -I ../include/ -g -Wall -std=c++11 -o test_hpd test_hpd.cpp -lrc_base -lpthread -ldevice_manage -L/usr/local/lib/ -L../../build/device_manage/libdevice_manage.so

