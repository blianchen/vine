################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Str.c \
../src/base64.c \
../src/exception.c \
../src/hashmap.c \
../src/logger.c \
../src/md5.c \
../src/mem.c \
../src/sha1.c \
../src/strconv.c \
../src/uri.c \
../src/utils.c 

OBJS += \
./src/Str.o \
./src/base64.o \
./src/exception.o \
./src/hashmap.o \
./src/logger.o \
./src/md5.o \
./src/mem.o \
./src/sha1.o \
./src/strconv.o \
./src/uri.o \
./src/utils.o 

C_DEPS += \
./src/Str.d \
./src/base64.d \
./src/exception.d \
./src/hashmap.d \
./src/logger.d \
./src/md5.d \
./src/mem.d \
./src/sha1.d \
./src/strconv.d \
./src/uri.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -I"/home/blc/c/corelib/include" -I"/home/blc/c/corelib/src" -I"/home/blc/c/corelib/libs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


