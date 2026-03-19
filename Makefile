CXX = g++
CXXFLAGS = -std=c++20 -I$(HOME)/Neniy

ifndef mc_version
$(error mc_version is not set)
endif

ifeq ($(build), release)
	CXXFLAGS += -O2
else
	CXXFLAGS += -g -fsanitize=address,undefined
endif

SRC = $(wildcard $(mc_version)/*.cpp)
OBJ = $(SRC:.cpp=.o)

all: build

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

build: $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -fPIC $^ $(HOME)/Neniy/core.so -Wl,-rpath,$(HOME)/Neniy -o $(mc_version)/impl.so

clean:
	rm -rf $(OBJ)

.PHONY: all build clean
