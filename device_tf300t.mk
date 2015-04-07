# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

$(call inherit-product-if-exists, vendor/asus/tf300t/tf300t-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/asus/tf300t/overlay

# Files needed for boot image
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/ramdisk/init.cardhu.rc:root/init.cardhu.rc \
    $(LOCAL_PATH)/ramdisk/init.tf.rc:root/init.tf.rc \
    $(LOCAL_PATH)/ramdisk/ueventd.cardhu.rc:root/ueventd.cardhu.rc \
    $(LOCAL_PATH)/ramdisk/init.cardhu.usb.rc:root/init.cardhu.usb.rc \
    $(LOCAL_PATH)/ramdisk/init.cardhu.cpu.rc:root/init.cardhu.cpu.rc \
    $(LOCAL_PATH)/ramdisk/fstab.cardhu:root/fstab.cardhu \
    $(LOCAL_PATH)/twrp.fstab:recovery/root/etc/twrp.fstab

# Wifi related files and packages
PRODUCT_PACKAGES += \
       hostapd \
       dhcpcd.conf \
       wpa_supplicant \
       wpa_supplicant.conf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf

# Input device config files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/idc/elan-touchscreen.idc:system/usr/idc/elan-touchscreen.idc \
    $(LOCAL_PATH)/idc/elantech_touchscreen.idc:system/usr/idc/elantech_touchscreen.idc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    $(LOCAL_PATH)/asusdec/org.omnirom.asusdec.xml:system/etc/permissions/org.omnirom.asusdec.xml

# Build characteristics setting 
PRODUCT_CHARACTERISTICS := tablet

# This device have enough room for precise davick
PRODUCT_TAGS += dalvik.gc.type-precise

# Extra packages to build for this device
PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory \
    make_ext4fs \
    fsck.f2fs \
    mkfs.f2fs \
    setup_fs \
    audio.a2dp.default \
    audio.r_submix.default \
    tinymix \
    libaudioutils \
    AutoParts \
    blobpack_tfp \
    wifimacwriter \
    org.omnirom.asusdec \
    libasusdec_jni

# Propertys specific for this device
PRODUCT_PROPERTY_OVERRIDES := \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=15 \
    tf.enable=y \
    ro.opengles.version=131072 \
    persist.sys.usb.config=mtp,adb

# Tegra 3 spacific overrides
PRODUCT_PROPERTY_OVERRIDES += \
    persist.tegra.nvmmlite=1 \
    ro.zygote.disable_gl_preload=1

# Prime specific overrides
PRODUCT_PROPERTY_OVERRIDES += \
    ro.epad.model=TF300T \
    ro.product.model=TF300T

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    camera2.portability.force_api=1

# set SELinux property value
PRODUCT_PROPERTY_OVERRIDES += \
    ro.build.selinux=1

# media files
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
    device/asus/tf300t/media_codecs.xml:system/etc/media_codecs.xml \
    device/asus/tf300t/media_profiles.xml:system/etc/media_profiles.xml \
    device/asus/tf300t/audio_policy.conf:system/etc/audio_policy.conf

# gps config files
PRODUCT_COPY_FILES += \
    device/asus/tf300t/gpsconfig.xml:system/etc/gps/gpsconfig.xml \
    device/asus/tf300t/gps.conf:system/etc/gps.conf \
    device/asus/tf300t/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf \
    device/asus/tf300t/get-gps-lto:system/bin/get-gps-lto

# Inherit tablet dalvik settings
$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)
