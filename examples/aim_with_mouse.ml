(**************************************************************************)
(*                                                                        *)
(*    Copyright 2022 OCamlPro                                             *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

open OcamlCanvas.V1

let pi = acos(-1.)

let () =

  Backend.(init default_options);

  let c = Canvas.createFramed "Aim with mouse"
            ~pos:(300, 50) ~size:(800, 600) in

  Canvas.setShadowBlur c 2.;
  Canvas.setShadowColor c (Color.of_argb 128 0 0 0);
  Canvas.setShadowOffset c (2., 2.);

  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 600.0);

  let a = Array.create_float 2 in
  Array.set a 0 20.0;
  Array.set a 1 20.0;
  Canvas.setLineDash c (a);

  Canvas.show c;
  let a = ref 0. and b = ref 0. and tan_alpha = ref 0. and t = ref 0.  in
  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true

    | Event.MouseMove { canvas = _; timestamp = _;
                        position = (x,y)} ->
        a := float_of_int x;
        b := float_of_int y;
        let g = 100. and v = 600. in
        let t = 0. -. (g *. !a *. !a) /. (v *. v) in
        tan_alpha :=
          ((0. -. !a) +.
             Float.sqrt (!a *. !a -. 4. *. t *. (!b +. t -. 600.))) /.
            (2. *. t);
        true;

    | Event.Frame { canvas = _; timestamp = _ } ->
        t := !t +. 1.;
        Canvas.setLineDashOffset c !t;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 600.0);
        Canvas.clearPath c;
        Canvas.moveTo c (0.,600.);
        let alpha = Float.atan(!tan_alpha) in
        let cos_a = Float.cos(alpha) and sin_a = Float.sin(alpha) in
        let n = 10 in
        for i = 1 to n do
          let x_1 = (float_of_int i) *. !a /. (float_of_int n)
          and g = 100.
          and v = 600. in
          let y_1 =
            (0. -. sin_a) *. (x_1 /. cos_a) +.
              g *. (x_1 *. x_1) /. (cos_a *. cos_a *. v *. v) +. 600. in
          Canvas.lineTo c (x_1, y_1);
        done;
        Canvas.stroke c;
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
