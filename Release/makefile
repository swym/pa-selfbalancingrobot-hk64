################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/test/subdir.mk
-include src/lib/subdir.mk
-include src/subdir.mk
-include .metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
pa-einpersonentransportmittel-hk64.lss \

FLASH_IMAGE += \
pa-einpersonentransportmittel-hk64.hex \

EEPROM_IMAGE += \
pa-einpersonentransportmittel-hk64.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: pa-einpersonentransportmittel-hk64.elf secondary-outputs

# Tool invocations
pa-einpersonentransportmittel-hk64.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,pa-einpersonentransportmittel-hk64.map -uvfprintf -mmcu=atmega64 -o "pa-einpersonentransportmittel-hk64.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

pa-einpersonentransportmittel-hk64.lss: pa-einpersonentransportmittel-hk64.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S pa-einpersonentransportmittel-hk64.elf  >"pa-einpersonentransportmittel-hk64.lss"
	@echo 'Finished building: $@'
	@echo ' '

pa-einpersonentransportmittel-hk64.hex: pa-einpersonentransportmittel-hk64.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex pa-einpersonentransportmittel-hk64.elf  "pa-einpersonentransportmittel-hk64.hex"
	@echo 'Finished building: $@'
	@echo ' '

pa-einpersonentransportmittel-hk64.eep: pa-einpersonentransportmittel-hk64.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex pa-einpersonentransportmittel-hk64.elf  "pa-einpersonentransportmittel-hk64.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: pa-einpersonentransportmittel-hk64.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega64 pa-einpersonentransportmittel-hk64.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) pa-einpersonentransportmittel-hk64.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
