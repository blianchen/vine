################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/base64.c \
../src/exception.c \
../src/hashmap.c \
../src/logger.c \
../src/md5.c \
../src/mem.c \
../src/sha1.c \
../src/str.c \
../src/strbuffer.c \
../src/strconv.c \
../src/timeutil.c \
../src/uri.c \
../src/utils.c \
../src/vector.c 

OBJS += \
./src/base64.o \
./src/exception.o \
./src/hashmap.o \
./src/logger.o \
./src/md5.o \
./src/mem.o \
./src/sha1.o \
./src/str.o \
./src/strbuffer.o \
./src/strconv.o \
./src/timeutil.o \
./src/uri.o \
./src/utils.o \
./src/vector.o 

C_DEPS += \
./src/base64.d \
./src/exception.d \
./src/hashmap.d \
./src/logger.d \
./src/md5.d \
./src/mem.d \
./src/sha1.d \
./src/str.d \
./src/strbuffer.d \
./src/strconv.d \
./src/timeutil.d \
./src/uri.d \
./src/utils.d \
./src/vector.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DMD_HAVE_EPOLL -I"/home/blc/c/score/include" -I"/home/blc/c/score/libs" -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


