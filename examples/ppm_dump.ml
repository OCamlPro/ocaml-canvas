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

  let c = Canvas.createOnscreen ~title:"PPM Dump"
            ~pos:(300, 200) ~size:(300, 200) () in

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

  let img = Canvas.getImageData c ~pos:(0, 0) ~size:(300, 200) in
  let img_ba = ImageData.to_bigarray img in

  let oc = open_out "picture.ppm" in
  let fmt = Format.formatter_of_out_channel oc in
  Format.fprintf fmt "P3\n%d %d\n%d\n" 300 200 255;

  for i = 0 to 199 do
    for j = 0 to 299 do
      let b = Bigarray.Array3.get img_ba i j 0 in
      let g = Bigarray.Array3.get img_ba i j 1 in
      let r = Bigarray.Array3.get img_ba i j 2 in
      let _a = Bigarray.Array3.get img_ba i j 3 in
      let sep = if (j land 3) = 3 then '\n' else ' ' in
      Format.fprintf fmt "%d %d %d%c" r g b sep
    done
  done;

  close_out oc;

  Canvas.putImageData c ~dpos:(0, 0) img ~spos:(0, 0) ~size:(300, 200);

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
    React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.position = (x, y); button = _ } } ->
        Canvas.setFillColor c Color.red;
        Canvas.clearPath c;
        Canvas.arc c ~center:(float_of_int x, float_of_int y)
          ~radius:5.0 ~theta1:0.0 ~theta2:(2.0 *. Const.pi) ~ccw:false;
        Canvas.fill c ~nonzero:false;
      ) Event.button_down;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
