################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/base64_test.c \
../test/core_test.c \
../test/corelib.c \
../test/crypto_test.c \
../test/exception_test.c \
../test/hashmap_test.c \
../test/md5_test.c \
../test/sha1_test.c \
../test/uri_test.c 

OBJS += \
./test/base64_test.o \
./test/core_test.o \
./test/corelib.o \
./test/crypto_test.o \
./test/exception_test.o \
./test/hashmap_test.o \
./test/md5_test.o \
./test/sha1_test.o \
./test/uri_test.o 

C_DEPS += \
./test/base64_test.d \
./test/core_test.d \
./test/corelib.d \
./test/crypto_test.d \
./test/exception_test.d \
./test/hashmap_test.d \
./test/md5_test.d \
./test/sha1_test.d \
./test/uri_test.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DMD_HAVE_EPOLL -I"/home/blc/c/score/include" -I"/home/blc/c/score/libs" -I"/home/blc/c/score/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


