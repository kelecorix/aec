################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/log/file_logger.c \
../src/log/logger.c \
../src/log/net_logger.c 

OBJS += \
./src/log/file_logger.o \
./src/log/logger.o \
./src/log/net_logger.o 

C_DEPS += \
./src/log/file_logger.d \
./src/log/logger.d \
./src/log/net_logger.d 


# Each subdirectory must supply rules for building sources it contributes
src/log/%.o: ../src/log/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/opt/owfs/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


