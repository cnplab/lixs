# LiXS: Lightweight XenStore

verbose	?= n
debug	?= n

LIXS_APP	:= app/lixs
LIXS_LIB	:=
LIXS_LIB	+= app/lixs_conf.o

CATCH_APP	:= test/run-catch
CATCH_LIB	:= $(patsubst %.cc, %.o, $(shell find test/catch/ -name "*.cc"))

LIB	:=
LIB	+= $(patsubst %.c, %.o, $(shell find lib/ -name "*.c"))
LIB	+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc"))


CFLAGS		+= -Iinc -Wall -MD -MP -g -O3 -std=gnu11
CXXFLAGS	+= -Iinc -Wall -MD -MP -g -O3 -std=gnu++11
LDFLAGS		+= -lxenctrl -lxenstore

ifeq ($(debug),y)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif


include make.mk


all: $(LIXS_APP)

tests: $(CATCH_APP)

install: $(LIXS_APP)
	$(call cmd, "INSTALL", $(LIXS_APP), cp -f, $(LIXS_APP) /usr/local/sbin/lixs)


$(LIXS_APP): % : %.o $(LIXS_LIB) $(LIB)
	$(call cxxlink, $^, $@)

$(CATCH_APP): % : %.o $(CATCH_LIB) $(LIB)
	$(call cxxlink, $^, $@)


clean:
	$(call cmd, "CLN", "*.o", rm -rf, $(shell find -name "*.o"))
	$(call cmd, "CLN", "*.d", rm -rf, $(shell find -name "*.d"))

distclean: clean
	$(call cmd, "CLN", "app/" , rm -rf, $(LIXS_APP))
	$(call cmd, "CLN", "test/", rm -rf, $(CATCH_APP))


.PHONY: all tests install clean distclean

-include $(LIXS_APP:%=%.d)
-include $(LIXS_LIB:%.o=%.d)
-include $(CATCH_APP:%=%.d)
-include $(CATCH_LIB:%.o=%.d)
-include $(LIB:%.o=%.d)
