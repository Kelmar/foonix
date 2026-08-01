cmake_minimum_required(VERSION 3.25)

project(grub)

add_custom_target(grub ALL DEPENDS grub.cfg)

install(
    FILES grub.cfg
    DESTINATION ${SYSROOT}/boot/grub
)