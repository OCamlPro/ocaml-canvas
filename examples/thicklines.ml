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

  let c = Canvas.createFramed "Hello world"
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

  Canvas.setLineCap c Canvas.Butt;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 50.0);
  Canvas.lineTo c (300.0, 50.0);
  Canvas.stroke c;

  Canvas.setLineCap c Canvas.RoundCap;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 100.0);
  Canvas.lineTo c (300.0, 100.0);
  Canvas.stroke c;

  Canvas.setLineCap c Canvas.Square;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 150.0);
  Canvas.lineTo c (300.0, 150.0);
  Canvas.stroke c;

  Canvas.setLineCap c Canvas.RoundCap;

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 200.0);
  Canvas.lineTo c (300.0, 200.0);
  Canvas.setTransform c (1.0,0.0,1.0,1.0,0.0,0.0);
  Canvas.stroke c;

  Canvas.setLineCap c Canvas.RoundCap;
  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 250.0);
  Canvas.lineTo c (300.0, 250.0);
  Canvas.setTransform c (1.0,0.0,-1.0,1.0,0.0,0.0);
  Canvas.stroke c;


  
  Canvas.setLineWidth c 20.0;
  Canvas.setLineCap c Canvas.Butt;
  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);
  Canvas.setLineJoin c Canvas.Round;
  
  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 325.0);
  Canvas.lineTo c (200.0, 275.0);
  Canvas.lineTo c (300.0, 320.0);
  Canvas.stroke c;

  
  Canvas.setLineCap c Canvas.Butt;
  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);
  Canvas.setLineJoin c Canvas.Bevel;
  
  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 375.0);
  Canvas.lineTo c (200.0, 325.0);
  Canvas.lineTo c (300.0, 375.0);
  Canvas.stroke c;
  
  Canvas.setLineCap c Canvas.Butt;
  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);
  Canvas.setLineJoin c Canvas.Round;
  
  Canvas.clearPath c;
  Canvas.moveTo c (200.0, 375.0);
  Canvas.lineTo c (100.0, 425.0);
  Canvas.lineTo c (300.0, 425.0);
  Canvas.closePath c;
  Canvas.stroke c;

  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);

  Canvas.clearPath c;
  Canvas.moveTo c (100.0, 505.0);
  Canvas.lineTo c (200.0, 455.0);
  Canvas.lineTo c (300.0, 505.0);
  Canvas.closePath c;
  Canvas.setTransform c (1.0,0.0,-1.0,1.0,0.0,0.0);
  Canvas.stroke c;

  Canvas.setTransform c (1.0,0.0,0.0,1.0,0.0,0.0);

  Canvas.clearPath c;
  Canvas.moveTo c (300.0, 595.0);
  Canvas.lineTo c (200.0, 535.0);
  Canvas.lineTo c (100.0, 595.0);
  Canvas.closePath c;
  Canvas.setTransform c (1.0,0.0,1.0,1.0,0.0,0.0);
  Canvas.stroke c;
  


  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true
    
    | Event.Frame { canvas = _; timestamp = _ } ->
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
