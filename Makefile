# lixs: LightweIght XenStore

APP	:=
APP	+= $(patsubst %.cc, %, $(shell find app/ -name "*.cc"))

TST	:=
TST	+= $(patsubst %.cc, %, $(shell find test/ -name "*.cc"))

LIB	:=
LIB	+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc"))

INC	:=
INC	+= $(shell find inc/ -name "*.hh")


CFLAGS		+= -Iinc -Wall
CXXFLAGS	+= -Iinc -Wall
LDFLAGS		+= -lxenctrl -lxenstore


all: $(APP) $(TST)

$(APP): % : %.o $(LIB)
	$(CXX) $(LDFLAGS) $^ -o $@

$(TST): % : %.o $(LIB)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cc $(INC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c $(INC)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(patsubst %, %.o, $(APP)) $(LIB)

distclean: clean
	rm -rf $(APP)


.PHONY: all clean distclean

