################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/luaapi/luast.c 

OBJS += \
./src/luaapi/luast.o 

C_DEPS += \
./src/luaapi/luast.d 


# Each subdirectory must supply rules for building sources it contributes
src/luaapi/%.o: ../src/luaapi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -I"/home/blc/c/score/include" -I"/home/blc/c/score/libs" -I"/home/blc/c/score/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


