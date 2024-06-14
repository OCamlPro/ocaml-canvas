
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

let add_executable name assets =
  Printf.printf {|
(executable
 (name %s)
 (public_name ocaml-canvas-%s)
 (modes byte_complete native%s)
 (modules %s)
 (libraries ocaml-canvas react)|} name name (if has_jsoo then " js" else "") name;
  if has_jsoo && assets <> [] then
    Printf.printf {|
 (js_of_ocaml
  (javascript_files %s-extfs.js)))

(rule
 (target %s-extfs.js)
 (action (run js_of_ocaml build-fs -o %%{target} -I ../assets%t)))

|} name name (fun oc ->
        List.iter (fun a ->
            Printf.printf {|
          %s:/static/assets/|} a) assets)
  else
    Printf.printf {|)

|}

let () =
  set_binary_mode_out stdout true;
  if has_jsoo then
    Printf.printf {|
(rule
  (target index.html)
  (deps (glob_files *.js))
  (action (run ./make_index/make_index.exe %%{target})))

|};
  add_executable "hello" [ "frog.png" ];
  add_executable "ppm_dump" [];
  add_executable "arcs" [];
  add_executable "gradients" [];
  add_executable "snake" [];
  add_executable "thicklines" [];
  add_executable "dashing_line" [];
  add_executable "compositions" [ "fabric.png"; "dragon.png" ];
  add_executable "aim_with_mouse" [];
  add_executable "clipping" [ "dragon.png" ];
  add_executable "hexagon_grid" [ "hexagon.png" ];
  add_executable "ball" [ "dragon.png" ];
  add_executable "spritesheet" [ "spritesheet.png" ];
  add_executable "draw" [ "colors.png" ];
  add_executable "window_with_textbox" [];
  add_executable "suncities" [];

  add_executable "demo1" [];
  add_executable "demo2" [];
  add_executable "demo3" [];
  add_executable "demo4" [];
  add_executable "demo5" [];
  add_executable "demo6" [];
  add_executable "saucisse" [];
