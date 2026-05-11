################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bme68x.c \
../Core/Src/bme68x_driver.c \
../Core/Src/cli.c \
../Core/Src/http_server.c \
../Core/Src/lan8720.c \
../Core/Src/main.c \
../Core/Src/sensor_data.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/w25q80.c 

OBJS += \
./Core/Src/bme68x.o \
./Core/Src/bme68x_driver.o \
./Core/Src/cli.o \
./Core/Src/http_server.o \
./Core/Src/lan8720.o \
./Core/Src/main.o \
./Core/Src/sensor_data.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/w25q80.o 

C_DEPS += \
./Core/Src/bme68x.d \
./Core/Src/bme68x_driver.d \
./Core/Src/cli.d \
./Core/Src/http_server.d \
./Core/Src/lan8720.d \
./Core/Src/main.d \
./Core/Src/sensor_data.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/w25q80.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"../Drivers/STM32F4xx_HAL_Driver/Inc" -I"../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"../Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"../Drivers/CMSIS/Include" -I../LWIP/App -I../LWIP/Target -I"../Middlewares/Third_Party/LwIP/src/include" -I"../Middlewares/Third_Party/LwIP/system" -I"../Drivers/BSP/Components/lan8742" -I"../Middlewares/Third_Party/LwIP/src/include/netif/ppp" -I"../Middlewares/Third_Party/LwIP/src/apps/http" -I"../Middlewares/Third_Party/LwIP/src/include/lwip" -I"../Middlewares/Third_Party/LwIP/src/include/lwip/apps" -I"../Middlewares/Third_Party/LwIP/src/include/lwip/priv" -I"../Middlewares/Third_Party/LwIP/src/include/lwip/prot" -I"../Middlewares/Third_Party/LwIP/src/include/netif" -I"../Middlewares/Third_Party/LwIP/src/include/compat/posix" -I"../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa" -I"../Middlewares/Third_Party/LwIP/src/include/compat/posix/net" -I"../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys" -I"../Middlewares/Third_Party/LwIP/src/include/compat/stdc" -I"../Middlewares/Third_Party/LwIP/system/arch" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bme68x.cyclo ./Core/Src/bme68x.d ./Core/Src/bme68x.o ./Core/Src/bme68x.su ./Core/Src/bme68x_driver.cyclo ./Core/Src/bme68x_driver.d ./Core/Src/bme68x_driver.o ./Core/Src/bme68x_driver.su ./Core/Src/cli.cyclo ./Core/Src/cli.d ./Core/Src/cli.o ./Core/Src/cli.su ./Core/Src/http_server.cyclo ./Core/Src/http_server.d ./Core/Src/http_server.o ./Core/Src/http_server.su ./Core/Src/lan8720.cyclo ./Core/Src/lan8720.d ./Core/Src/lan8720.o ./Core/Src/lan8720.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/sensor_data.cyclo ./Core/Src/sensor_data.d ./Core/Src/sensor_data.o ./Core/Src/sensor_data.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/w25q80.cyclo ./Core/Src/w25q80.d ./Core/Src/w25q80.o ./Core/Src/w25q80.su

.PHONY: clean-Core-2f-Src

