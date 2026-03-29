CXX = g++
CXXFLAGS = -std=c++20 -MMD -MP -I$(HOME)/Neniy

ifndef mc_version
$(error mc_version is not set)
endif

ifeq ($(build), release)
CXXFLAGS += -O2
else
CXXFLAGS += -g -fsanitize=address,undefined
endif

SRC = $(wildcard $(mc_version)/*.cpp)
OBJ_DIR = object
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

CORE_SO = $(HOME)/Neniy/core.so

all: $(mc_version)/impl.so

$(OBJ): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(mc_version)/impl.so: $(OBJ) $(CORE_SO)
	$(CXX) $(CXXFLAGS) -shared -fPIC $(OBJ) $(CORE_SO) -Wl,-rpath,$(HOME)/Neniy -o $@
	cp $@ $(HOME)/Neniy/asset/$(mc_version)

clean:
	rm -f $(mc_version)/impl.so
	rm -rf object

.PHONY: all clean

-include $(OBJ:.o=.d)
