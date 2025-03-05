################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../time_slice/Src/task.c \
../time_slice/Src/time_slice.c 

OBJS += \
./time_slice/Src/task.o \
./time_slice/Src/time_slice.o 

C_DEPS += \
./time_slice/Src/task.d \
./time_slice/Src/time_slice.d 


# Each subdirectory must supply rules for building sources it contributes
time_slice/Src/%.o time_slice/Src/%.su time_slice/Src/%.cyclo: ../time_slice/Src/%.c time_slice/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/subhadeep/STM32CubeIDE/workspace_1.17.0/mini_scheduler/time_slice/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-time_slice-2f-Src

clean-time_slice-2f-Src:
	-$(RM) ./time_slice/Src/task.cyclo ./time_slice/Src/task.d ./time_slice/Src/task.o ./time_slice/Src/task.su ./time_slice/Src/time_slice.cyclo ./time_slice/Src/time_slice.d ./time_slice/Src/time_slice.o ./time_slice/Src/time_slice.su

.PHONY: clean-time_slice-2f-Src

