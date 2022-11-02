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
  mutable frog_opt : [`Offscreen] Canvas.t option;
  mutable frames : Int64.t
}

let state = {
  frog_opt = None;
  frames = 0L;
}

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world" ~pos:(300, 200) ~size:(300, 200) in

  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(300.0, 200.0);

  Canvas.setStrokeColor c Color.cyan;
  Canvas.setLineWidth c 10.0;
  Canvas.clearPath c;
  Canvas.moveTo c (5.0, 5.0);
  Canvas.lineTo c (295.0, 5.0);
  Canvas.lineTo c (295.0, 195.0);
  Canvas.lineTo c (5.0, 195.0);
  Canvas.closePath c;
  Canvas.stroke c;

  Canvas.setFont c "Liberation Sans" ~size:36.0
    ~slant:Font.Roman ~weight:Font.bold;

  Canvas.setFillColor c (Color.of_rgb 0 64 255);
  Canvas.setLineWidth c 1.0;
  Canvas.save c;
  Canvas.translate c (150.0, 100.0);
  Canvas.rotate c (-. Const.pi_8);
  Canvas.fillText c "Hello world !" (-130.0, 20.0);
  Canvas.restore c;

  Canvas.show c;

  let c2 = Canvas.createOffscreen ~size:(20, 20) in
  Canvas.setFillColor c2 Color.blue;
  Canvas.fillRect c2 ~pos:(0.0, 0.0) ~size:(20.0, 20.0);
  Canvas.save c;
  Canvas.rotate c 0.3;
  Canvas.scale c (5.0, 5.0);
  Canvas.blit ~dst:c ~dpos:(10, 0) ~src:c2 ~spos:(0, 0) ~size:(15, 15);
  Canvas.restore c;

  let event_frog = Canvas.createOffscreenFromPNG "assets/frog.png" in
  retain_event @@
    React.E.map (fun frog ->
        state.frog_opt <- Some (frog);
        let size = Canvas.getSize frog in
        Canvas.save c;
        Canvas.setTransform c Transform.id;
        Canvas.scale c (0.25, 0.25);
        Canvas.blit ~dst:c ~dpos:(750, 400) ~src:frog ~spos:(0, 0) ~size;
        Canvas.restore c
      ) event_frog;

  retain_event @@
    React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.key; char = _; flags = _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.canvas = c; timestamp = _;
                       data = { Event.position = (x, y); button } } ->
        match button, state.frog_opt with
        | ButtonRight, Some (frog) ->
            let size = Canvas.getSize frog in
            let w, h = size in
            Canvas.save c;
            Canvas.setTransform c Transform.id;
            Canvas.translate c (float_of_int x, float_of_int y);
            Canvas.scale c (0.25, 0.25);
            Canvas.translate c (-0.5 *. float_of_int w, -0.5 *. float_of_int h);
            Canvas.blit ~dst:c ~dpos:(0, 0) ~src:frog ~spos:(0, 0) ~size;
            Canvas.restore c
        | ButtonLeft, _ ->
            Canvas.setFillColor c Color.red;
            Canvas.clearPath c;
            Canvas.arc c ~center:(float_of_int x, float_of_int y)
              ~radius:5.0 ~theta1:0.0 ~theta2:(2.0 *. Const.pi) ~ccw:false;
            Canvas.fill c ~nonzero:false;
        | _ ->
            ()
      ) Event.button_down;

  retain_event @@
    React.E.map (fun { Event.canvas = _; timestamp = _ } ->
        state.frames <- Int64.add state.frames Int64.one
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Displayed %Ld frames. Goodbye !\n" state.frames)
