STM32CUBEEXPANSION_53L1A1 = .

COMPONENT_SRCDIRS := . \
	$(STM32CUBEEXPANSION_53L1A1)/core/src

COMPONENT_ADD_INCLUDEDIRS = \
	$(STM32CUBEEXPANSION_53L1A1)/core/inc \
	$(STM32CUBEEXPANSION_53L1A1)/platform/inc

CFLAGS += -Wno-unused-variable -Wno-maybe-uninitialized -DI2C_HandleTypeDef=int
