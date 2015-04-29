################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/st/subdir.mk
-include src/luaapi/subdir.mk
-include src/db/redis/subdir.mk
-include src/db/postgresql/subdir.mk
-include src/db/subdir.mk
-include src/subdir.mk
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
all: vine

# Tool invocations
vine: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/blc/c/vine/deps/lua/src" -lpthread -lm -ldl -lhiredis -lpq -lcheck -o "vine" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) vine
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
