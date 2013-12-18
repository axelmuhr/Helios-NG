# make the various documents

ROFF=nroff

all: howto manner standard copyright install

install: install.mn tmac.n
	$(ROFF) tmac.n install.mn >install.doc

howto: howto.mn tmac.n
	$(ROFF) tmac.n howto.mn >howto.doc

manner: manner.mn tmac.n
	$(ROFF) tmac.n manner.mn>manner.doc

standard: standard.mn tmac.n
	$(ROFF) tmac.n standard.mn>standard.doc

copyright: copyright.mn tmac.n
	$(ROFF) tmac.n copyright.mn>copyright.doc
