################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ui/keyboard.c \
../src/ui/lcd.c \
../src/ui/menu.c \
../src/ui/ui.c 

OBJS += \
./src/ui/keyboard.o \
./src/ui/lcd.o \
./src/ui/menu.o \
./src/ui/ui.o 

C_DEPS += \
./src/ui/keyboard.d \
./src/ui/lcd.d \
./src/ui/menu.d \
./src/ui/ui.d 


# Each subdirectory must supply rules for building sources it contributes
src/ui/%.o: ../src/ui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/opt/owfs/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


