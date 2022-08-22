
(rule
  (target index.html)
  (deps (glob_files *.js))
  (action (run ./make_index/make_index.exe %{target})))


(executable
 (name hello)
 (public_name ocaml-canvas-hello)
 (modes byte_complete native js)
 (modules hello)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files hello-extfs.js)))

(rule
 (target hello-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          frog.png:/static/assets/)))


(executable
 (name ppm_dump)
 (public_name ocaml-canvas-ppm_dump)
 (modes byte_complete native js)
 (modules ppm_dump)
 (libraries ocaml-canvas))


(executable
 (name arcs)
 (public_name ocaml-canvas-arcs)
 (modes byte_complete native js)
 (modules arcs)
 (libraries ocaml-canvas))


(executable
 (name gradients)
 (public_name ocaml-canvas-gradients)
 (modes byte_complete native js)
 (modules gradients)
 (libraries ocaml-canvas))


(executable
 (name snake)
 (public_name ocaml-canvas-snake)
 (modes byte_complete native js)
 (modules snake)
 (libraries ocaml-canvas))


(executable
 (name thicklines)
 (public_name ocaml-canvas-thicklines)
 (modes byte_complete native js)
 (modules thicklines)
 (libraries ocaml-canvas))


(executable
 (name dashing_line)
 (public_name ocaml-canvas-dashing_line)
 (modes byte_complete native js)
 (modules dashing_line)
 (libraries ocaml-canvas))


(executable
 (name compositions)
 (public_name ocaml-canvas-compositions)
 (modes byte_complete native js)
 (modules compositions)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files compositions-extfs.js)))

(rule
 (target compositions-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          fabric.png:/static/assets/
          dragon.png:/static/assets/)))


(executable
 (name aim_with_mouse)
 (public_name ocaml-canvas-aim_with_mouse)
 (modes byte_complete native js)
 (modules aim_with_mouse)
 (libraries ocaml-canvas))


(executable
 (name clipping)
 (public_name ocaml-canvas-clipping)
 (modes byte_complete native js)
 (modules clipping)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files clipping-extfs.js)))

(rule
 (target clipping-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          dragon.png:/static/assets/)))


(executable
 (name hexagon_grid)
 (public_name ocaml-canvas-hexagon_grid)
 (modes byte_complete native js)
 (modules hexagon_grid)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files hexagon_grid-extfs.js)))

(rule
 (target hexagon_grid-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          hexagon.png:/static/assets/)))


(executable
 (name ball)
 (public_name ocaml-canvas-ball)
 (modes byte_complete native js)
 (modules ball)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files ball-extfs.js)))

(rule
 (target ball-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          dragon.png:/static/assets/)))


(executable
 (name spritesheet)
 (public_name ocaml-canvas-spritesheet)
 (modes byte_complete native js)
 (modules spritesheet)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files spritesheet-extfs.js)))

(rule
 (target spritesheet-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          spritesheet.png:/static/assets/)))


(executable
 (name draw)
 (public_name ocaml-canvas-draw)
 (modes byte_complete native js)
 (modules draw)
 (libraries ocaml-canvas)
 (js_of_ocaml (javascript_files draw-extfs.js)))

(rule
 (target draw-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          colors.png:/static/assets/)))


(executable
 (name window_with_textbox)
 (public_name ocaml-canvas-window_with_textbox)
 (modes byte_complete native js)
 (modules window_with_textbox)
 (libraries ocaml-canvas))

