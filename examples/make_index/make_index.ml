
let gen_sample_index file =
  let filename = file ^ ".html" in
  if Sys.file_exists filename then () else
    let tl = open_in "sample_template.html" in
    let out = open_out filename in
    try
      while true do
        let l = input_line tl in
        let l' = Str.global_replace (Str.regexp_string "%%FILE%%") file l in
        Printf.fprintf out "%s\n" l'
      done;
    with End_of_file ->
      close_in tl;
      close_out out

let gen_global_index file files =
  let tl = open_in "global_template.html" in
  let out = open_out file in
  let samples = Buffer.create 1024 in
  List.iter (fun file ->
      Printf.bprintf samples "<li><a href=\"%s.html\">%s</a></li>\n" file file
    ) files;
  let samples = Buffer.contents samples in
  try
    while true do
      let l = input_line tl in
      let l' = Str.global_replace (Str.regexp_string "%%SAMPLES%%") samples l in
      Printf.fprintf out "%s\n" l'
    done;
  with End_of_file ->
    close_in tl;
    close_out out

let () =
  let files = Array.to_list (Sys.readdir "./") in
  let js_files =
    List.fold_left (fun js_files file ->
        try
(*
          let pos = String.rindex file '.' in
          let ext = String.sub file pos (String.length file - pos) in
          if not (String.equal ext ".ml") then ml_files
          else String.sub file 0 pos :: ml_files
*)
          let ext = String.sub file (String.length file - 6) 6 in
          if not (String.equal ext ".bc.js") then js_files
          else String.sub file 0 (String.length file - 6) :: js_files
        with _ ->
          js_files
      ) [] files |> List.fast_sort String.compare
  in
  List.iter gen_sample_index js_files;
  gen_global_index Sys.argv.(1) js_files
