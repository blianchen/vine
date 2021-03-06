################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/base64.c \
../src/cJSON.c \
../src/exception.c \
../src/ffid.c \
../src/hashmap.c \
../src/http_parser.c \
../src/intmap.c \
../src/logger.c \
../src/md5.c \
../src/mem.c \
../src/nets.c \
../src/sha1.c \
../src/str.c \
../src/strbuffer.c \
../src/strconv.c \
../src/timeutil.c \
../src/uri.c \
../src/utils.c \
../src/vector.c \
../src/vine.c 

OBJS += \
./src/base64.o \
./src/cJSON.o \
./src/exception.o \
./src/ffid.o \
./src/hashmap.o \
./src/http_parser.o \
./src/intmap.o \
./src/logger.o \
./src/md5.o \
./src/mem.o \
./src/nets.o \
./src/sha1.o \
./src/str.o \
./src/strbuffer.o \
./src/strconv.o \
./src/timeutil.o \
./src/uri.o \
./src/utils.o \
./src/vector.o \
./src/vine.o 

C_DEPS += \
./src/base64.d \
./src/cJSON.d \
./src/exception.d \
./src/ffid.d \
./src/hashmap.d \
./src/http_parser.d \
./src/intmap.d \
./src/logger.d \
./src/md5.d \
./src/mem.d \
./src/nets.d \
./src/sha1.d \
./src/str.d \
./src/strbuffer.d \
./src/strconv.d \
./src/timeutil.d \
./src/uri.d \
./src/utils.d \
./src/vector.d \
./src/vine.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DHAVE_LIBPQ -DHAVE_HIREDIS -DMD_HAVE_EPOLL -I"/home/blc/c/vine/include" -I"/home/blc/c/vine/deps/lua/src" -I"/home/blc/c/vine/deps/libbson/src/bson" -include"/home/blc/c/vine/deps/lua/src/lua.h" -include"/home/blc/c/vine/deps/lua/src/lapi.h" -include"/home/blc/c/vine/deps/lua/src/lauxlib.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


