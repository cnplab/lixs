# lixs: LightweIght XenStore

verbose	?= n
debug	?= n


APP	:=
APP	+= $(patsubst %.cc, %, $(shell find app/ -name "*.cc"))

TST	:=
TST	+= $(patsubst %.cc, %, $(shell find test/ -name "*.cc"))

LIB	:=
LIB	+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc"))

INC	:=
INC	+= $(shell find inc/ -name "*.hh")


CFLAGS		+= -Iinc -Wall -g -O3
CXXFLAGS	+= -Iinc -Wall -g -O3 -std=gnu++11
LDFLAGS		+= -lxenctrl -lxenstore

ifeq ($(debug),y)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif


include make.mk

all: $(APP) $(TST)

$(APP): % : %.o $(LIB)
	$(call cxxlink, $^, $@)

$(TST): % : %.o $(LIB)
	$(call cxxlink, $^, $@)

%.o: %.cc $(INC)
	$(call cxxcompile, $<, $@)

%.o: %.c $(INC)
	$(call ccompile, $<, $@)


clean:
	$(call cmd, "CLN", "*.o [ app/  ]", rm -rf, $(patsubst %, %.o, $(APP)))
	$(call cmd, "CLN", "*.o [ test/ ]", rm -rf, $(patsubst %, %.o, $(TST)))
	$(call cmd, "CLN", "*.o [ lib/  ]", rm -rf, $(LIB))

distclean: clean
	$(call cmd, "CLN", "* [ app/  ]" , rm -rf, $(APP))
	$(call cmd, "CLN", "* [ test/ ]", rm -rf, $(TST))


.PHONY: all clean distclean

