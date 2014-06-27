################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lib/hamming.c \
../src/lib/rfm12.c \
../src/lib/twi_master.c \
../src/lib/uart.c 

OBJS += \
./src/lib/hamming.o \
./src/lib/rfm12.o \
./src/lib/twi_master.o \
./src/lib/uart.o 

C_DEPS += \
./src/lib/hamming.d \
./src/lib/rfm12.d \
./src/lib/twi_master.d \
./src/lib/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/%.o: ../src/lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -Wextra -mmcu=atmega64 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


