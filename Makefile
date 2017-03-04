CXX = g++

CXXFLAGS = -Wall -g -std=c++14 -MD -fPIC -O3

LIBS = 

SRCS = $(wildcard src/*.cpp)

OBJS = $(SRCS:.cpp=.o)

MAIN = xmunch

.PHONY: depend clean

all: $(MAIN) test
	@echo "xmunch build."

$(MAIN): $(OBJS)
	@echo Linking...
	@$(CXX) $(CXXFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

%.o : %.cpp
	@echo "$< --> $@"
	@$(CXX) $(CXXFLAGS) -c $<  -o $@
	@cp $*.d $*.P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
		rm -f "$*.d"

test: $(MAIN) tests/*.*
	@echo "running tests"
	@tests/run

clean:
	@rm src/*.o  $(MAIN)


-include $(SRCS:.cpp=.P)
