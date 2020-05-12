#
# Makefile
#
# See the README file for copyright information and how to reach the author.
#
#

include Make.config

TARGET      = p4d
CMDTARGET   = p4
CHARTTARGET = dbchart
HISTFILE    = "HISTORY.h"

LIBS += $(shell mysql_config --libs_r) -lrt -lcrypto -lcurl -lpthread

DEFINES += -D_GNU_SOURCE -DTARGET='"$(TARGET)"'

VERSION      = $(shell grep 'define _VERSION ' $(HISTFILE) | awk '{ print $$3 }' | sed -e 's/[";]//g')
ARCHIVE      = $(TARGET)-$(VERSION)
DEB_BASE_DIR = /root/debs
DEB_DEST     = $(DEB_BASE_DIR)/p4d-$(VERSION)

LASTHIST     = $(shell grep '^20[0-3][0-9]' $(HISTFILE) | head -1)
LASTCOMMENT  = $(subst |,\n,$(shell sed -n '/$(LASTHIST)/,/^ *$$/p' $(HISTFILE) | tr '\n' '|'))
LASTTAG      = $(shell git describe --tags --abbrev=0)
BRANCH       = $(shell git rev-parse --abbrev-ref HEAD)
GIT_REV      = $(shell git describe --always 2>/dev/null)

# object files

LOBJS        =  lib/db.o lib/dbdict.o lib/common.o lib/serial.o lib/curl.o
OBJS         = $(LOBJS) main.o p4io.o service.o w1.o webif.o hass.o
MQTTBJS      = lib/mqtt.c
CHARTOBJS    = $(LOBJS) chart.o
CMDOBJS      = p4cmd.o p4io.o lib/serial.o service.o w1.o lib/common.o

CFLAGS    	+= $(shell mysql_config --include)
DEFINES   	+= -DDEAMON=P4d
OBJS      	+= p4d.o

ifdef TEST_MODE
	DEFINES += -D__TEST
endif

ifdef HASSMQTT
   OBJS    += $(MQTTBJS)
   LIBS    += -lpaho-mqtt3cs
	DEFINES += -DMQTT_HASS
endif

ifdef GIT_REV
   DEFINES += -DGIT_REV='"$(GIT_REV)"'
endif

# rules:

all: $(TARGET) $(CMDTARGET) $(CHARTTARGET)

$(TARGET) : paho-mqtt $(OBJS)
	$(doLink) $(OBJS) $(LIBS) -o $@

$(CHARTTARGET): $(CHARTOBJS)
	$(doLink) $(CHARTOBJS) $(LIBS) -o $@

$(CMDTARGET) : $(CMDOBJS)
	$(doLink) $(CMDOBJS) $(LIBS) -o $@

install: $(TARGET) $(CMDTARGET) install-p4d

install-p4d: install-config install-scripts
	install --mode=755 -D $(TARGET) $(BINDEST)
	install --mode=755 -D $(CMDTARGET) $(BINDEST)
	make install-$(INIT_SYSTEM)
   ifneq ($(DESTDIR),)
	   @cp -r contrib/DEBIAN $(DESTDIR)
	   @chown root:root -R $(DESTDIR)/DEBIAN
		sed -i s:"<VERSION>":"$(VERSION)":g $(DESTDIR)/DEBIAN/control
	   @mkdir -p $(DESTDIR)/usr/lib
	   @mkdir -p $(DESTDIR)/usr/bin
	   @mkdir -p $(DESTDIR)/usr/share/man/man1
   endif

inst_restart: $(TARGET) $(CMDTARGET) install-config install-scripts
	systemctl stop p4d
	@cp -p $(TARGET) $(CMDTARGET) $(BINDEST)
	systemctl start p4d

install-sysV:
	install --mode=755 -D ./contrib/p4d $(DESTDIR)/etc/init.d/
	install --mode=755 -D ./contrib/runp4d $(BINDEST)
	update-rc.d p4d defaults

install-systemd:
	cat contrib/p4d.service | sed s:"<BINDEST>":"$(_BINDEST)":g | sed s:"<AFTER>":"$(INIT_AFTER)":g | install --mode=644 -C -D /dev/stdin $(SYSTEMDDEST)/p4d.service
	chmod a+r $(SYSTEMDDEST)/p4d.service
   ifeq ($(DESTDIR),)
	   systemctl daemon-reload
	   systemctl enable p4d
   endif

