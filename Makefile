MAKEFLAGS += --no-builtin-rules
CXX = g++
# Equality function
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

# Resource files
res := $(filter-out %.hpp %.o %_bridge.cpp,$(wildcard res/*))
# Source
src := $(wildcard *.cpp)
# Source headers
hdr := $(wildcard *.hpp)
# Source dependencies
dep := $(src:.cpp=.d)
# Source objects
srco := $(src:.cpp=.o)
# Resource objects
reso := $(addsuffix .o,$(res))
# Resource headers
resh := $(addsuffix .hpp,$(res))
# All objects
obj := $(srco) $(reso)
CXXFLAGS := -Wall -Wextra -Werror -O0 -ggdb3

.PHONY: all clean
all: javas

$(filter-out $(wildcard *.d),$(dep)): %.d: %.cpp
	./generate_dep.sh "$(CXX)" "$(<:.cpp=)"
	
javas: $(obj)
	$(CXX) $(CXXFLAGS) -o $@ $^
$(srco): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I.
# note binary_FILE_size is meaningless with position independent linking
resh_template := $\
	\#ifndef FILE_HPP\n$\
	\#define FILE_HPP\n$\
  extern char *_binary_FILE_start;\n$\
	extern char *_binary_FILE_end;\n$\
	const char *pResourceFile_FILE = reinterpret_cast<const char *>(&_binary_FILE_start);\n$\
	const unsigned int resourceLength_FILE = reinterpret_cast<const char *>(&_binary_FILE_end)$\
	- reinterpret_cast<const char *>(&_binary_FILE_start);\n$\
	\#endif
$(resh): %.hpp: %.o
	echo -e '$(subst FILE,$(shell echo $(<:.o=) | tr ./ _),$(resh_template))' > $@
$(reso): %.o: %
	ld -r -b binary -o $@ $<

clean:
	rm -rf javas $(obj) $(resh) $(dep)

# Existing dependency files have rules to update themselves. This also directs make to build
# everything in $(dep_missing). Don't update/generate dependency files if clean is the only target.
ifeq (,$(call eq,clean,$(MAKECMDGOALS)))
  include $(dep)
endif
