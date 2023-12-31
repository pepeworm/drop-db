CXX=g++
CXXFLAGS:=${CXXFLAGS} -Wall -Wextra -Wpedantic -fno-trapping-math -fno-math-errno -fno-signed-zeros -march=native -falign-functions=16 -lstdc++fs -std=c++20
SRCS=$(shell find . | grep \\.cpp$)
HDRS=$(shell find . | grep \\.hpp$)
OBJS=$(SRCS:.cpp=.cpp.o)

.PHONY: debug release clean

# will build debug as default because its at the top
debug: CXXFLAGS += -g
debug: a.out

release: CXXFLAGS += -O2
release: a.out
	strip -s a.out

a.out: $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS)

%.cpp.o: %.cpp $(HDRS) Makefile
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -f $(OBJS) a.out
