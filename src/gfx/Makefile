obj := 3dgeom.o controller.o base_cam.o

opt := -O3 -mmmx -msse

CXXFLAGS := $(opt) -ansi -pedantic -Wall -DSINGLE_PRECISION_MATH\
		-I. -I../common -I../3dengfx -I../n3dmath2

#gfx.a: $(obj)
#	ar crus $@ $(obj)

gfx_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
