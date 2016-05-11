# LiXS: Lightweight XenStore

# Basic configuration
verbose	?= n
debug	?= n


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
CFLAGS		+= -Iinc -Wall -MD -MP -g -O3 -std=gnu11
CXXFLAGS	+= -Iinc -Wall -MD -MP -g -O3 -std=gnu++11
LDFLAGS		+= -lxenctrl -lxenstore
ifeq ($(debug),y)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif


# Build targets
all: $(LIXS_APP)

tests: $(CATCH_APP)

install: $(LIXS_APP)
	$(call cmd, "INSTALL", $(LIXS_APP), cp -f, $(LIXS_APP) /usr/local/sbin/lixs)

clean:
	$(call cmd, "CLEAN", "*.o", rm -rf, $(shell find -name "*.o"))
	$(call cmd, "CLEAN", "*.d", rm -rf, $(shell find -name "*.d"))

distclean: clean
	$(call cmd, "CLEAN", $(LIXS_APP) , rm -f, $(LIXS_APP))
	$(call cmd, "CLEAN", $(CATCH_APP), rm -f, $(CATCH_APP))

.PHONY: all tests install clean distclean


# Include default rules
include rules.mk

# Build rules for binaries
$(LIXS_APP): % : %.o $(LIXS_LIB) $(LIBLIXS)
	$(call cxxlink, $^, $@)

$(CATCH_APP): % : %.o $(CATCH_LIB) $(LIBLIXS)
	$(call cxxlink, $^, $@)


# Include auto-generated dependency information
-include $(LIXS_APP:%=%.d)
-include $(LIXS_LIB:%.o=%.d)
-include $(CATCH_APP:%=%.d)
-include $(CATCH_LIB:%.o=%.d)
-include $(LIB:%.o=%.d)
