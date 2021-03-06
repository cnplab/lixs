# LiXS: Lightweight XenStore

# Basic configuration
verbose		?= n
config		?= config.mk

ifeq (,$(filter $(MAKECMDGOALS),configure clean distclean))
-include $(config)
endif


# Define binaries and objects
LIXS_APP	:= app/lixs
LIXS_LIB	:=
LIXS_LIB	+= app/lixs_conf.o

CATCH_APP	:= test/run-catch
CATCH_LIB	:= $(patsubst %.cc, %.o, $(shell find test/catch/ -name "*.cc"))

LIBLIXS		:=
LIBLIXS		+= $(patsubst %.c, %.o, $(shell find lib/ -name "*.c"))
LIBLIXS		+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc"))


# Default build and linking flags
ifneq ($(CONFIG_XEN_USR_ROOT),)
CFLAGS		+= -I $(CONFIG_XEN_USR_ROOT)/usr/local/include
CXXFLAGS	+= -I $(CONFIG_XEN_USR_ROOT)/usr/local/include
endif
CFLAGS		+= -Iinc -Wall -MD -MP -g -O3 -std=gnu11
CXXFLAGS	+= -Iinc -Wall -MD -MP -g -O3 -std=gnu++11

ifneq ($(CONFIG_XEN_USR_ROOT),)
LDFLAGS		+= -L $(CONFIG_XEN_USR_ROOT)/usr/local/lib
LDFLAGS		+= -Wl,-rpath-link,$(CONFIG_XEN_USR_ROOT)/usr/local/lib
endif
LDFLAGS		+= -lxenctrl -lxenstore

# Configuration macros
CCFLAGS		+= -DLOGGER_MAX_LEVEL=LOGGER_MAX_LEVEL_$(CONFIG_LOGGER_MAX_LEVEL)
CXXFLAGS	+= -DLOGGER_MAX_LEVEL=LOGGER_MAX_LEVEL_$(CONFIG_LOGGER_MAX_LEVEL)
# On Xen 4.7 the libxc API changed being split into multiple libs. Defining the
# following macros gives access to the old API. Lets not change the used API
# for now, to keep compatibility with older version of Xen.
CFLAGS		+= -DXC_WANT_COMPAT_EVTCHN_API
CFLAGS		+= -DXC_WANT_COMPAT_GNTTAB_API
CFLAGS		+= -DXC_WANT_COMPAT_MAP_FOREIGN_API
CXXFLAGS	+= -DXC_WANT_COMPAT_EVTCHN_API
CXXFLAGS	+= -DXC_WANT_COMPAT_GNTTAB_API
CXXFLAGS	+= -DXC_WANT_COMPAT_MAP_FOREIGN_API


# Build targets
all: $(LIXS_APP)

tests: $(CATCH_APP)

configure: $(config)

install: $(LIXS_APP)
	$(call cmd, "INSTALL", $(LIXS_APP), cp -f, $(LIXS_APP) /usr/local/sbin/lixs)

clean:
	$(call cmd, "CLEAN", "*.o", rm -rf, $(shell find -name "*.o"))
	$(call cmd, "CLEAN", "*.d", rm -rf, $(shell find -name "*.d"))

distclean: clean
	$(call cmd, "CLEAN", $(LIXS_APP) , rm -f, $(LIXS_APP))
	$(call cmd, "CLEAN", $(CATCH_APP), rm -f, $(CATCH_APP))
	$(call cmd, "CLEAN", config.mk, rm -f, config.mk)

.PHONY: all tests configure install clean distclean


# Include default rules
include rules.mk

# Build rules for binaries
$(LIXS_APP): % : %.o $(LIXS_LIB) $(LIBLIXS)
	$(call cxxlink, $^, $@)

$(CATCH_APP): % : %.o $(CATCH_LIB) $(LIBLIXS)
	$(call cxxlink, $^, $@)

# Build rules for configuration
config.mk: config.mk.in
	$(call cmd, "CONFIG", $@, cp, $^ $@)


# Include auto-generated dependency information
-include $(LIXS_APP:%=%.d)
-include $(LIXS_LIB:%.o=%.d)
-include $(CATCH_APP:%=%.d)
-include $(CATCH_LIB:%.o=%.d)
-include $(LIBLIXS:%.o=%.d)
