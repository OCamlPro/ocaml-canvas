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
  (ocaml (>= 4.03))
  (dune (>= 2.2))
  (dune-configurator (>= 1.11))
  (react (>= 1.0))
  (odoc (and :with-doc (>= 1.4)))

  (conf-freetype (and (<> :os macos) (<> :os win32)))
  (conf-libfontconfig (and (<> :os macos) (<> :os win32)))
  (conf-xkbcommon (and (<> :os macos) (<> :os win32)))

  (conf-libxcb (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-shm (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-image (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-xkb (and (<> :os macos) (<> :os win32)))
  (conf-libxcb-keysyms (and (<> :os macos) (<> :os win32))))

 (depopts
  (js_of_ocaml (and :build (>= 3.6)))
  (conf-libwayland (and (<> :os macos) (<> :os win32)))
  (conf-wayland-protocols (and (<> :os macos) (<> :os win32))))

 (synopsis "The OCaml-Canvas library")
 (description "\
The OCaml-Canvas library provides a portable Canvas for OCaml.
"))
