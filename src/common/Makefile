obj := color2.o curves.o image.o logger.o config_parser.o timer.o

opt := #-O3 -mmmx -msse -mfpmath=sse

CXXFLAGS := $(opt) -ansi -pedantic -Wall -DSINGLE_PRECISION_MATH -I../n3dmath2
CFLAGS := $(opt) -ansi -pedantic -Wall

#common.a: $(obj)
#	ar cru $@ $(obj)
#	ranlib $@

common_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

color2.o: color2.cpp color2.hpp
curves.o: curves.cpp curves.hpp

image.o: image.c image.h
logger.o: logger.c logger.h
config_parser.o: config_parser.c config_parser.h
timer.o: timer.c timer.h


.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o
