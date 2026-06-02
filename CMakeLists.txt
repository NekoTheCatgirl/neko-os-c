cmake_minimum_required(VERSION 4.0)
project(neko-os LANGUAGES C CXX ASM_NASM)

set(CMAKE_SKIP_INSTALL_RULES YES)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY YES)

set(CMAKE_TOOLCHAIN_FILE toolchain-x86_64.cmake FORCE)

set(KERNEL_C_FLAGS
    -ffreestanding
    -fno-stack-protector
    -fno-pic
    -fno-pie
    -fno-PIE
    -mno-red-zone
    -mno-mmx -mno-sse -mno-sse2   # No FPU/SIMD in kernel
    -mcmodel=kernel
    -Wall -Wextra
    -O3
)

set(KERNEL_LINK_FLAGS
    -ffreestanding
    -nostdlib
    -z max-page-size=0x1000
)

add_subdirectory(assets)
add_subdirectory(boot)
add_subdirectory(drivers)
add_subdirectory(fonts)
add_subdirectory(kernel)
add_subdirectory(lib)

add_executable(neko_os.elf
               $<TARGET_OBJECTS:boot_objs>
               $<TARGET_OBJECTS:lib_objs>
               $<TARGET_OBJECTS:drivers_objs>
               $<TARGET_OBJECTS:drivers_asm_objs>
               $<TARGET_OBJECTS:kernel_objs>
               ${CMAKE_BINARY_DIR}/logo.o
               ${CMAKE_BINARY_DIR}/font.o
)

set_target_properties(neko_os.elf PROPERTIES
                      LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld
)

target_link_options(neko_os.elf PRIVATE
                    ${KERNEL_LINK_FLAGS}
                    -T ${CMAKE_SOURCE_DIR}/linker.ld
)

add_dependencies(neko_os.elf generate_logo generate_font)

add_custom_command(TARGET neko_os.elf POST_BUILD
                   COMMAND x86_64-elf-objcopy -O binary neko_os.elf neko_os.bin
                   COMMENT "Creating flat binary"
)

add_custom_target(iso
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
                  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:neko_os.elf> ${CMAKE_BINARY_DIR}/iso/boot/
                  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/iso/boot/grub/grub.cfg ${CMAKE_BINARY_DIR}/iso/boot/grub/
                  COMMAND grub-mkrescue --output=${CMAKE_BINARY_DIR}/neko_os.iso ${CMAKE_BINARY_DIR}/iso
                  DEPENDS neko_os.elf
                  COMMENT "Building ISO image"
)

add_custom_target(run
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/disk
                  COMMAND ${CMAKE_SOURCE_DIR}/cmake/create_disk.sh ${CMAKE_BINARY_DIR}/disk/disk.img
                  COMMAND ${CMAKE_SOURCE_DIR}/cmake/create_efivars.sh ${CMAKE_BINARY_DIR}/efivars.fd
                  COMMAND qemu-system-x86_64
	                            -machine q35
	                            -drive if=pflash,format=raw,readonly=on,file=/usr/share/qemu/edk2-x86_64-code.fd
	                            -drive if=pflash,format=raw,file=${CMAKE_BINARY_DIR}/efivars.fd
	                            -vga std
                                -cdrom ${CMAKE_BINARY_DIR}/neko_os.iso
                                -drive file=${CMAKE_BINARY_DIR}/disk/disk.img,format=raw,index=0,media=disk
                                -serial stdio
                                -m 2G
                                -smp 4
                                -enable-kvm
                                -cpu host
                                -no-reboot
                                -no-shutdown
                  DEPENDS iso
                  COMMENT "Booting neko_os in QEMU"
)

add_custom_target(clean-disk
                  COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/disk/disk.img
                  COMMENT "Wiping virtual disk"
)