RM := rm -rf

EXCLUDE_FILES := libs/lua.c libs/luac.c

DIRS = $(shell find src -maxdepth 3 -type d)
DIRS += $(shell find libs -maxdepth 3 -type d)

SRCS := $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c $(dir)/*.S))
SRCS := $(filter-out $(EXCLUDE_FILES), $(SRCS))

OBJS = $(patsubst %.c, %.o, $(SRCS))
#OBJS += $(patsubst %.S,%.o,$(SRCS)) 

test:
	@echo $(SRCS)
.PHONY:test

# All Target
#all: test
#
# Tool invocations
vine: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -lpthread -lm -ldl -lhiredis -lpq -lcheck -o "vine" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DLINUX -DDEBUG -DMD_HAVE_EPOLL -Dlua_c -I"/home/blc/c/score/include" -I"/home/blc/c/score/libs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: %.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) vine
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: