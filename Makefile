# lixs: LightweIght XenStore

verbose	?= n
debug	?= n


APP	:=
APP	+= $(patsubst %.cc, %, $(shell find app/ -name "*.cc"))

TST	:=
TST	+= $(patsubst %.cc, %, $(shell find test/ -name "*.cc"))

LIB	:=
LIB	+= $(patsubst %.c, %.o, $(shell find lib/ -name "*.c" -not -path "lib/app/*"))
LIB	+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc" -not -path "lib/app/*"))

INC	:=
INC	+= $(shell find inc/ -name "*.h")
INC	+= $(shell find inc/ -name "*.hh")

app_lib  =
app_lib += $(patsubst %.c, %.o, $(shell find lib/$1/ -name "*.c" 2>/dev/null))
app_lib += $(patsubst %.cc, %.o, $(shell find lib/$1/ -name "*.cc" 2>/dev/null))


CFLAGS		+= -Iinc -Wall -g -O3 -std=gnu11
CXXFLAGS	+= -Iinc -Wall -g -O3 -std=gnu++11
LDFLAGS		+= -lxenctrl -lxenstore

ifeq ($(debug),y)
CFLAGS		+= -DDEBUG
CXXFLAGS	+= -DDEBUG
endif


include make.mk

all: $(APP) $(TST)

# Because of some make intricacies adding $(call app_lib,%) to the general rule
# dependency list doesn't work. Curiously I managed to get the function to
# receive the argument but it only worked with a $(shell echo $1) not the find
# command. For now we just need to add a specific rule for each of the apps that
# require per app libraries.
app/lixs: $(call app_lib,app/lixs)
$(APP) $(TST): % : %.o $(LIB)
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

