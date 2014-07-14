# lixs: LightweIght XenStore

APP	:=
APP	+= $(patsubst %.cc, %, $(shell find app/ -name "*.cc"))

LIB	:=
LIB	+= $(patsubst %.cc, %.o, $(shell find lib/ -name "*.cc"))

INC	:=
INC	+= $(shell find inc/ -name "*.hh")


CFLAGS		+= -Iinc -Wall
CXXFLAGS	+= -Iinc -Wall
LDFLAGS		+= -lxenctrl


all: $(APP)

$(APP): % : %.o $(LIB)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: $(INC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(patsubst %, %.o, $(APP)) $(LIB)

distclean: clean
	rm -rf $(APP)

