################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/st/event.c \
../src/st/io.c \
../src/st/key.c \
../src/st/node.c \
../src/st/sched.c \
../src/st/stk.c \
../src/st/sync.c \
../src/st/threadmsg.c 

S_UPPER_SRCS += \
../src/st/md.S 

OBJS += \
./src/st/event.o \
./src/st/io.o \
./src/st/key.o \
./src/st/md.o \
./src/st/node.o \
./src/st/sched.o \
./src/st/stk.o \
./src/st/sync.o \
./src/st/threadmsg.o 

C_DEPS += \
./src/st/event.d \
./src/st/io.d \
./src/st/key.d \
./src/st/node.d \
./src/st/sched.d \
./src/st/stk.d \
./src/st/sync.d \
./src/st/threadmsg.d 


# Each subdirectory must supply rules for building sources it contributes
src/st/%.o: ../src/st/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DMD_HAVE_EPOLL -Dlua_c -I"/home/blc/c/score/include" -I"/home/blc/c/score/libs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/st/%.o: ../src/st/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


