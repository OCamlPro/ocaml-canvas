.PHONY: all build build-deps fmt fmt-check install dev-deps test
.PHONY: doc-common odoc view sphinx
.PHONY: clean distclean

DEV_DEPS := merlin ocamlformat odoc ppx_expect ppx_inline_test


SPHINX_TARGET:=_build/docs/sphinx

ODOC_TARGET:=_build/docs/doc/.


all: build

build:
	-opam exec -- dune build @configure --auto-promote
	opam exec -- dune build @all

build-deps:
	if ! [ -e _opam ]; then \
	   opam switch create . 4.14.0 ; \
	fi
	opam install ./*.opam --deps-only

doc-common: build
	mkdir -p _build/docs
	rsync -auv docs/. _build/docs/.

sphinx: doc-common
	sphinx-build sphinx ${SPHINX_TARGET}

odoc: doc-common
	mkdir -p ${ODOC_TARGET}
	opam exec -- dune build @doc
	rsync -auv --delete _build/default/_doc/_html/. ${ODOC_TARGET}

doc: doc-common odoc sphinx

view:
	xdg-open file://$$(pwd)/_build/docs/index.html

fmt:
	opam exec -- dune build @fmt --auto-promote

fmt-check:
	opam exec -- dune build @fmt

install:
	opam pin -y --no-action -k path .
	opam install -y .

opam:
	opam pin -k path .

uninstall:
	opam uninstall .

dev-deps:
	opam install ./*.opam --deps-only --with-doc --with-test

test:
	opam exec -- dune build @runtest

clean:
	rm -rf _build

distclean: clean
	rm -rf _opam
