################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/acceleration_t.c \
../src/bma020.c \
../src/controller.c \
../src/main.c \
../src/md25.c \
../src/motor_control.c \
../src/moving_average.c \
../src/timer.c 

OBJS += \
./src/acceleration_t.o \
./src/bma020.o \
./src/controller.o \
./src/main.o \
./src/md25.o \
./src/motor_control.o \
./src/moving_average.o \
./src/timer.o 

C_DEPS += \
./src/acceleration_t.d \
./src/bma020.d \
./src/controller.d \
./src/main.d \
./src/md25.d \
./src/motor_control.d \
./src/moving_average.d \
./src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -Wextra -mmcu=atmega64 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


