src = $(wildcard src/*.cpp) $(wildcard src/3dengfx/*.cpp) $(wildcard src/gfx/*.cpp) \
	  $(wildcard src/dsys/*.cpp) $(wildcard src/n3dmath2/*.cpp) $(wildcard src/common/*.cpp)
csrc = $(wildcard src/*.c) $(wildcard src/dsys/*.c) $(wildcard src/nlibase/*.c) \
	   $(wildcard src/common/*.c)

obj = $(src:.cpp=.o) $(csrc:.c=.o)
dep = $(src:.cpp=.d) $(csrc:.c=.d)
bin = demo

warn = -pedantic -Wall
#opt = -O3 -ffast-math
dbg = -g
def = -DSINGLE_PRECISION_MATH
inc = -Isrc/common -Isrc/gfx -Isrc/3dengfx -Isrc/nlibase -Isrc/n3dmath2 -Isrc/dsys

CFLAGS = $(warn) $(opt) $(dbg) $(inc) $(def) -MMD `sdl-config --cflags`
CXXFLAGS = $(warn) $(opt) $(dbg) $(inc) $(def) -MMD `sdl-config --cflags`
LDFLAGS = -lGL -lGLU -lbz2 -lpng -lvorbisfile -lvorbis -logg `sdl-config --libs`

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
