add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/font.o
		COMMAND x86_64-elf-objcopy
				-O elf64-x86-64
				-B i386
				-I binary
				ter-v16n.psf
				${CMAKE_BINARY_DIR}/font.o
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ter-v16n.psf
		COMMENT "Embedding font"
)

add_custom_target(generate_font DEPENDS ${CMAKE_BINARY_DIR}/font.o)