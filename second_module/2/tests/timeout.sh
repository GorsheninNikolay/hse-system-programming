#!/bin/sh
exec 3<>/dev/tcp/127.0.0.1/8080
echo 'sleep 10' >&3
cat <&3
