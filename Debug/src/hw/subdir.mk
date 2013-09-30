################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hw/ac.c \
../src/hw/i2c.c \
../src/hw/ow.c \
../src/hw/site.c \
../src/hw/throttle.c \
../src/hw/vent.c 

OBJS += \
./src/hw/ac.o \
./src/hw/i2c.o \
./src/hw/ow.o \
./src/hw/site.o \
./src/hw/throttle.o \
./src/hw/vent.o 

C_DEPS += \
./src/hw/ac.d \
./src/hw/i2c.d \
./src/hw/ow.d \
./src/hw/site.d \
./src/hw/throttle.d \
./src/hw/vent.d 


# Each subdirectory must supply rules for building sources it contributes
src/hw/%.o: ../src/hw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


