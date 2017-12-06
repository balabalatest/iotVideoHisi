################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include misc/subdir.mk
-include iotRtmp/librtmp/subdir.mk
-include iotRtmp/subdir.mk
-include iotMqtt/libmqtt/subdir.mk
-include iotMqtt/client/subdir.mk
-include iotMqtt/subdir.mk
-include hi3518/common/subdir.mk
-include cJSON/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: iotVideoHisi

# Tool invocations
iotVideoHisi: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-hisiv300-linux-uclibcgnueabi-gcc  -o "iotVideoHisi" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) iotVideoHisi
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
