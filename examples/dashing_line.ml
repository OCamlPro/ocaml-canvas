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

let state = ref 0.0

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Dashing line"
            ~pos:(300, 50) ~size:(400, 650) () in

  Canvas.show c;

  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(400.0, 650.0);

  Canvas.setLineDash c [| 20.0; 20.0 |];
  Canvas.setLineDashOffset c 781.0;
  let b = Canvas.getLineDash c in
  Array.set b 0 40.0;
  Canvas.setLineDash c (b);

  Canvas.setStrokeColor c Color.black;
  Canvas.setLineWidth c 2.0;
  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 0.0);
  Canvas.lineTo c (100.0, 650.0);
  Canvas.stroke c;
  Canvas.clearPath c;
  Canvas.moveTo c (300.0, 100.0);
  Canvas.lineTo c (300.0, 400.0);
  Canvas.stroke c;

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
    React.E.map (fun _ ->
        state := !state +. 1.0;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(400.0, 650.0);
        Canvas.setLineDashOffset c !state;
        Canvas.stroke c
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
