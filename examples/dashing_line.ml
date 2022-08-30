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

  let a = Array.create_float 2 in
  Array.set a 0 20.0;
  Array.set a 1 20.0;
  Canvas.setLineDash c (a);
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
  Canvas.show c;

  let r = ref 0.0 in

  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true

    
    | Event.Frame { canvas = _; timestamp = _ } ->
        r := !r +. 1.0;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(400.0, 650.0);
        Canvas.setLineDashOffset c !r;
        Canvas.stroke c;
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
