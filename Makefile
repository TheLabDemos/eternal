libnames := 3dengfx gfx nlibase dsys common n3dmath2
obj := src/src_pack.o $(foreach lib,$(libnames),src/$(lib)/$(lib)_pack.o)

opt := -O3 -mmmx -msse

CXXFLAGS := $(opt) -ansi -pedantic -Wall -DSINGLE_PRECISION_MATH\
			-Icommon -Igfx -I3dengfx -Inlibase `sdl-config --cflags`

demo: $(obj)
	$(CXX) -o $@ $(obj) -lGL -lGLU -lbz2 `sdl-config --libs` `pkg-config libpng --libs` -lvorbisfile -lvorbis -logg
	mv demo bin/
	rm -f eternal; rm -f eternal_nogui
	make config_tool
	ln -s bin/demo eternal_nogui

%.o:
	cd $(@D); make

#%.o:
#	cd src; make

3dengfx:
	cd src/3dengfx; make; cd ../..

gfx:
	cd src/gfx; make; cd ../..

dsys:
	cd src/dsys; make; cd ../..

config_tool:
	cd src/config_tool; make; cd ../..; ln -s src/config_tool/config_tool eternal

.PHONY: clean
clean:
	@echo Cleaning...
	cd src/common; make clean
	cd src/3dengfx; make clean
	cd src/gfx; make clean
	cd src/nlibase; make clean
	cd src/dsys; make clean
	cd src/n3dmath2; make clean
	cd src; make clean
