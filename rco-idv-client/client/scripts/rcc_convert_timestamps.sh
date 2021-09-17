#!/bin/bash

standard_time_ms=$(expr $1 / 1000)
standard_time="`date -d @$standard_time_ms +'%F %H:%M:%S'`"
exec /usr/local/bin/factory/first_connect_server_date.sh "$standard_time"
