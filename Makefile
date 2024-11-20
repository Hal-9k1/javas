MAKEFLAGS += --no-builtin-rules
CXX = g++
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

res := $(filter-out %.hpp %.o,$(wildcard res/*))
src := $(wildcard *.cpp)
hdr := $(wildcard *.hpp)
dep := $(src:.cpp=.d)
srco := $(src:.cpp=.o)
reso := $(addsuffix .o,$(res))
resh := $(addsuffix .hpp,$(res))
obj := $(srco) $(reso)
errors := -Wall -Wextra -Werror -O0 -ggdb3 

.PHONY: all clean
all: javas

$(filter-out $(wildcard *.d),$(dep)): %.d: %.cpp
	./generate_dep.sh "$(CXX)" "$(<:.cpp=)"
	
javas: $(obj)
	$(CXX) $(errors) -o $@ $^
$(srco): %.o: %.cpp
	$(CXX) $(errors) -c -o $@ $< -I.
# note binary_FILE_size is meaningless with position independent linking
resh_template := \
	\#ifndef FILE_HPP\n\
	\#define FILE_HPP\n\
  extern char *_binary_FILE_start;\n\
	extern char *_binary_FILE_end;\n\
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
