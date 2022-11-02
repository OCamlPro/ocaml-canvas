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

type state = {
  mutable pos : Point.t;
  mutable vec : float * float;
  mutable m_pos : Point.t;
  mutable pressing : bool;
}

let state = {
  pos = (260.0, 100.0);
  vec = (0.0, 0.0);
  m_pos = (0.0, 0.0);
  pressing = false
}

let () =

  Backend.init ();

  let width = 520 in
  let height = 200 in
  let w = float_of_int width in
  let h = float_of_int height in

  let c = Canvas.createFramed "Compositions"
            ~pos:(300, 200) ~size:(width, height) in
  Canvas.moveTo c (0.0, 0.0);
  Canvas.lineTo c (520.0, 0.0);
  Canvas.lineTo c (520.0, 200.0);
  Canvas.lineTo c (0.0, 200.0);
  Canvas.closePath c;
  Canvas.setLineWidth c 10.0;
  Canvas.stroke c;

  Canvas.show c;

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
    React.E.map (fun { Event.data = position; _ } ->
        state.m_pos <- Point.of_ints position
      ) Event.mouse_move;

  retain_event @@
    React.E.map (fun _ ->
        state.pressing <- true
      ) Event.button_down;

  retain_event @@
    React.E.map (fun _ ->
        state.pressing <- false
      ) Event.button_up;

  retain_event @@
    React.E.map (fun _ ->
        let dt = 0.033 and r = 30.0 and damp = 0.99 in
        let dir = Point.sub state.m_pos state.pos in
        let force =
          if state.pressing then Vector.mul dir 4.0
          else (0.0, 0.0)
        in
        let accel = Vector.add force (0.0, 109.8) in
        let vec = Vector.mul state.vec damp in
        let vec = Vector.add vec (Vector.mul accel dt) in

        let vx, vy = vec in
        let x, y = Point.translate state.pos ~by:(Vector.mul vec dt) in
        let x, vx =
          if x > w -. r then w -. r, min (0.0 -. vx) 0.0
          else if x < 0.0 +. r then 0. +. r, max (0.0 -. vx) 0.0
          else x, vx
        in
        let y, vy =
          if y > h -. r then h -. r, min (0.0 -. vy) 0.0
          else if y < 0.0 +. r then 0. +. r, max (0.0 -. vy) 0.0
          else y, vy
        in
        let pos = (x, y) in
        let vec = (vx, vy) in

        let mv = Point.sub pos state.pos in

        Canvas.setShadowBlur c 0.0;
        Canvas.setShadowColor c Color.transpBlack;
        Canvas.setShadowOffset c (0.0, 0.0);
        Canvas.setFillColor c Color.white;
        Canvas.fillRect c ~pos:(5.0, 5.0) ~size:(510.0, 190.0);
        Canvas.setFillColor c Color.black;
        Canvas.clearPath c;

        let speed = Vector.norm mv in
        let blur = 10.0 *. (exp (speed /. 15.0) /.
                              (1.0 +. exp (speed /. 15.0))) -. 5.0 in
        Canvas.setShadowBlur c blur;
        Canvas.setShadowColor c (Color.of_argb 64 0 0 0);
        Canvas.setShadowOffset c (Vector.mul mv (-. 10.0 *. blur /. speed));
        Canvas.arc c ~center:pos ~radius:r
          ~theta1:0.0 ~theta2:(2.0 *. Const.pi) ~ccw:false;
        Canvas.fill c ~nonzero:true;

        state.pos <- pos;
        state.vec <- vec
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
