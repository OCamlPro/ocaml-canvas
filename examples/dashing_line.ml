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

let () =

  Backend.init ();

  let c = Canvas.createFramed "Dashing line"
            ~pos:(300, 50) ~size:(400, 650) in

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

  Backend.run (fun state -> function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        state, true

    | Event.CanvasClosed { canvas = _; timestamp = _ } ->
        Backend.stop ();
        state, true

    | Event.Frame { canvas = _; timestamp = _ } ->
        let state = state +. 1.0 in
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(400.0, 650.0);
        Canvas.setLineDashOffset c state;
        Canvas.stroke c;
        state, true

    | _ ->
        state, false

    ) (function _state ->
         Printf.printf "Goodbye !\n"
    ) 0.0
