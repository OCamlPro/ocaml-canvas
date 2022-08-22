(**************************************************************************)
(*                                                                        *)
(*    Copyright 2022 OCamlPro                                             *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

open OcamlCanvas

let pi = acos(-1.)

let () =

  Backend.(init default_options);
  let width = 520 and height = 200 in
  let w = float_of_int width and h = float_of_int height in
  let c = Canvas.createFramed "Compositions"
            ~pos:(300, 200) ~size:(width, height) in
  Canvas.moveTo c (0., 0.);
  Canvas.lineTo c (520., 0.);
  Canvas.lineTo c (520., 200.);
  Canvas.lineTo c (0., 200.);
  Canvas.closePath c;
  Canvas.setLineWidth c 10.;
  Canvas.stroke c;
  Canvas.show c;
  
  let x = ref 260. and y = ref 100. in
  let old_x = ref 260. and old_y = ref 260. in
  let vx = ref 0. and vy = ref 0. in
  let ax = ref 0. and ay = ref 0. in
  let fx = ref 0. and fy = ref 0. in
  let m_pos_x = ref 0. and m_pos_y = ref 0. in
  let pressing = ref false in

  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true
    
    | Event.MouseMove { canvas = _; timestamp = _;
                        position = (x,y)} ->
        m_pos_x := float_of_int x;
        m_pos_y := float_of_int y;
        true;
    
    | Event.ButtonAction {canvas = _; timestamp = _;
                          position = _;
                          button = _;
                          state = s;} -> 
        if (s = Down) then (pressing := true;) else (pressing := false;);
        true

    | Event.Frame { canvas = _; timestamp = _ } ->
        let dt = 0.033 and r = 30. and damp = 0.99 in
        let dx = !m_pos_x -. !x and 
            dy = !m_pos_y -. !y in
        if (!pressing) then (fx := 4. *. dx; fy := 4. *. dy;)
                       else (fx := 0.; fy := 0.;);
        ay := 109.8 +. !fy;
        ax := !fx;
        vx := !vx *. damp;
        vy := !vy *. damp;
        vx := !vx +. !ax *. dt;
        vy := !vy +. !ay *. dt;
        x := !x +. !vx *. dt;
        y := !y +. !vy *. dt;
        if (!y > h -. r) then (y := h -. r; vy := Float.min (0. -. !vy) (0.));
        if (!y < 0. +. r) then (y := 0. +. r; vy := Float.max (0. -. !vy) (0.));
        if (!x > w -. r) then (x := w -. r; vx := Float.min (0. -. !vx) (0.));
        if (!x < 0. +. r) then (x := 0. +. r; vx := Float.max (0. -. !vx) (0.));
        let mv_x = !x -. !old_x and mv_y = !y -. !old_y in
        old_x := !x;
        old_y := !y;
        Canvas.setShadowBlur c 0.;
        Canvas.setShadowColor c (Color.of_argb 0 0 0 0);
        Canvas.setShadowOffset c (0., 0.);
        Canvas.setFillColor c Color.white;
        Canvas.fillRect c ~pos:(5., 5.) ~size:(510., 190.);
        Canvas.setFillColor c Color.black;
        Canvas.clearPath c;
        let speed = Float.sqrt (mv_x *. mv_x +. mv_y *. mv_y) in
        let blur = 10. *. (Float.exp (speed /. 15.) /. (1. +. Float.exp (speed /. 15.))) -. 5. in
        Canvas.setShadowBlur c blur;
        Canvas.setShadowColor c (Color.of_argb 64 0 0 0);
        Canvas.setShadowOffset c (0. -. 10. *. (mv_x /. speed) *. blur, 0. -. 10. *. (mv_y /. speed) *. blur);
        Canvas.arc_ c (!x, !y) r 0. (2. *. pi) false;
        Canvas.fill c ~nonzero:true;
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
