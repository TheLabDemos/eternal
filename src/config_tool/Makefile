obj := config_tool.o

CXXFLAGS := -g -ansi -pedantic -Wall `pkg-config gtkmm-2.0 libglademm-2.0 --cflags`

config_tool: $(obj)
	$(CXX) -o $@ $(obj) `pkg-config gtkmm-2.0 libglademm-2.0 --libs`

%.d: %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

include $(obj:.o=.d)

.PHONY: clean
clean:
	@echo Cleaning...
	@rm -f $(obj) *.d