install-none:

install-config:
	if ! test -d $(CONFDEST); then \
	   mkdir -p $(CONFDEST); \
	   mkdir -p $(CONFDEST)/scripts.d; \
	   chmod a+rx $(CONFDEST); \
	fi
	if ! test -f $(CONFDEST)/p4d.conf; then \
	   install --mode=644 -D ./configs/p4d.conf $(CONFDEST)/; \
	fi
	install --mode=644 -D ./configs/p4d.dat $(CONFDEST)/;

install-scripts:
	if ! test -d $(BINDEST); then \
		mkdir -p "$(BINDEST)" \
	   chmod a+rx $(BINDEST); \
	fi
	install -D ./scripts/p4d-* $(BINDEST)/

install-web:
	if ! test -d $(WEBDEST); then \
		mkdir -p "$(WEBDEST)"; \
		chmod a+rx "$(WEBDEST)"; \
	fi
	if test -f "$(WEBDEST)/stylesheet.css"; then \
		cp -Pp "$(WEBDEST)/stylesheet.css" "$(WEBDEST)/stylesheet.css.save"; \
	fi
	if test -f "$(WEBDEST)/config.php"; then \
		cp -p "$(WEBDEST)/config.php" "$(WEBDEST)/config.php.save"; \
	fi
	cp -r ./htdocs/* $(WEBDEST)/
	if test -f "$(WEBDEST)/config.php.save"; then \
		cp -p "$(WEBDEST)/config.php" "$(WEBDEST)/config.php.dist"; \
		cp -p "$(WEBDEST)/config.php.save" "$(WEBDEST)/config.php"; \
	fi
	if test -f "$(WEBDEST)/stylesheet.css.save"; then \
		cp -Pp "$(WEBDEST)/stylesheet.css.save" "$(WEBDEST)/stylesheet.css"; \
	fi
	cat ./htdocs/header.php | sed s:"<VERSION>":"$(VERSION)":g > "$(WEBDEST)/header.php"; \
	chmod -R a+r "$(WEBDEST)"; \
	chown -R $(WEBOWNER):$(WEBOWNER) "$(WEBDEST)"

install-apache-conf:
	@mkdir -p $(APACHECFGDEST)/conf-available
	@mkdir -p $(APACHECFGDEST)/conf-enabled
	install --mode=644 -D apache2/p4.conf $(APACHECFGDEST)/conf-available/
	rm -f $(APACHECFGDEST)/conf-enabled/p4.conf
	ln -s ../conf-available/p4.conf $(APACHECFGDEST)/conf-enabled/p4.conf

install-pcharts:
	if ! test -d $(PCHARTDEST); then \
		git clone https://github.com/bozhinov/pChart2.0-for-PHP7.git $(PCHARTDEST); \
	fi
	cd $(PCHARTDEST); \
	git pull; \
	git checkout 7.x-compatible; \
	ln -s $(_PCHARTDEST) $(WEBDEST)/pChart; \
	chown -R $(WEBOWNER):$(WEBOWNER) $(PCHARTDEST); \

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(ARCHIVE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(ARCHIVE).tgz

clean:
	rm -f */*.o *.o core* *~ */*~ lib/t *.jpg
	rm -f $(TARGET) $(CHARTTARGET) $(CMDTARGET) $(ARCHIVE).tgz
	rm -f com2

cppchk:
	cppcheck --template="{file}:{line}:{severity}:{message}" --language=c++ --force *.c *.h

com2: $(LOBJS) c2tst.c p4io.c service.c
	$(CPP) $(CFLAGS) c2tst.c p4io.c service.c $(LOBJS) $(LIBS) -o $@

paho-mqtt:
ifdef HASSMQTT
	if [ ! -d ~/build/paho.mqtt.c ]; then \
		mkdir -p ~/build; \
		cd ~/build; \
		git clone https://github.com/eclipse/paho.mqtt.c.git; \
		sed -i '/if test ! -f ..DESTDIR..{libdir}.lib..MQTTLIB_C..so..{MAJOR_VERSION.; then ln -s/d' ~/build/paho.mqtt.c/Makefile; \
		sed -i '/\- ..INSTALL_DATA. .{blddir}.doc.MQTTClient.man.man3.MQTTClient.h.3 ..DESTDIR..{man3dir}/d' ~/build/paho.mqtt.c/Makefile; \
		sed -i '/\- ..INSTALL_DATA. .{blddir}.doc.MQTTAsync.man.man3.MQTTAsync.h.3 ..DESTDIR..{man3dir}/d' ~/build/paho.mqtt.c/Makefile; \
		sed -i s/'rm [$$]'/'rm -f $$'/g ~/build/paho.mqtt.c/Makefile; \
	fi
	cd ~/build/paho.mqtt.c; \
	make -s; \
	sudo rm -f /usr/local/lib/libpaho*; \
	sudo make -s uninstall prefix=/usr; \
	sudo make -s install prefix=/usr
endif

build-deb:
	rm -rf $(DEB_DEST)
	make -s install-p4d DESTDIR=$(DEB_DEST) PREFIX=/usr INIT_AFTER=mysql.service
	make -s install-web DESTDIR=$(DEB_DEST) PREFIX=/usr
	make -s install-apache-conf DESTDIR=$(DEB_DEST) PREFIX=/usr
	make -s install-pcharts DESTDIR=$(DEB_DEST) PREFIX=/usr
#	cat contrib/p4d.service | sed s:"<BINDEST>":"$(_BINDEST)":g | sed s:"<AFTER>":"$(INIT_AFTER)":g | install --mode=644 -C -D /dev/stdin $(DEB_DEST)/DEBIAN/p4d.service
#	chmod a+r $(DEB_DEST)/DEBIAN/p4d.service
	cd ~/build/paho.mqtt.c; \
	make -s install DESTDIR=$(DEB_DEST) prefix=/usr
	dpkg-deb --build $(DEB_BASE_DIR)/p4d-$(VERSION)

publish-deb:
	echo 'put $(DEB_BASE_DIR)/p4d-${VERSION}.deb' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d
	echo 'put contrib/install-deb.sh' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d
	echo 'rm p4d-latest.deb' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d
	echo 'ln -s p4d-${VERSION}.deb p4d-latest.deb' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d
	echo 'chmod 644 p4d-${VERSION}.deb' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d
	echo 'chmod 755 install-deb.sh' | sftp -i ~/.ssh/id_rsa2 p7583735@home26485763.1and1-data.host:p4d

#***************************************************************************
# dependencies
#***************************************************************************

HEADER = lib/db.h lib/dbdict.h lib/common.h

lib/common.o    :  lib/common.c    $(HEADER)
lib/db.o        :  lib/db.c        $(HEADER)
lib/dbdict.o    :  lib/dbdict.c    $(HEADER)
lib/curl.o      :  lib/curl.c      $(HEADER)
lib/serial.o    :  lib/serial.c    $(HEADER) lib/serial.h
lib/mqtt.o      :  lib/mqtt.c      lib/mqtt.h

main.o          :  main.c          $(HEADER) p4d.h
p4d.o           :  p4d.c           $(HEADER) p4d.h p4io.h w1.h lib/mqtt.h
p4io.o          :  p4io.c          $(HEADER) p4io.h
webif.o         :  webif.c         $(HEADER) p4d.h
w1.o            :  w1.c            $(HEADER) w1.h
service.o       :  service.c       $(HEADER) service.h
hass.o          :  hass.c          p4d.h
chart.o         :  chart.c

# ------------------------------------------------------
# Git / Versioning / Tagging
# ------------------------------------------------------

vcheck:
	git fetch
	if test "$(LASTTAG)" = "$(VERSION)"; then \
		echo "Warning: tag/version '$(VERSION)' already exists, update HISTORY first. Aborting!"; \
		exit 1; \
	fi

push: vcheck
	echo "tagging git with $(VERSION)"
	git tag $(VERSION)
	git push --tags
	git push

commit: vcheck
	git commit -m "$(LASTCOMMENT)" -a

git: commit push

showv:
	@echo "Git ($(BRANCH)):\\n  Version: $(LASTTAG) (tag)"
	@echo "Local:"
	@echo "  Version: $(VERSION)"
	@echo "  Change:"
	@echo -n "   $(LASTCOMMENT)"
