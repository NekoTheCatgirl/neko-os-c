#!/bin/bash
if [ ! -f "$1" ]; then
    cp /usr/share/qemu/edk2-i386-vars.fd "$1"
fi