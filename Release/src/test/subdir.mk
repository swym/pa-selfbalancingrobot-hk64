################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/test/test_acceleration.c \
../src/test/test_bma020.c \
../src/test/test_md25.c \
../src/test/test_motor_control.c \
../src/test/test_twi_master.c 

OBJS += \
./src/test/test_acceleration.o \
./src/test/test_bma020.o \
./src/test/test_md25.o \
./src/test/test_motor_control.o \
./src/test/test_twi_master.o 

C_DEPS += \
./src/test/test_acceleration.d \
./src/test/test_bma020.d \
./src/test/test_md25.d \
./src/test/test_motor_control.d \
./src/test/test_twi_master.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -Wextra -mmcu=atmega64 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


