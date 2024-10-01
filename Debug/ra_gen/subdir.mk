################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra_gen/common_data.c \
../ra_gen/hal_data.c \
../ra_gen/main.c \
../ra_gen/pin_data.c \
../ra_gen/vector_data.c 

C_DEPS += \
./ra_gen/common_data.d \
./ra_gen/hal_data.d \
./ra_gen/main.d \
./ra_gen/pin_data.d \
./ra_gen/vector_data.d 

OBJS += \
./ra_gen/common_data.o \
./ra_gen/hal_data.o \
./ra_gen/main.o \
./ra_gen/pin_data.o \
./ra_gen/vector_data.o 

SREC += \
minieco_paperoga_b_rotondi.srec 

MAP += \
minieco_paperoga_b_rotondi.map 


# Each subdirectory must supply rules for building sources it contributes
ra_gen/%.o: ../ra_gen/%.c
	$(file > $@.in,-mcpu=cortex-m23 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM23 -D_RA_ORDINAL=1 -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-debounce/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-stopwatch/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-state/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/c-watcher/src" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/libs/liblightmodbus/include" -I"." -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc/api" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/fsp/inc/instances" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_gen" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_cfg/fsp_cfg/bsp" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra_cfg/fsp_cfg" -I"/home/maldus/Projects/Rotondi/minieco-paperoga-b-rotondi/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"/minieco_paperoga_b_rotondi/libs/c-state/src" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

