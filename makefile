
# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

# Your platform. See PLATS for possible values.
PLAT= none


# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========
#PLATS= aix ansi bsd freebsd generic linux macosx mingw posix solaris
PLATS := aix freebsd linux solaris

all:	$(PLAT)

$(PLATS) clean echo:
	cd deps && $(MAKE) TARG=$@
	cd src && $(MAKE) $@


test:
	cd test && $(MAKE)

none:
	@echo "Please do"
	@echo "   make PLATFORM"
	@echo "where PLATFORM is one of these:"
	@echo "   $(PLATS)"

.PHONY: all $(PLATS) clean test none echo

