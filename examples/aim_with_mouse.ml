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

let events = ref []

let retain_event e =
  events := e :: !events

let clear_events () =
  events := []

let state = ref (0.0, 0.0, 0.0, 0.0)

let () =

  Backend.init ();

  let c = Canvas.createFramed "Aim with mouse"
            ~pos:(300, 50) ~size:(800, 600) in

  Canvas.show c;

  Canvas.setShadowBlur c 2.;
  Canvas.setShadowColor c (Color.of_argb 128 0 0 0);
  Canvas.setShadowOffset c (2., 2.);

  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 600.0);

  let a = Array.create_float 2 in
  Array.set a 0 20.0;
  Array.set a 1 20.0;
  Canvas.setLineDash c (a);

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.data = (x, y); _ } ->
        let _a, _b, _tan_alpha, old_t = !state in
        let a = float_of_int x in
        let b = float_of_int y in
        let g = 100. and v = 600. in
        let t = -. (g *. a *. a) /. (v *. v) in
        let tan_alpha =
          (-. a +.
             sqrt (a *. a -. 4.0 *. t *. (b +. t -. 600.0))) /.
            (2.0 *. t) in
        state := (a, b, tan_alpha, old_t)
      ) Event.mouse_move;

  retain_event @@
    React.E.map (fun _ ->
        let a, b, tan_alpha, t = !state in
        let t = t -. 1. in
        Canvas.setLineDashOffset c t;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 600.0);
        Canvas.clearPath c;
        Canvas.moveTo c (0.0, 600.0);
        let alpha = atan tan_alpha in
        let cos_a = cos alpha and sin_a = sin alpha in
        let n = 10 in
        for i = 1 to n do
          let x_1 = (float_of_int i) *. a /. (float_of_int n)
          and g = 100. and v = 600. in
          let y_1 =
            (-. sin_a) *. (x_1 /. cos_a) +.
              g *. (x_1 *. x_1) /. (cos_a *. cos_a *. v *. v) +. 600.0 in
          Canvas.lineTo c (x_1, y_1)
        done;
        Canvas.stroke c;
        Canvas.clearPath c;
        state := (a, b, tan_alpha, t)
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
