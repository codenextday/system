.PHONY:  clean all install

all:
	$(MAKE) -C isi $@
	$(MAKE) -C mem2mem_test $@
	$(MAKE) -C isp_daemon $@
	$(MAKE) -C isp_debug $@
	@echo "\n--------------all ok----------------"

clean:
	$(MAKE) -C isi $@ clean
	$(MAKE) -C mem2mem_test $@ clean
	$(MAKE) -C isp_daemon $@ clean
	$(MAKE) -C isp_debug $@ clean

install:
	$(MAKE) -C mem2mem_test $@ DESTDIR=$(DESTDIR)
	$(MAKE) -C isi $@ DESTDIR=$(DESTDIR)
	$(MAKE) -C isp_daemon $@ DESTDIR=$(DESTDIR)
	$(MAKE) -C isp_debug $@ DESTDIR=$(DESTDIR)
	$(MAKE) -C prebuildlib $@ DESTDIR=$(DESTDIR)

