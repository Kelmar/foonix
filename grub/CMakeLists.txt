cmake_minimum_required(VERSION 3.20)

project(grub)

add_custom_target(grub ALL DEPENDS grub.cfg)

install(
    FILES grub.cfg
    DESTINATION ${SYSROOT}/boot/grub
)