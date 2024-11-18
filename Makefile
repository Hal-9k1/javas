res := $(filter-out %.hpp %.o,$(wildcard res/*))
$(info $(res))
src := $(wildcard *.cpp)
srco := $(src:.cpp=.o)
reso := $(addsuffix .o,$(res))
resh := $(addsuffix .hpp,$(res))
obj := $(srco) $(reso)
errors := -Wall -Wextra -Werror 

.PHONY: all clean
all: javas
javas: $(obj)
	g++ $(errors) -o $@ $^
$(srco): %.o: %.cpp
	g++ $(errors) -c -o $@ $< -I.
# note binary_FILE_size is meaningless with position independent linking
resh_template := \
  extern char *binary_FILE_start;\n\
	extern char *binary_FILE_end;
$(resh): %.hpp: %.o
	echo -e '$(subst FILE,$(shell echo $(<:.o=) | tr ./ _),$(resh_template))' > $@
$(reso): %.o: %
	ld -r -b binary -o $@ $<
	
clean:
	rm -rf javas $(obj) $(resh)
