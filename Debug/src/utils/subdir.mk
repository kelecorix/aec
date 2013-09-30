################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils/hashmap.c \
../src/utils/threads.c \
../src/utils/utils.c 

OBJS += \
./src/utils/hashmap.o \
./src/utils/threads.o \
./src/utils/utils.o 

C_DEPS += \
./src/utils/hashmap.d \
./src/utils/threads.d \
./src/utils/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/%.o: ../src/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/opt/owfs/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


