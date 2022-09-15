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

type Event.payload += CanvasBlit of [`Offscreen] Canvas.t * [`Onscreen] Canvas.t * (int * int)

let pi = acos(-1.)

let () =

  Backend.(init default_options);

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(300, 200) in

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
  Canvas.rotate c (-.pi /. 8.0);
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

  let p_c3 = Canvas.createOffscreenFromPNG "assets/frog.png" in
  ignore @@
    Promise.bind p_c3 (fun c3 ->
        let size = Canvas.getSize c3 in
        Canvas.save c;
        Canvas.setTransform c Transform.id;
        Canvas.scale c (0.25, 0.25);
        Canvas.blit ~dst:c ~dpos:(750, 400) ~src:c3 ~spos:(0, 0) ~size;
        Canvas.restore c;
        Promise.return ());

  Backend.run (fun state -> function

    | Event.Custom { timestamp = _; payload = CanvasBlit (c', c, (x, y)) } ->
        let size = Canvas.getSize c' in
        Canvas.save c;
        Canvas.setTransform c Transform.id;
        let (w, h) = Canvas.getSize c' in
        Canvas.translate c (float_of_int x, float_of_int y);
        Canvas.scale c (0.25, 0.25);
        Canvas.translate c (-. 0.5 *. float_of_int w, - 0.5 *. float_of_int h);
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:c' ~spos:(0, 0) ~size;
        Canvas.restore c;
        state, true

    | Event.ButtonAction { canvas = _c; timestamp = _; position = (x, y);
                           button = ButtonRight; state = Down } ->
        let p_c = Canvas.createOffscreenFromPNG "assets/frog.png" in
        ignore @@
          Promise.bind p_c (fun c' ->
              Backend.sendCustomEvent (CanvasBlit (c', c, (x, y)));
              Promise.return ());
        state, true

    | Event.ButtonAction { canvas = c; timestamp = _;
                           position = (x, y); button = _; state = Down } ->
        Canvas.setFillColor c Color.red;
        Canvas.clearPath c;
        Canvas.arc c ~center:(float_of_int x, float_of_int y)
          ~radius:5.0 ~theta1:0.0 ~theta2:(pi *. 2.0) ~ccw:false;
        Canvas.fill c ~nonzero:false;
        state, true

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        state, true

    | Event.CanvasClosed { canvas = _; timestamp = _ } ->
        Backend.stop ();
        state, true

    | Event.Frame { canvas = _; timestamp = _ } ->
        Int64.add state Int64.one, true

    | _ ->
        state, false

    ) (function state ->
         Printf.printf "Displayed %Ld frames. Goodbye !\n" state
    ) 0L
