obj := 	demo2.o \
		part_start.o \
		part_volsph.o \
		part_hairy.o \
		part_tunnel.o \
		part_tunnel2.o \
		part_pic.o \
		part_statues.o \
		sdlvf.o

opt := -O3 -mmmx -msse

CXXFLAGS := $(opt) -ansi -pedantic -Wall -DSINGLE_PRECISION_MATH\
			-Icommon -Igfx -I3dengfx -Inlibase -Idsys -In3dmath2 `sdl-config --cflags`

CFLAGS := $(CXXFLAGS)

src_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f $(obj) *.o *.d
