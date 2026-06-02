add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/font.o
		COMMAND x86_64-elf-ld
			-r
			-b binary
			${CMAKE_CURRENT_SOURCE_DIR}/ter-v16n.psf
			${CMAKE_BINARY_DIR}/font.o
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ter-v16n.psf
)