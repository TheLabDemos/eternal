obj := nlibase.o

opt := -O3 -mmmx -msse

CFLAGS := $(opt) -std=c89 -pedantic -Wall

#nlibase.a: $(obj)
#	ar crus $@ $(obj)

nlibase_pack.o: $(obj)
	$(LD) -r -o $@ $(obj)

include $(obj:.o=.d)

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f *.o *.d
