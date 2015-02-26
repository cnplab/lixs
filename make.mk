CCOMPILE	 = $(CC) $(CFLAGS) -c $(1) -o $(2)
CLINK		 = $(CC) $(CFLAGS) $(1) $(LDFLAGS) -o $(2)
CXXCOMPILE	 = $(CXX) $(CXXFLAGS) -c $(1) -o $(2)
CXXLINK		 = $(CXX) $(CXXFLAGS) $(1) $(LDFLAGS) -o $(2)

ifneq ($(verbose),y)
ccompile	 = @printf " %-4s %s\n" "CC"  $@ && $(CCOMPILE)
clink		 = @printf " %-4s %s\n" "LD"  $@ && $(CLINK)
cxxcompile	 = @printf " %-4s %s\n" "CXX" $@ && $(CXXCOMPILE)
cxxlink		 = @printf " %-4s %s\n" "LD"  $@ && $(CXXLINK)
cmd			 = @printf " %-4s %s\n" $(1) $(2) && $(3) $(4)
else
ccompile	 = $(CCOMPILE)
link		 = $(CLINK)
cxxcompile	 = $(CXXCOMPILE)
cxxlink		 = $(CXXLINK)
cmd			 = $(3) $(4)
endif

