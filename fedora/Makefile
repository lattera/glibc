# Makefile for maintaining glibc fedora-branch and creating Fedora source RPMs.

glibc.spec: # The default target.

tag-prefix := fedora-

.PHONY: update commit-merge tag archive finish_archive srpm rpm

snapshot-date-fmt := +'%Y-%m-%d %H:%M %Z'
snapshot-name-fmt := +'%Y%m%dT%H%M'

include branch.mk


on-branch = $(filter-out HEAD,$(glibc-base))

snapshot-date := $($(glibc-branch)-sync-date)
snapshot-name := $(shell date -u -d '$(snapshot-date)' $(snapshot-name-fmt))

tag-name := glibc$(patsubst glibc-%-branch,-%,\
			    $(filter-out HEAD,$(glibc-base)))
tar-name := $(subst _,.,$(tag-name))
select-snapshot = -r '$($(glibc-branch)-sync-tag)'
snapshot-id = $($(glibc-branch)-sync-tag)

branch-name := $(patsubst %-$(patsubst glibc-%,%,$(tag-name)),%,\
			  $(glibc-branch))

branch.mk-vars = glibc-branch glibc-base DIST_BRANCH COLLECTION \
		 releases-url

ifeq (,$(upstream-tag))
ifneq (,$(upstream))
upstream-tag := glibc-$(subst .,_,$(upstream))
endif
endif

update:
	now=`date -u $(snapshot-date-fmt)`; \
	$(if $(upstream-tag),new='$(upstream-tag)';,\
	name=`date -u -d "$$now" $(snapshot-name-fmt)`; \
	new="$(tag-prefix)$(tag-name)-$$name"; \
	cvs -Q rtag $(if $(on-branch),-r $(glibc-base),-D "$$now") $$new libc;\
	)\
	(echo '# This file is updated automatically by Makefile.'; \
	 $(foreach var,$(branch.mk-vars) $(if $(upstream-tag),upstream),\
		   $(if $($(var)),echo '$(var) := $($(var))';)) \
	 echo "$(glibc-branch)-sync-date := $$now"; \
	 echo "$(glibc-branch)-sync-tag := $$new"; \
	) > branch.mk; \
	cd ..; (set -x; cvs -q update -d -P \
			    -j'$($(glibc-branch)-sync-tag)' -j"$$new") \
		   2>&1 | tee fedora/update.log

.PHONY: snapshot-time
snapshot-time: branch.mk
	date -d '$(snapshot-date)'

commit-merge:
	cd ..; cvs -Q commit -m"Updated to $(snapshot-id)"

glibc.spec: glibc.spec.in ../version.h branch.mk
	(echo '%define glibcdate $(snapshot-name)'; \
	 echo '%define glibcname $(tar-name)'; \
	 echo '%define glibcsrcdir $(srcdir-name)'; \
	 echo '%define glibc_release_tarballs $(if $(upstream),1,0)'; \
	 sed -n '/VERSION/s/^.*"\([^"]*\)"$$/%define glibcversion \1/p' \
	     ../version.h; \
	 cat $<) > $@.new
	mv -f $@.new $@

ifeq (,$(wildcard glibc.spec))
Makefile: glibc.spec ;
else

spec-nvr := $(shell rpm -q --qf '%{NAME}-%{VERSION}-%{RELEASE}\n' \
			--specfile glibc.spec | sed 1q)
spec-tag = $(subst .,_,$(spec-nvr))

ifeq ($(tagged),no)
select-release = -r $(glibc-branch)-branch
else
select-release = -r $(tag-prefix)$(spec-tag)
endif

tag: glibc.spec
	cd ..; cvs -Q tag -c $(tag-prefix)$(spec-tag)

force-tag: glibc.spec
	cd ..; cvs -Q tag -F -c $(tag-prefix)$(spec-tag)

ifeq (,$(upstream))
srcdir-name = $(tar-name)-$(snapshot-name)
else
tar-name = glibc-$(upstream)
srcdir-name = $(tar-name)

