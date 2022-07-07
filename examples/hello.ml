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

  let _ = Canvas.exportPNG c "picture.png" in


  Canvas.setFillColor c Color.red;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(300.0, 200.0);

  let _ = Canvas.importPNG c ~pos:(0,0) "picture.png" in

(*
  let c2 = Canvas.createOffscreenFromPNG "picture.png" in
  let size = Canvas.getSize c2 in
  Canvas.blit ~dst:c ~dpos:(100, 50) ~src:c2 ~spos:(5, 5)
    ~size:(fst size - 10, snd size - 10);
*)
(*
  let id = ImageData.createFromPNG "picture.png" in
  ImageData.exportPNG id "picture2.png";
  ImageData.importPNG id ~pos:(-200, -100) "picture2.png";
  let c2 = Canvas.createOffscreenFromImageData id in
  let size = Canvas.getSize c2 in
  Canvas.blit ~dst:c ~dpos:(-20, -20) ~src:c2 ~spos:(5, 5)
    ~size:(fst size - 10, snd size - 10);
*)
(*
  let id = Canvas.getImageData c ~pos:(50,100) ~size:(100,100) in
  (* let id = Canvas.getImageData c ~pos:(0,0) ~size:(100,100) in *)
  ImageData.exportPNG id "id.png";

  Canvas.setImageData c ~dpos:(-50,-50) id ~spos:(0,0) ~size:(100,100);
*)

  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true

    | Event.ButtonAction { canvas = _; timestamp = _;
                           position = (x, y); button = _; state = Down } ->
        Canvas.setFillColor c Color.red;
        Canvas.clearPath c;
        Canvas.arc c ~center:(float_of_int x, float_of_int y)
          ~radius:5.0 ~theta1:0.0 ~theta2:(pi *. 2.0) ~ccw:false;
        Canvas.fill c ~nonzero:false;
        true

    | Event.Frame { canvas = _; timestamp = _ } ->
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
