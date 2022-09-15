(lang dune 2.1)
(name ocaml-canvas)
(version 0.9)

(formatting disabled)
(generate_opam_files false)

(license "LGPL-2.1-or-later WITH OCaml-LGPL-linking-exception")
(authors "David Declerck")
(maintainers david.declerck@ocamlpro.com)
(source (github OCamlPro/ocaml-canvas))
(bug_reports https://github.com/OCamlPro/ocaml-canvas/issues)
(homepage https://ocamlpro.github.io/ocaml-canvas/)
(documentation https://ocamlpro.github.io/ocaml-canvas/sphinx)

(package
 (name ocaml-canvas)
 (depends
  (ocaml (and :build (>= 4.03.0)))
  (dune (and :build (>= 2.2)))
  (dune-configurator (and :build (>= 1.11)))
  (conf-freetype (and (<> :os macos) (<> :os win32)))
  (conf-libfontconfig (and (<> :os macos) (<> :os win32)))
  (conf-libxcb (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-shm (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-image (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-xkb (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-keysyms (and (<> :os macos) (<> :os win32)))
  (conf-libwayland (and (<> :os macos) (<> :os win32)))
  (conf-xkbcommon (and (<> :os macos) (<> :os win32)))
  (odoc :with-doc))
 (depopts
  (js_of_ocaml (and :build (>= 3.6))))
 (synopsis "The OCaml-Canvas library")
 (description "\
The OCaml-Canvas library provides a portable Canvas for OCaml.
"))
