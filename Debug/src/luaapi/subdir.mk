################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/luaapi/luadb.c \
../src/luaapi/luanet.c \
../src/luaapi/luapack.c \
../src/luaapi/luast.c \
../src/luaapi/luautils.c 

OBJS += \
./src/luaapi/luadb.o \
./src/luaapi/luanet.o \
./src/luaapi/luapack.o \
./src/luaapi/luast.o \
./src/luaapi/luautils.o 

C_DEPS += \
./src/luaapi/luadb.d \
./src/luaapi/luanet.d \
./src/luaapi/luapack.d \
./src/luaapi/luast.d \
./src/luaapi/luautils.d 


# Each subdirectory must supply rules for building sources it contributes
src/luaapi/%.o: ../src/luaapi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DMD_HAVE_EPOLL -I"/home/blc/c/vine/include" -I"/home/blc/c/vine/deps/lua/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


