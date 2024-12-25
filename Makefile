MAKEFLAGS += --no-builtin-rules

include Makefile.localconfig

ifdef WINDOWS
  ifndef CXX
    CXX = x86_64-w64-mingw32-g++
  endif
  LDLIBS := -lole32
else
  ifndef CXX
    CXX = g++
  endif
endif

ifdef DEBUG
  debug_defs := DEBUG _DEBUG
  debug_cxx_flags := -O0 -ggdb3
else
  debug_defs := NDEBUG
  debug_cxx_flags := -O3 -s
endif

ifndef NOWARN
  warnings := -Wall -Wextra -Werror
endif

triplet := $(shell $(CXX) -dumpmachine)

# Equality function
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

# Resource files
# Add help.txt even though it's built; it's a special case
res := $(filter-out %.hpp %.o,$(wildcard res/*) res/help.txt)
# Resource headers (needed for generate_dep.sh)
resh := $(addsuffix .hpp,$(res))
# Source files
src := $(wildcard *.cpp)
# Source headers (needed for generate_dep.sh)
hdr := $(wildcard *.hpp)
# Source dependencies
dep := $(src:.cpp=.d)
# Source objects
obj := $(src:.cpp=.o)
# Preprocessor definitions
defs := COMPILE_TRIPLET=\"$(triplet)\" GIT_COMMIT=\"$(shell git rev-parse --short HEAD)\"\
  GIT_TAG=\"$(shell git name-rev --name-only --tags HEAD)\" $(debug_defs)
# Compiler/preprocessor flags
CXXFLAGS := $(warnings) $(debug_cxx_flags) $(addprefix -D,$(defs))
# Linker flags
LDFLAGS := -static -static-libstdc++

.PHONY: all clean
all: javas

# Build missing dependencies (existing ones can remake themselves)
$(filter-out $(wildcard *.d),$(dep)): %.d: %.cpp
	./generate_dep.sh "$(CXX)" "$(<:.cpp=)"
	
# Build executable
# Only evaluate wildcard at linking time, when the resh rule would have made them as intermediates
# already
javas: $(obj)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(wildcard res/*.o) $(LDLIBS) -o $@

# Build source objects
$(obj): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I.

# Build resource headers as needed by deps
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

# Build resource objects as needed by resh rule
res/%.o: res/%
	ld -r -b binary -o $@ $<
# Don't delete res/*.o because they're needed for linking, even though javas doesn't have a
# dependency on them (figuring out which are needed by included headers would be a pain):
.NOTINTERMEDIATE: res/%.o

# Deletes all generated files
clean:
	rm -rf javas $(obj) $(wildcard res/*.hpp) $(wildcard res/*.o) $(dep) res/help.txt

# Existing dependency files have rules to update themselves. This also directs make to build
# missing dep files.
# Don't update/generate dependency files if clean is the only target.
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
