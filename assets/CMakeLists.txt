add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/logo.o
		COMMAND x86_64-elf-objcopy
				-O elf64-x86-64
				-B i386
				-I binary
				logo.raw
				${CMAKE_BINARY_DIR}/logo.o
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/assets
		DEPENDS ${CMAKE_SOURCE_DIR}/assets/logo.raw
		COMMENT "Embedding logo"
)

add_custom_target(generate_logo DEPENDS ${CMAKE_BINARY_DIR}/logo.o)