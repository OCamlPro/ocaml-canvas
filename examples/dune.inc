
(executable
 (name hello)
 (public_name ocaml-canvas-hello)
 (modes byte_complete native js)
 (modules hello)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files hello-extfs.js)))

(rule
 (target hello-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          frog.png:/static/assets/)))

(executable
 (name ppm_dump)
 (public_name ocaml-canvas-ppm_dump)
 (modes byte_complete native js)
 (modules ppm_dump)
 (libraries ocaml-canvas react))


(executable
 (name arcs)
 (public_name ocaml-canvas-arcs)
 (modes byte_complete native js)
 (modules arcs)
 (libraries ocaml-canvas react))


(executable
 (name gradients)
 (public_name ocaml-canvas-gradients)
 (modes byte_complete native js)
 (modules gradients)
 (libraries ocaml-canvas react))


(executable
 (name snake)
 (public_name ocaml-canvas-snake)
 (modes byte_complete native js)
 (modules snake)
 (libraries ocaml-canvas react))


(executable
 (name thicklines)
 (public_name ocaml-canvas-thicklines)
 (modes byte_complete native js)
 (modules thicklines)
 (libraries ocaml-canvas react))


(executable
 (name dashing_line)
 (public_name ocaml-canvas-dashing_line)
 (modes byte_complete native js)
 (modules dashing_line)
 (libraries ocaml-canvas react))


(executable
 (name compositions)
 (public_name ocaml-canvas-compositions)
 (modes byte_complete native js)
 (modules compositions)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files compositions-extfs.js)))

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
 (libraries ocaml-canvas react))


(executable
 (name clipping)
 (public_name ocaml-canvas-clipping)
 (modes byte_complete native js)
 (modules clipping)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files clipping-extfs.js)))

(rule
 (target clipping-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          dragon.png:/static/assets/)))

(executable
 (name hexagon_grid)
 (public_name ocaml-canvas-hexagon_grid)
 (modes byte_complete native js)
 (modules hexagon_grid)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files hexagon_grid-extfs.js)))

(rule
 (target hexagon_grid-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          hexagon.png:/static/assets/)))

(executable
 (name ball)
 (public_name ocaml-canvas-ball)
 (modes byte_complete native js)
 (modules ball)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files ball-extfs.js)))

(rule
 (target ball-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          dragon.png:/static/assets/)))

(executable
 (name spritesheet)
 (public_name ocaml-canvas-spritesheet)
 (modes byte_complete native js)
 (modules spritesheet)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files spritesheet-extfs.js)))

(rule
 (target spritesheet-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          spritesheet.png:/static/assets/)))

(executable
 (name draw)
 (public_name ocaml-canvas-draw)
 (modes byte_complete native js)
 (modules draw)
 (libraries ocaml-canvas react)
 (js_of_ocaml
  (javascript_files draw-extfs.js)))

(rule
 (target draw-extfs.js)
 (action (run js_of_ocaml build-fs -o %{target} -I ../assets
          colors.png:/static/assets/)))

(executable
 (name window_with_textbox)
 (public_name ocaml-canvas-window_with_textbox)
 (modes byte_complete native js)
 (modules window_with_textbox)
 (libraries ocaml-canvas react))


(executable
 (name suncities)
 (public_name ocaml-canvas-suncities)
 (modes byte_complete native js)
 (modules suncities)
 (libraries ocaml-canvas react))


(executable
 (name demo1)
 (public_name ocaml-canvas-demo1)
 (modes byte_complete native js)
 (modules demo1)
 (libraries ocaml-canvas react))


(executable
 (name demo2)
 (public_name ocaml-canvas-demo2)
 (modes byte_complete native js)
 (modules demo2)
 (libraries ocaml-canvas react))


(executable
 (name demo3)
 (public_name ocaml-canvas-demo3)
 (modes byte_complete native js)
 (modules demo3)
 (libraries ocaml-canvas react))


(executable
 (name demo4)
 (public_name ocaml-canvas-demo4)
 (modes byte_complete native js)
 (modules demo4)
 (libraries ocaml-canvas react))


(executable
 (name demo5)
 (public_name ocaml-canvas-demo5)
 (modes byte_complete native js)
 (modules demo5)
 (libraries ocaml-canvas react))


(executable
 (name demo6)
 (public_name ocaml-canvas-demo6)
 (modes byte_complete native js)
 (modules demo6)
 (libraries ocaml-canvas react))


(executable
 (name saucisse)
 (public_name ocaml-canvas-saucisse)
 (modes byte_complete native js)
 (modules saucisse)
 (libraries ocaml-canvas react))


(rule
  (targets index.html saucisse.html demo6.html demo5.html demo4.html demo3.html demo2.html demo1.html window_with_textbox.html draw.html spritesheet.html ball.html hexagon_grid.html clipping.html aim_with_mouse.html compositions.html dashing_line.html thicklines.html snake.html gradients.html arcs.html ppm_dump.html hello.html)
  (deps (glob_files *.js))
  (action (run ./make_index/make_index.exe %{targets})))

