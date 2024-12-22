#!/bin/sh
exec 3<>/dev/tcp/127.0.0.1/8080
echo 'ls -l' >&3
cat <&3
