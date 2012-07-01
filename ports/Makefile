# This makefile is not used by the glibc build process.
# It's purely for making ports tarballs.

.PHONY: dist dist-ports
dist: dist-ports

# Do `make dist dist-version=X.Y.Z' to make tar files of an older version.
dist-version = $(version)
# Also try 'dist-tag=some="-r TAG"' (or -D DATE) to get some tag rather
# than the release tag for X.Y.Z.
dist-tag = -r glibc-$(subst .,_,$(dist-version))

distname = glibc-ports-$(dist-version)

do-export = cvs $(CVSOPTS) -Q export -d $(basename $@) $(dist-tag)

dist-ports: $(foreach Z,.bz2 .gz,$(distname).tar$Z)
	md5sum $^
$(distname).tar:
	@rm -fr $(basename $@)
	$(MAKE) -q `find sysdeps -name configure`
	$(do-export) ports
	find $(basename $@) -name configure -print | xargs touch
	tar cf $@ $(basename $@)
	rm -fr $(basename $@)

sysdeps-of-stem = sysdeps/$* sysdeps/unix/sysv/linux/$*

.PRECIOUS: %.gz %.bz2 # Don't delete output as intermediate files.
dist-port-%: $(foreach Z,.bz2 .gz,glibc-port-%-$(dist-version).tar$Z)
	md5sum $^
glibc-port-%-$(dist-version).tar: ChangeLog.%
	@rm -fr $(basename $@)
	$(MAKE) -q `find $(sysdeps-of-stem) -name configure`
	$(do-export) ports/ChangeLog.$* $(addprefix ports/,$(sysdeps-of-stem))
	mv $(basename $@)/ports/* $(basename $@)/
	rmdir $(basename $@)/ports
	find $(basename $@) -name configure -print | xargs touch
	tar cf $@ $(basename $@)
	rm -fr $(basename $@)

%.bz2: %; bzip2 -9vk $<
%.gz: %; gzip -9vnc $< > $@.new && mv -f $@.new $@
