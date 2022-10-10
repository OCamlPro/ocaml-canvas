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

let interpInt x1 x2 t =
  int_of_float ((1.0 -. t) *. (float_of_int x1) +. t *. (float_of_int x2))

let interpColor c1 c2 t =
  let r1, g1, b1 = Color.to_rgb c1 in
  let r2, g2, b2 = Color.to_rgb c2 in
  Color.of_rgb (interpInt r1 r2 t) (interpInt g1 g2 t) (interpInt b1 b2 t)

let hsv_to_rgb h s v =
  let c = v *. s in
  let m = v -. c in
  let x = c *. (1.0 -. abs_float (((mod_float (h /. 60.0) 2.0) -. 1.0))) in
  let r, g, b = match h with
    | a when a < 60.0 -> c, x , 0.0
    | a when a < 120.0 -> x, c, 0.0
    | a when a < 180.0 -> 0.0, c, x
    | a when a < 240.0 -> 0.0, x, c
    | a when a < 300.0 -> x, 0.0, c
    | _ -> c, 0.0, x
  in
  Color.of_rgb
    (int_of_float ((r +. m) *. 255.0))
    (int_of_float ((g +. m) *. 255.0))
    (int_of_float ((b +. m) *. 255.0))

let () =

  Backend.init ();

  let c =
    Canvas.createFramed "test" ~pos:(960 - 640, 540 - 360) ~size:(1280, 720) in
  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(1280.0, 720.0);
  Canvas.show c;

  Backend.run (fun state -> function

      | Event.KeyAction { canvas = _; timestamp = _;
                          key; char = _; flags = _; state = Down } ->
          if key = Event.KeyEscape then
            Backend.stop ();
          state, true

      | Event.CanvasClosed { canvas = _; timestamp = _ } ->
          Backend.stop ();
          state, true

      | Frame { canvas = c; timestamp = _ } ->
          let state = state +. 1. /. 60. in
          Canvas.setFillColor c (hsv_to_rgb (state *. 36.0)  1.0 1.0);
          Canvas.fillRect c ~pos:(128.0 *. state, 0.0) ~size:(128.0, 360.0);
          Canvas.setFillColor c
            (interpColor Color.black Color.white (state *. 0.1));
          Canvas.fillRect c ~pos:(128.0 *. state, 360.0) ~size:(128.0, 360.0);
          state, true

      | _ ->
          state, false

    ) (function _state ->
      Printf.printf "Goodbye !\n"
    ) (-1.0)
