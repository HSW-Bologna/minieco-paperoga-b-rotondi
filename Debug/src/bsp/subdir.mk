################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp/adc.c \
../src/bsp/coin_reader.c \
../src/bsp/digin.c \
../src/bsp/heartbit.c \
../src/bsp/power_off.c \
../src/bsp/pwm.c \
../src/bsp/relay.c \
../src/bsp/rs232.c \
../src/bsp/spi.c \
../src/bsp/system.c \
../src/bsp/temperature_humidity_probe.c 

C_DEPS += \
./src/bsp/adc.d \
./src/bsp/coin_reader.d \
./src/bsp/digin.d \
./src/bsp/heartbit.d \
./src/bsp/power_off.d \
./src/bsp/pwm.d \
./src/bsp/relay.d \
./src/bsp/rs232.d \
./src/bsp/spi.d \
./src/bsp/system.d \
./src/bsp/temperature_humidity_probe.d 

OBJS += \
./src/bsp/adc.o \
./src/bsp/coin_reader.o \
./src/bsp/digin.o \
./src/bsp/heartbit.o \
./src/bsp/power_off.o \
./src/bsp/pwm.o \
./src/bsp/relay.o \
./src/bsp/rs232.o \
./src/bsp/spi.o \
./src/bsp/system.o \
./src/bsp/temperature_humidity_probe.o 

SREC += \
minieco_paperoga_b_rotondi.srec 

MAP += \
minieco_paperoga_b_rotondi.map 


# Each subdirectory must supply rules for building sources it contributes
src/bsp/%.o: ../src/bsp/%.c
	$(file > $@.in,-mcpu=cortex-m23 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM23 -D_RA_ORDINAL=1 -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-debounce/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-stopwatch/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-state/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-watcher/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/liblightmodbus/include" -I"." -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc/api" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc/instances" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_gen" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_cfg/fsp_cfg/bsp" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_cfg/fsp_cfg" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"/minieco_paperoga_b_rotondi/libs/c-state/src" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

