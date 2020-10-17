.PHONY:  clean all install

TARGET_mipi_dphy=drv/mipi_dphy/mipi_dphy.drv
TARGET_AR330=drv/AR330/ar330.drv
TARGET_AR230=drv/AR230/ar230.drv
TARGET_OV5648=drv/OV5648/ov5648.drv
TARGET_OV5695=drv/OV5695/ov5695.drv

all:
	$(MAKE) -C drv $@
	cp drv/*/*.so ../prebuildlib
	@echo "\n--------------all sensors drv ok----------------"

clean:
	$(MAKE) -C drv $@ clean

install:
	@echo "\n--------------cp ar330 & ar230 ok &----------------"
	@install -m 755 -d $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_mipi_dphy) $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_AR330) $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_AR230) $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_OV5648) $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_OV5695) $(DESTDIR)/usr/lib/
	$(MAKE) -C drv $@ DESTDIR=$(DESTDIR)

