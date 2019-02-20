rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC=g++
CFLAGS+=-I. -Wall -std=c++11
VPATH = %.cpp %.o Matcher
DEPS = $(call rwildcard,, *.hpp)
SRC = $(call rwildcard,, *.cpp)
OBJ = $(SRC:.cpp=.o)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

INDEX: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	find . -type f -name '*.o' -delete