ifneq ($($(glibc-branch)-sync-tag),$(upstream-tag))
define upstream-tag-check
	@echo Need to run make update upstream=$(upstream); exit 1
endef
endif
endif

# Note this rule assumes that your working directory (..) is up to date.
$(tar-name)-$(branch-name).patch: makepatch.awk glibc.spec \
				  ../ChangeLog ../*/ChangeLog
	$(upstream-tag-check)
# Use -kk to suppress patches for $ Id $ differences between branches.
	(cd ..; cvs -q diff -kk -upN $(select-snapshot) $(select-release)) | \
	awk -v OLDVER=$(srcdir-name) \
	    -v NEWVER=$(srcdir-name)-$(branch-name) \
	    -f $< > patch.tmp
	mv -f patch.tmp $@

# makepatch.awk omits these files from the patch; we put them in a tar file.
outside-patch = fedora c_stubs rtkaio powerpc-cpu \
		localedata/charmaps/GB18030 iconvdata/gb18030.c

$(tar-name)-$(branch-name)-$(snapshot-name).tar.bz2: Makefile branch.mk \
						     ../ChangeLog \
						     ../*/ChangeLog
	$(upstream-tag-check)
	@rm -rf libc
	cvs -Q export $(select-release) $(addprefix libc/,$(outside-patch))
	tar cf - -C libc $(outside-patch) | bzip2 -9 > $@.new
	rm -rf libc
	mv -f $@.new $@

glibc-tarballs = $(srcdir-name).tar.bz2
ifeq (,$(upstream))
$(srcdir-name).tar.bz2: Makefile branch.mk
	@rm -rf $(srcdir-name)
	cvs -Q export -d $(srcdir-name) $(select-snapshot) libc
	tar cf - $(srcdir-name) | bzip2 -9 > $@.new
	rm -rf $(srcdir-name)
	mv -f $@.new $@
else
glibc-tarballs += $(srcdir-name:glibc-%=glibc-linuxthreads-%).tar.bz2 \
		  $(srcdir-name:glibc-%=glibc-libidn-%).tar.bz2

$(glibc-tarballs):
	curl -O $(releases-url)/$@
endif

archives = $(glibc-tarballs) \
	   $(tar-name)-$(branch-name)-$(snapshot-name).tar.bz2 \
	   $(tar-name)-$(branch-name).patch

finish_archive: $(archives)

archive: glibc.spec
	$(MAKE) tag finish_archive

rpm srpm: $(spec-nvr).src.rpm
$(spec-nvr).src.rpm: glibc.spec $(archives)
	rpmbuild --define "_sourcedir `pwd`" \
		 --define "_specdir `pwd`" \
		 --define "_srcrpmdir `pwd`" \
		 --nodeps -bs $<

###
# These rules are for dealing with the Fedora package repository
# and the Red Hat internal build system.
#
# To use this, put in ~/.cvsdistrc at least a line:
#	DIST_COMMON = /some/checkout/of/dist/common
# We will find cvs-import.sh there and use its CVS/Root for where to commit.

.PHONY: dist-import

-include ~/.cvsdistrc

ifdef DIST_COMMON
DIST_BRANCH ?= devel
COLLECTION ?= dist-fc4
BHC_FLAGS ?= -n

bhc-baseurl := cvs://cvs.devel.redhat.com/cvs/dist?rpms
bhc-url = $(bhc-baseurl)/glibc/$(DIST_BRANCH)\#$(spec-tag)

dist-import: $(spec-nvr).src.rpm
	$(DIST_COMMON)/cvs-import.sh -b $(DIST_BRANCH) $<
	rpm -qp $< --queryformat '[%{FILEMD5S}  %{FILENAMES}\n]' > $@.new
	mv -f $@.new $@

dist-build: dist-import
	bhc $(BHC_FLAGS) $(COLLECTION) '$(bhc-url)'

endif
###

endif
