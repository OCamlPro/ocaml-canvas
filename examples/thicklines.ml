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

  let c = Canvas.createFramed "Thick lines"
            ~pos:(300, 50) ~size:(400, 650) in

  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(400.0, 650.0);

  Canvas.setStrokeColor c Color.black;
  Canvas.setLineWidth c 2.0;
  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 0.0);
  Canvas.lineTo c (100.0, 650.0);
  Canvas.stroke c;
  Canvas.clearPath c;
  Canvas.moveTo c (300.0, 0.0);
  Canvas.lineTo c (300.0, 650.0);
  Canvas.stroke c;
  Canvas.show c;
  Canvas.clearPath c;

  Canvas.setLineWidth c 10.0;

  Canvas.setLineCap c Cap.Butt;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 50.0);
  Canvas.lineTo c (300.0, 50.0);
  Canvas.stroke c;

  Canvas.setLineCap c Cap.Round;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 100.0);
  Canvas.lineTo c (300.0, 100.0);
  Canvas.stroke c;

  Canvas.setLineCap c Cap.Square;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 150.0);
  Canvas.lineTo c (300.0, 150.0);
  Canvas.stroke c;

  Canvas.setLineCap c Cap.Round;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 200.0);
  Canvas.lineTo c (300.0, 200.0);
  Canvas.setTransform c (Transform.create (1.0, 0.0, 1.0, 1.0, 0.0, 0.0));
  Canvas.stroke c;

  Canvas.setLineCap c Cap.Round;
  Canvas.setTransform c Transform.id;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 250.0);
  Canvas.lineTo c (300.0, 250.0);
  Canvas.setTransform c (Transform.create (1.0, 0.0, -1.0, 1.0, 0.0, 0.0));
  Canvas.stroke c;



  Canvas.setLineWidth c 20.0;
  Canvas.setLineCap c Cap.Butt;
  Canvas.setTransform c Transform.id;
  Canvas.setLineJoin c Join.Round;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 325.0);
  Canvas.lineTo c (200.0, 275.0);
  Canvas.lineTo c (300.0, 320.0);
  Canvas.stroke c;


  Canvas.setLineCap c Cap.Butt;
  Canvas.setTransform c Transform.id;
  Canvas.setLineJoin c Join.Bevel;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 375.0);
  Canvas.lineTo c (200.0, 325.0);
  Canvas.lineTo c (300.0, 375.0);
  Canvas.stroke c;

  Canvas.setLineCap c Cap.Butt;
  Canvas.setTransform c Transform.id;
  Canvas.setLineJoin c Join.Round;

  Canvas.clearPath c;
  Canvas.moveTo c (200.0, 375.0);
  Canvas.lineTo c (100.0, 425.0);
  Canvas.lineTo c (300.0, 425.0);
  Canvas.closePath c;
  Canvas.stroke c;

  Canvas.setTransform c Transform.id;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 505.0);
  Canvas.lineTo c (200.0, 455.0);
  Canvas.lineTo c (300.0, 505.0);
  Canvas.closePath c;
  Canvas.setTransform c (Transform.create (1.0, 0.0, -1.0, 1.0, 0.0, 0.0));
  Canvas.stroke c;

  let ex = Path.create () in
  Canvas.clearPath c;
  Path.moveTo ex (300.0, 595.0);
  Path.lineTo ex (200.0, 535.0);
  Path.lineTo ex (100.0, 595.0);
  Path.close ex;
  Canvas.setTransform c (Transform.create (1.0, 0.0, 0.1, 1.0, 0.0, 0.0));
  Canvas.strokePath c ex;

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
