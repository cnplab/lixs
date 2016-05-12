# Define verbose command
ifneq ($(verbose),y)
cmd			 = @printf " %-10s %s\n" $(1) $(2) && $(3) $(4)
else
cmd			 = $(3) $(4)
endif

# Compile and link commands
ccompile	 = $(call cmd, "CC", $2, $(CC), $(CFLAGS) -c $(1) -o $(2))
clink		 = $(call cmd, "LD", $2, $(CC), $(CFLAGS) $(1) $(LDFLAGS) -o $(2))
cxxcompile	 = $(call cmd, "CXX", $2, $(CXX), $(CXXFLAGS) -c $(1) -o $(2))
cxxlink		 = $(call cmd, "LD", $2, $(CXX), $(CXXFLAGS) $(1) $(LDFLAGS) -o $(2))


# Default build rules for objects
%.o: %.cc $(config)
	$(call cxxcompile, $<, $@)

%.o: %.c $(config)
	$(call ccompile, $<, $@)
