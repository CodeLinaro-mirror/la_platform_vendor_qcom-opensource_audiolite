#!/bin/bash
#SPDX-License-Identifier: GPL-2.0-only
#Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.

BUILD_PATH=$(dirname "$0")

# execute the script under dtb base dir
ln -s $(pwd) ${BUILD_PATH}/safelinux-system-cfg

cd ${BUILD_PATH}
make RPM_BUILD=true all
cd -

cp ${BUILD_PATH}/*.dtbo oot
cp ${BUILD_PATH}/*.dts oot
