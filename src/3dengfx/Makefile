obj :=  3denginefx.o textures.o camera.o except.o material.o\
	object.o texman.o light.o load_geom.o\
	ggen.o 3dscene.o sceneloader.o


CXXFLAGS := -g -ansi -pedantic -Wall -DSINGLE_PRECISION_MATH\
			-I. -I../common -I../gfx -I../nlibase -I../n3dmath2 `sdl-config --cflags`

3dengfx_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$


.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
