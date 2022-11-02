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

let () =

  Backend.init ();

  let c = Canvas.createFramed "Clipping"
            ~pos:(300, 200) ~size:(300, 300) in
  Canvas.save c;
  Canvas.scale c (2.0, 2.0);
  Canvas.rotate c 0.3;

  let square = Path.create () in
  Path.rect square ~pos:(50.0, 50.0) ~size:(50.0, 50.0);
  Canvas.clipPath c square ~nonzero:false;
  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(300.0, 300.0);
  Canvas.show c;
  Canvas.restore c;
  Canvas.setFillColor c Color.red;
  Canvas.rect c ~pos:(150.0, 20.0) ~size:(75.0, 75.0);
  Canvas.scale c (2.0, 2.0);
  Canvas.rotate c 0.3;
  Canvas.rect c ~pos:(50.0, 50.0) ~size:(50.0, 50.0);
  Canvas.clip c ~nonzero:false;
  Canvas.save c;
  Canvas.restore c;

  let event_dragonImage = ImageData.createFromPNG "assets/dragon.png" in
  retain_event @@
    React.E.map (fun dragonImage ->
        let pattern = Canvas.createPattern c dragonImage RepeatXY in
        Canvas.setFillPattern c pattern;
        Canvas.scale c (0.1, 0.1);
        Canvas.rotate c (0.0 -. 0.3);
        Canvas.save c;
        Canvas.save c;
        Canvas.restore c;
        Canvas.restore c;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(3000.0, 3000.0)
      ) event_dragonImage;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
