################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp/heartbit.c \
../src/bsp/system.c 

C_DEPS += \
./src/bsp/heartbit.d \
./src/bsp/system.d 

OBJS += \
./src/bsp/heartbit.o \
./src/bsp/system.o 

SREC += \
minieco_paperoga_b_rotondi.srec 

MAP += \
minieco_paperoga_b_rotondi.map 


# Each subdirectory must supply rules for building sources it contributes
src/bsp/%.o: ../src/bsp/%.c
	$(file > $@.in,-mcpu=cortex-m23 -mthumb -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM23 -D_RA_ORDINAL=1 -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/src" -I"." -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra/fsp/inc" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra/fsp/inc/api" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra/fsp/inc/instances" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra/arm/CMSIS_5/CMSIS/Core/Include" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra_gen" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra_cfg/fsp_cfg/bsp" -I"C:/Users/Maldus/Documents/Projects/minieco_paperoga_b_rotondi/ra_cfg/fsp_cfg" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

