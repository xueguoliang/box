
CPPFLAGS += -fPIC
CC:=g++
src:=$(shell find *.cpp)
obj:=$(src:.cpp=.o)
dep:=$(src:.cpp=.d)

target:=libbox.so

all: $(target)

-include $(dep)

%.d: %.cpp
	$(CC) -MM $< > $@

$(target): $(obj)
	$(CC) -fPIC -shared $^ -o $@

.PHONY: clean install uninstall
clean:
	rm -rf *.o *.d *.so

install:
	mkdir /usr/include/box
	cp *.h /usr/include/box
	cp *.so /usr/lib

uninstall:
	rm -rf /usr/include/box
	rm -rf /usr/lib/libbox.so

