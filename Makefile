MAKEFLAGS += --no-builtin-rules
WINDOWS := 1

ifdef WINDOWS
  CXX = x86_64-w64-mingw32-g++
  LDLIBS := -lole32
else
  CXX = g++
endif

# Equality function
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

# Resource files
res := $(filter-out %.hpp %.o,$(wildcard res/*))
# Source
src := $(wildcard *.cpp)
# Source headers
hdr := $(wildcard *.hpp)
# Source dependencies
dep := $(src:.cpp=.d)
# Source objects
obj := $(src:.cpp=.o)
# Preprocessor definitions
defs := CXX_COMPILER=\"$(CXX)\" GIT_COMMIT=\"$(shell git rev-parse --short HEAD)\"\
  GIT_TAG=\"$(shell git name-rev --name-only --tags HEAD)\"
# Compiler/preprocessor flags
CXXFLAGS := -Wall -Wextra -Werror -O0 -ggdb3 $(addprefix -D,$(defs))
# Linker flags
LDFLAGS := -static -static-libstdc++

.PHONY: all clean
all: javas

$(filter-out $(wildcard *.d),$(dep)): %.d: %.cpp
	./generate_dep.sh "$(CXX)" "$(<:.cpp=)"
	
javas: $(obj)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(wildcard res/*.o) $(LDLIBS) -o $@
$(obj): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I.
# Note binary_FILE_size is meaningless with position independent linking
resh_template := $\
	\#ifndef FILE_HPP\n$\
	\#define FILE_HPP\n$\
  extern char *_binary_FILE_start;\n$\
	extern char *_binary_FILE_end;\n$\
	const char *pResourceFile_FILE = reinterpret_cast<const char *>(&_binary_FILE_start);\n$\
	const unsigned int resourceLength_FILE = reinterpret_cast<const char *>(&_binary_FILE_end)$\
	- reinterpret_cast<const char *>(&_binary_FILE_start);\n$\
	\#endif
res/%.hpp: res/%.o
	echo -e '$(subst FILE,$(shell echo $(<:.o=) | tr ./ _),$(resh_template))' > $@
res/%.o: res/%
	ld -r -b binary -o $@ $<
# Don't delete res/*.o because they're needed for linking, even though javas doesn't have a
# dependency on them (figuring out which are needed by included headers would be a pain):
.NOTINTERMEDIATE: res/%.o

clean:
	rm -rf javas $(obj) $(wildcard res/*.hpp) $(wildcard res/*.o) $(dep) res/help.txt

# Existing dependency files have rules to update themselves. This also directs make to build
# everything in $(dep_missing). Don't update/generate dependency files if clean is the only target.
ifeq (,$(call eq,clean,$(MAKECMDGOALS)))
  include $(dep)
endif


# Dumb one-off rules that I'm too lazy to generalize and make conventions for:
ifdef WINDOWS
res/help.txt: res/help-windows.txt
	cp $^ $@
else
res/help.txt: res/help-unixish.txt
	cp $^ $@
endif
