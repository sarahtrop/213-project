ROOT     := .
TARGETS  := project
CXXFLAGS := `sdl2-config --cflags` -g -O0 --std=c++11 -o0
LDFLAGS  := `sdl2-config --libs` -lpthread

include $(ROOT)/common.mk
