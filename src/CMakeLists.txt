idf_component_register(
    SRCS
		main.cpp
		power.cpp
		thread.cpp

    REQUIRES
		driver
		esp_adc
		esp_timer
		freertos
		pthread
)

target_compile_options(
	${COMPONENT_LIB}
	PRIVATE
		-Wall
		-Wextra
		-Wshadow
		-Werror
)
