################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config/config.c \
../src/config/i2c.c 

OBJS += \
./src/config/config.o \
./src/config/i2c.o 

C_DEPS += \
./src/config/config.d \
./src/config/i2c.d 


# Each subdirectory must supply rules for building sources it contributes
src/config/%.o: ../src/config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/opt/owfs/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


