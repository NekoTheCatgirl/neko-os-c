#!/usr/bin/env bash
if [ ! -f "$1" ]; then
  qemu-img create -f raw "$1" 512M
fi