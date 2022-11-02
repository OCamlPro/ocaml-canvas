
let has_jsoo =
  try
    let out, _, _ =
      Unix.open_process_full "js_of_ocaml --version" (Unix.environment ()) in
    let ver = input_line out in
    let i1 = String.index_from ver 0 '.' in
    let i2 = String.index_from ver (i1+1) '.' in
    let maj = int_of_string (String.sub ver 0 i1) in
    let min = int_of_string (String.sub ver (i1+1) (i2-i1-1)) in
    maj > 3 || maj = 3 && min >= 6
  with _ -> false

let () =
  set_binary_mode_out stdout true;
  Printf.printf {|
(library
 (name ocaml_canvas)
 (public_name ocaml-canvas)
 (synopsis "Portable OCaml Canvas library")
 (wrapped false)
 (libraries bigarray react)
 (modules ocamlCanvas)
 (foreign_stubs
  (language c)
  (names config util unicode point rect list hashtable event
         gdi_keyboard gdi_backend gdi_target gdi_window gdi_surface
         qtz_keyboard qtz_backend qtz_target qtz_window qtz_surface
         x11_keysym x11_keyboard x11_backend x11_target x11_window x11_surface
         wl_backend wl_target wl_window wl_surface xdg-shell-protocol
         window pixmap image_interpolation filters surface transform draw_instr
         font_desc gdi_font qtz_font unx_font font
         gdi_impexp qtz_impexp unx_impexp impexp
         path arc path2d polygon polygonize
         gradient pattern draw_style color_composition poly_render
         state canvas backend
         ml_convert ml_canvas)
  (flags (:standard) (:include ccopt.sexp)))
 (c_library_flags (:standard) (:include cclib.sexp))|};
  if has_jsoo then
    Printf.printf {|
 (js_of_ocaml
  (javascript_files implem/event.js implem/translate.js
                    stubs/ml_tags.js stubs/ml_convert.js stubs/ml_canvas.js)))

|}
  else
    Printf.printf {|)

|}
