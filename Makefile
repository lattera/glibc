# This boilerplate is necessary just because any add-on directory
# gets added as a normal subdirectory for the glibc build process.

subdir = ports

include ../Rules

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
	$(do-export) ports
	tar cf $@ $(basename $@)
	rm -fr $(basename $@)

.PRECIOUS: %.gz %.bz2 # Don't delete output as intermediate files.
dist-port-%: $(foreach Z,.bz2 .gz,glibc-port-%-$(dist-version).tar$Z)
	md5sum $^
glibc-port-%-$(dist-version).tar: configure ChangeLog
	@rm -fr $(basename $@)
	$(do-export) -l ports
	rm -f $(basename $@)/ChangeLog.[a-z]*
	$(do-export) ports/ChangeLog.$* \
		     ports/sysdeps/$* ports/sysdeps/unix/sysv/linux/$*
	mv $(basename $@)/ports/* $(basename $@)/
	rmdir $(basename $@)/ports
	tar cf $@ $(basename $@)
	rm -fr $(basename $@)
