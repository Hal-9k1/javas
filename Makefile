src := $(wildcard *.cpp)
obj := $(src:.cpp=.o)
headers := $(wildcard *.hpp)
errors := -Wall -Wextra -Werror 
.PHONY: all clean
all: javas
javas: $(obj)
	g++ $(errors) -o $@ $^
$(obj): %.o: %.cpp
	g++ $(errors) -c -o $@ $< -I.
clean:
	rm -rf javas $(obj)
