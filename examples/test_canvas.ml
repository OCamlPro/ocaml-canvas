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



let f1 () =

  Backend.(init default_options);

  let create_canvas name x y width height =

    let c = Canvas.createFramed (name ^ " é \u{2600} \u{2622} \u{1F42B}")
              ~pos:(x, y) ~size:(width, height) in

    (* Background *)
    Canvas.setFillColor c (Color.of_string name);
    Canvas.fillRect c ~pos:(0.0, 0.0)
      ~size:(float_of_int width, float_of_int height);

    (* Cyan square *)
    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 150.0);
    Canvas.lineTo c (50.0, 50.0);
    Canvas.lineTo c (150.0, 50.0);
    Canvas.lineTo c (150.0, 150.0);
    Canvas.lineTo c (50.0, 150.0);
    Canvas.closePath c;
    Canvas.setStrokeColor c (Color.of_string "cyan");
    Canvas.setLineWidth c 10.0;
    Canvas.stroke c;

    (* Red line *)
    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 25.0);
    Canvas.lineTo c (150.0, 25.0);
    Canvas.setStrokeColor c (Color.of_string "red");
    Canvas.stroke c;

    (* Canvas name as string *)
(*    Canvas.setFont c "Chilanka" ~size:20.0 ~slant:Roman ~weight:Font.regular;*)
(*    Canvas.setFont c "Candara" ~size:20.0 ~slant:Roman ~weight:Font.regular;*)

    (* Canvas.setFont c "Liberation Sans" ~size:20.0 ~slant:Roman ~weight:Font.regular; *)

    Canvas.setFillColor c (Color.of_string "black");
    Canvas.save c;
    Canvas.translate c
      ((float_of_int width) /. 2.0,
       (float_of_int height) /. 2.0);
    Canvas.rotate c 0.3;
    Canvas.setLineWidth c 1.0;
    Canvas.strokeText c name (-100.0, 0.0);
    Canvas.fillRect c ~pos:(-90.0, 0.0) ~size:(180.0, 10.0);
    Canvas.restore c;

    (* Small dot in the middle *)
    (* Canvas.save c; *)
    Canvas.translate c
      ((float_of_int width) /. 2.0, (float_of_int height) /. 2.0);
    Canvas.fillRect c ~pos:(-5.0, -5.0) ~size:(10.0, 10.0);
    (* Canvas.restore c; *)

    Canvas.show c;

    c
  in

  let _c = create_canvas "Orange" 50 90 200 200 in

  Backend.run (function
      | KeyAction { canvas = c; timestamp = _;
                    key; char; flags = _; state = Down } ->

          Canvas.save c;
          Canvas.setTransform c (1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
          Canvas.setFillColor c (Color.of_string "White");
          Canvas.fillRect c ~pos:(80.0, 70.0) ~size:(50.0, 50.0);
          Canvas.setFillColor c (Color.of_string "Black");
          let b = Buffer.create 4 in
          Buffer.add_utf_8_uchar b char;
          Canvas.fillText c (Buffer.contents b) (100.0, 100.0);
          Canvas.restore c;

          (* let _ = failwith "WOW" in *)

          if key = Event.KeyEscape then
            Backend.stop ();

          true

      | ButtonAction { canvas = c; timestamp = _;
                       position = (x, y); button = b; state = Down } ->

          let width, height = Canvas.getSize c in

          Canvas.setSize c (width, height);
          let color =
            match b with
              | ButtonLeft -> "Cyan"
              | ButtonRight -> "Pink"
              | _ -> "Red"
          in
          Canvas.setFillColor c (Color.of_string color);

          Canvas.fillRect c
            ~pos:(float_of_int (x - 5), float_of_int (y - 5))
            ~size:(10.0, 10.0);


          let id = Canvas.getImageData c ~pos:(x-2, y-2) ~size:(5, 5) in

          for i = 0 to 4 do
            for j = 0 to 4 do
              id.{i,j,0} <- 255 - id.{i,j,0};
              id.{i,j,1} <- 255 - id.{i,j,1};
              id.{i,j,2} <- 255 - id.{i,j,2}
            done
          done;

          Canvas.putImageData c ~dpos:(x-2, y-2) id ~spos:(0, 0) ~size:(5, 5);

(*
          Canvas.putPixel c (x, y) Color.red;
          Canvas.putPixel c (x+1, y) Color.red;
          Canvas.putPixel c (x+1, y+1) Color.red;
          Canvas.putPixel c (x, y+1) Color.red;
*)

          true

      | Frame { canvas = c; timestamp = _ } ->
          Canvas.rotate c 0.01;
          Canvas.clearPath c;
          Canvas.setFillColor c (Color.of_string "Cyan");
          Canvas.arc c ~center:(50.0, 50.0) ~radius:20.0
            ~theta1:0.0 ~theta2:1.5 ~ccw:false;
          Canvas.fill c ~nonzero:false;

          true

      | _ ->
        false
    ) (function () ->
         Printf.printf "Done with main loop\n"
    )









let f2 () =

  Backend.(init default_options);

  let c1 = Canvas.createFramed "Test1" ~pos:(50, 50) ~size:(200, 200) in
  Canvas.setFillColor c1 (Color.of_string "red");
  Canvas.fillRect c1 ~pos:(0.0, 0.0) ~size:(200.0, 200.0);
  Canvas.show c1;

  let c2 = Canvas.createFramed "Test2" ~pos:(250, 50) ~size:(200, 200) in
  Canvas.setFillColor c2 (Color.of_string "green");
  Canvas.fillRect c2 ~pos:(0.0, 0.0) ~size:(200.0, 200.0);
  Canvas.show c2;

  let c1_color = ref (Color.of_string "red") in
  let c2_color = ref (Color.of_string "green") in

  Backend.run (function
      | Event.CanvasResized { canvas = _; timestamp = _; size = _ } ->
          true
      | CanvasMoved { canvas = _; timestamp = _; position = _ } ->
          false
      | CanvasFocused { canvas = _; timestamp = _; focus = _ } ->
          false
      | CanvasClosed { canvas = _; timestamp = _ } ->
          false
      | KeyAction { canvas = _; timestamp = _;
                    key = _; char = _; flags = _; state = _ } ->
          false
      | ButtonAction { canvas = _; timestamp = _;
                       position = _; button = _; state = _ } ->
          true
      | MouseMove { canvas = _; timestamp = _; position = (_x, _y) } ->
          true
      | Frame { canvas = c; timestamp = _ } ->
          let color =
            if (*Canvas.*)(c == c1) then
              (c1_color :=
                 Color.of_int (
                   (Color.to_int !c1_color) + 1); !c1_color)
            else (c2_color :=
                    Color.of_int (
                      (Color.to_int !c2_color) + 1); !c2_color)
          in

          Canvas.setFillColor c color;
          Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(200.0, 200.0);

          true
    ) (function () -> ())










let f3 () =

  Backend.(init default_options);

  let create_canvas name x y width height =

    let c = Canvas.createFramed (name ^ " é \u{2600} \u{2622} \u{1F42B}")
              ~pos:(x, y) ~size:(width, height) in

    (* Background *)
    Canvas.setFillColor c (Color.of_string name);
    Canvas.fillRect c ~pos:(0.0, 0.0)
      ~size:(float_of_int width, float_of_int height);

    (* Cyan square *)
    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 150.0);
    Canvas.lineTo c (50.0, 50.0);
    Canvas.lineTo c (150.0, 50.0);
    Canvas.lineTo c (150.0, 150.0);
    Canvas.lineTo c (50.0, 150.0);
    Canvas.closePath c;
    Canvas.setStrokeColor c (Color.of_string "cyan");
    Canvas.setLineWidth c 10.0;
    Canvas.stroke c;

    (* Red line *)
    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 25.0);
    Canvas.lineTo c (150.0, 25.0);
    Canvas.setStrokeColor c (Color.of_string "red");
    Canvas.stroke c;

    (* Canvas name as string *)
(*    Canvas.setFont c "Chilanka" ~size:20.0 ~slant:Roman ~weight:Font.regular; *)
    (* Canvas.setFont c "Candara" ~size:20.0 ~slant:Roman ~weight:Font.regular;
     * Canvas.setFillColor c (Color.of_string "black");
     * Canvas.save c;
     * Canvas.translate c
     *   ((float_of_int width) /. 2.0, (float_of_int height) /. 2.0);
     * Canvas.rotate c 0.3;
     * Canvas.fillText c name (-100.0, 0.0);
     * Canvas.fillRect c ~pos:(-90.0, 0.0) ~size:(180.0, 10.0);
     * Canvas.restore c; *)

    (* Small dot in the middle *)
    (* Canvas.save c; *)
    Canvas.translate c
      ((float_of_int width) /. 2.0, (float_of_int height) /. 2.0);
    Canvas.fillRect c ~pos:(-5.0, -5.0) ~size:(10.0, 10.0);
    (* Canvas.restore c; *)

    Canvas.show c;

    c
  in

  let c1 = create_canvas "Red" 30 30 200 200 in
  let c2 = create_canvas "Green" 90 50 200 200 in
  let c3 = create_canvas "Blue" 70 70 200 200 in
  let c4 = create_canvas "Orange" 50 90 200 200 in

  let c5 = Canvas.createFrameless ~pos:(300, 500*0+30) ~size:(500, 500) in
  Canvas.setFillColor c5 (Color.of_string "Lime");
  Canvas.fillRect c5 ~pos:(0.0, 0.0) ~size:(500.0, 500.0);
  Canvas.show c5;

  let c6 = Canvas.createOffscreen ~size:(200, 200) in
  Canvas.setFillColor c6 (Color.of_string "Pink");
  Canvas.fillRect c6 ~pos:(0.0, 0.0) ~size:(200.0, 200.0);

  Canvas.blit ~dst:c5 ~dpos:(10, 10) ~src:c6 ~spos:(0, 0) ~size:(200, 200);
  Canvas.blit ~dst:c5 ~dpos:(250, 10) ~src:c4 ~spos:(0, 0) ~size:(200, 200);
  Canvas.blit ~dst:c5 ~dpos:(400, 250) ~src:c3 ~spos:(0, 0) ~size:(200, 200);

  Canvas.setFont c5 "FreeSerif" ~size:36.0 ~slant:Roman ~weight:Font.regular;
  (* Canvas.setFont c5 "Symbola" ~size:36.0 ~slant:Roman ~weight:Font.regular; *)
  (* Canvas.setFont c5 "DejaVu Sans" ~size:72.0 ~slant:Roman ~weight:Font.regular; *)
  (* Canvas.setFont c5 "Liberation Sans" ~size:24.0 ~slant:Roman ~weight:Font.regular; *)
  (*Canvas.setFont c5 "Liberation Serif" ~size:72.0 ~slant:Roman ~weight:Font.regular;*)
  (*Canvas.setFont c5 "Times New Roman" ~size:72.0 ~slant:Roman ~weight:Font.regular;*)
  Canvas.setFillColor c5 (Color.of_string "black");
  Canvas.save c5;
(*  Canvas.scale c5 (2.0, 2.0); *)
  Canvas.fillText c5
    (* "This is sample text é \u{2600} \u{2622} \u{1F42B}" (10.0/.1.0) (300.0/.1.0) (400.0); *)
    "ABC é \u{2600} \u{2622} \u{1F42B}" (10.0/.1.0, 300.0/.1.0);
  Canvas.clearPath c5;
  Canvas.moveTo c5 (10.0, 300.0);
  Canvas.lineTo c5 (410.0, 300.0);
  Canvas.stroke c5;
  Canvas.restore c5;



(* add canvas created event *)
(* about to destroy *)
  Backend.run (function
      | Event.CanvasResized { canvas = c; timestamp = _;
                              size = (width, _height) } ->
Printf.printf "Canvas resized %d\n" width;
Format.print_flush ();
          let color =
            (* let open Canvas in *)
            if c == c1 then "Red"
            else if c == c2 then "Green"
            else if c == c3 then "Blue"
            else if c == c4 then "Orange"
            else "Black"
          in
          Canvas.setFillColor c (Color.of_string color);
          let width, height = Canvas.getSize c in
          Canvas.fillRect c ~pos:(0.0, 0.0)
            ~size:(float_of_int width, float_of_int height);
          true;

      | CanvasMoved { canvas = _c; timestamp = _; position = (x, y) } ->
      Printf.printf "Canvas moved %d, %d\n" x y;
      Format.print_flush ();
          false

      | CanvasFocused { canvas = c; timestamp = _; focus } ->
        Printf.printf "Focus\n";
        Format.print_flush ();
      let name =
        (* let open Canvas in *)
        if c = c1 then "Red"
        else if c = c2 then "Green"
        else if c = c3 then "Blue"
        else if c = c4 then "Orange"
        else "Black"
      in
          Printf.printf "Focus %s on %s\n"
            (if focus = In then "in" else "out") name;
          Format.print_flush ();
          false
      | CanvasClosed { canvas = _c; timestamp = _ } ->
        Printf.printf "Close\n";
        Format.print_flush (); (*
          let _ = create_canvas "Cyan" (50, 50) (100, 100) in *)
          false
      | KeyAction { canvas = _; timestamp = _;
                    key; char; flags; state } ->
          Printf.printf "Key_down : %04X (%04X) (shift=%b) (alt=%b) (ctrl=%b) (dead=%b) (%s)\n"
            (Event.int_of_key key) (Uchar.to_int char)
            flags.flag_shift flags.flag_alt flags.flag_control flags.flag_dead
            (if state = Up then "up" else "down");
            Format.print_flush ();

          if key = Event.KeyQ then
            Backend.stop ();

          false
      | ButtonAction { canvas = _; timestamp = _;
                       position = (x, y); button = _; state = Up } ->
      Printf.printf "Button_up at %d %d\n" x y;
      Format.print_flush ();
          false
      | ButtonAction { canvas = c; timestamp = _;
                       position = (x, y); button = b; state = Down } ->
        Printf.printf "Buton\n";
        Format.print_flush ();

(*
(*          let _ = create_canvas "Cyan" 50 50 100 100 in *)
        let ccc = Canvas.createFramed
              ("X" ^ " é \u{2600} \u{2622} \u{1F42B}") 10 10 200 200 in
    Canvas.show ccc;
*)

          let width, height = Canvas.getSize c in
          (* let width = width + 10 in *)
Printf.printf "Button_down at %d %d / set_size %d %d\n" x y width height;
Format.print_flush ();
          Canvas.setSize c (width, height);
          let color =
            match b with
              | ButtonLeft -> "Cyan"
              | ButtonRight -> "Pink"
              | _ -> "Orange"
          in
          Canvas.setFillColor c (Color.of_string color);

          Canvas.fillRect c
            ~pos:(float_of_int (x - 5), float_of_int (y - 5))
            ~size:(10.0, 10.0);

          Canvas.putPixel c (x, y) Color.red;
          Canvas.putPixel c (x+1, y) Color.red;
          Canvas.putPixel c (x+1, y+1) Color.red;
          Canvas.putPixel c (x, y+1) Color.red;

          true
      | MouseMove { canvas = _c; timestamp = _; position = _ } ->
(*        Printf.printf "Mouse\n";
        Format.print_flush (); *)
(*
          Printf.printf "Mouse move at %d %d\n" x y;
          Format.print_flush (); *)
          false
      | Frame { canvas = c; timestamp = _ } ->

        (* Printf.printf "Frame\n";
         * Format.print_flush (); *)

        Canvas.clearPath c;
        Canvas.setFillColor c (Color.of_string "Cyan");
        Canvas.arc c ~center:(100.0, 100.0) ~radius:20.0
          ~theta1:0.0 ~theta2:1.5 ~ccw:false;
        Canvas.fill c ~nonzero:false;

(*

      let _x = 10 + Int64.to_int (Int64.rem (Int64.div t (Int64.of_int 1_000_00)) (Int64.of_int 100)) in

(*      Canvas.translate c (0.1, 0.1); *)
(*      Canvas.scale c (1.005, 1.005); *)
(*      Canvas.shear c (0.0, 0.01); *)
      Canvas.rotate c 0.01;

      Canvas.setFillColor c (Color.of_string "Cyan");
(*      Canvas.fillRect c
          ~pos:(float_of_int (x - 5), 150.0) ~size:(10.0,10.0); *)
      Canvas.fillRect c ~pos:(20.0, 20.0) ~size:(10.0, 10.0);
*)

          true
    ) (function () ->
      Printf.printf "We're done\n";
      Format.print_flush ();
      Backend.run (function
          | KeyAction { canvas = _c; timestamp = _;
                        key = _; char = _; flags = _; state = Down } ->
            Backend.stop ();
            true
          | _ -> false
        ) (function () ->
          Printf.printf "We're REALLY done\n"
        )

    )










let f4 () =

  Backend.(init default_options);

  let create_canvas name x y width height =
    let c = Canvas.createFramed (name ^ " é \u{2600} \u{2622} \u{1F42B}")
              ~pos:(x, y) ~size:(width, height) in
    Canvas.setFillColor c (Color.of_string name);
    Canvas.fillRect c ~pos:(0.0, 0.0)
      ~size:(float_of_int width, float_of_int height);
(*
    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 150.0);
    Canvas.lineTo c (50.0, 50.0);
    Canvas.lineTo c (150.0, 50.0);
    Canvas.lineTo c (150.0, 150.0);
    Canvas.lineTo c (50.0, 150.0);
    Canvas.setStrokeColor c (Color.of_string "cyan");
    Canvas.stroke c;

    Canvas.clearPath c;
    Canvas.moveTo c (50.0, 25.0);
    Canvas.lineTo c (150.0, 25.0);
    Canvas.setStrokeColor c (Color.of_string "red");
    Canvas.stroke c;

    Canvas.setFillColor c (Color.of_string "black");
    Canvas.fillText c name (100.0, 100.0);
*)
    Canvas.translate c
      ((float_of_int width) /. 2.0, (float_of_int height) /. 2.0);

    let _ = Canvas.fillRect c ~pos:(-5.0, -5.0) ~size:(10.0, 10.0) in

    Canvas.show c;

    c
  in

  (*let _c = create_canvas "Red" 1900 30 500 500 in *)
  let _c = create_canvas "Red" 190 30 500 500 in

  let _rot = ref 0.0 in

let _done = ref false in

  Backend.run (function
      | Event.CanvasResized { canvas = _; timestamp = _; size = _ } ->
          true
      | CanvasMoved { canvas = _; timestamp = _; position = _ } ->
          false
      | CanvasFocused { canvas = _; timestamp = _; focus = _ } ->
          false
      | CanvasClosed { canvas = _; timestamp = _ } ->
          false
      | KeyAction { canvas = _; timestamp = _;
                    key = _; char = _; flags = _; state = _ } ->
          false
      | ButtonAction { canvas = _; timestamp = _;
                       position = _; button = _; state = Up } ->
          false
      | ButtonAction { canvas = _; timestamp = _;
                       position = _; button = _; state = Down } ->
          false
      | MouseMove { canvas = _; timestamp = _; position = _ } ->
          false
      | Frame { canvas = c; timestamp = _ } ->
          Canvas.setLineWidth c 20.0;

          Canvas.setFillColor c (Color.of_string "Cyan");
          Canvas.setStrokeColor c (Color.of_string "orange");

          (*Canvas.rotate c 0.025;*)
          (*Canvas.shear c (0.001, 0.0);*)
          (*Canvas.scale c (0.01, 0.0);*)

          Canvas.clearPath c;

if not !_done then begin

(*
          Canvas.moveTo c (-100.0, -100.0);
          Canvas.arc c ~center:(-100.0, -100.0) ~radius:50.0
            ~theta1:0.0 ~theta2:(Float.pi *. 1.5) ~ccw:false;
          Canvas.stroke c;
          (*Canvas.fill c ~nonzero:false ;*)
*)

(*
          Canvas.moveTo c (-100.0, -100.0);
          Canvas.arcTo c ~p1:(100.0, -100.0) ~p2:(100.0, 100.0) ~radius:20.0;
          Canvas.lineTo c (100.0, 100.0);
          Canvas.stroke c;
          (*Canvas.fill c ~nonzero:false;*)
*)

(*
          Canvas.moveTo c (-100.0, 0.0);
          Canvas.lineTo c (100.0, 0.0);
          Canvas.lineTo c (100.0, 100.0);
          Canvas.lineTo c (0.0, 100.0);
          Canvas.lineTo c (0.0, -100.0);
          Canvas.stroke c;
*)

          Canvas.moveTo c (-100.0, 0.0);
          Canvas.lineTo c (100.0, 0.0);
          Canvas.lineTo c (100.0, 100.0);
          Canvas.lineTo c (0.0, 100.0);
          Canvas.lineTo c (0.0, -100.0);

          Canvas.lineTo c (10.0, -100.0);
          Canvas.lineTo c (10.0, 90.0);
          Canvas.lineTo c (90.0, 90.0);
          Canvas.lineTo c (90.0, 10.0);
          Canvas.lineTo c (-100.0, 10.0);

          Canvas.lineTo c (-100.0, 0.0);
          Canvas.closePath c;

          Canvas.fill c ~nonzero:true;

_done := true;
end;


(*
          Canvas.moveTo c (-100.0, 100.0);
          Canvas.bezierCurveTo c
            ~cp1:(-100.0, -100.0) ~cp2:(100.0, -100.0) ~p:(100.0,100.0);
          Canvas.closePath c;

          Canvas.lineTo c (-100.0 200.0);
          Canvas.lineTo c (100.0, 200.0);

          Canvas.stroke c;

          Canvas.closePath c;

          Canvas.stroke c;
*)

(*
          Canvas.setLineWidth c 10.0;

          Canvas.moveTo c (0.0, 0.0);
          Canvas.ellipse c ~center:(0.0, 0.0) ~radius:(50.0, 75.0)
            ~rotation:1.0 ~theta1:0.0 ~theta2:(Float.pi *. 1.75) false;
(*         Canvas.closePath c; *)
          Canvas.stroke c;
*)


(*
          Canvas.fillRect c ~pos:(-300.0, -300.0) ~size:(600.0, 600.0);

          Canvas.clearPath c;

          Canvas.moveTo c (-100.0, -100.0);
          Canvas.lineTo c (100.0, -100.0);

          rot := !rot +. 0.025;
          Canvas.rotate c (!rot);
          Canvas.lineTo c (100.0, 100.0);
          Canvas.rotate c (-. !rot);

          Canvas.stroke c;
*)


(*
          Canvas.moveTo c (50.0, 50.0);
          Canvas.lineTo c (100.0, 50.0);
          Canvas.lineTo c (100.0, 100.0);
          Canvas.lineTo c (50.0, 100.0);
          Canvas.lineTo c (50.0, 50.0);
          Canvas.closePath c;
          Canvas.stroke c;
*)

(*
          Canvas.moveTo c (50.0, 50.0);
          Canvas.lineTo c (50.0, 100.0);
          Canvas.lineTo c (100.0, 100.0);
          Canvas.lineTo c (100.0, 50.0);
          Canvas.lineTo c (50.0, 50.0);
          Canvas.closePath c;
          Canvas.stroke c;
*)

(*
          Canvas.moveTo c (50.0, 50.0);
          Canvas.arc c ~center:(100.0, 100.0) ~radius:20.0
            ~theta1:0.0 ~theta2:1.5 ~ccw:false;
*)

(*
          Canvas.moveTo c (50.0, 50.0);
          Canvas.arcTo c ~p1:(100.0, 50.0) ~p2:(100.0, 100.0) ~radius:10.0;
          Canvas.lineTo c (100.0, 100.0);
*)

(*
          Canvas.moveTo c (400.0, 100.0);
          Canvas.arc c ~center:(350.0, 150.0) ~radius:50.0
            ~theta1:0.0 ~theta2:(Float.pi /. 2.0) ~ccw:false;
          Canvas.lineTo c (300.0, 200).0;
          Canvas.fill c ~nonzero:false;

          Canvas.moveTo c (100.0, 400.0);
          Canvas.arc c ~center:(150.0, 350.0) ~radius:50.0
            ~theta1:0.0 ~theta2:(Float.pi *. 1.5) ~ccw:true;
          Canvas.lineTo c (100.0, 300.0);
          Canvas.fill c ~nonzero:false;
*)


(*
          Canvas.moveTo c (10.0, 100.0);
          Canvas.bezierCurveTo c
            ~cp1:(10.0, 50.0) ~cp2:(190.0, 150.0) ~p:(190.0, 100.0);
*)

(*
          Canvas.moveTo c (10.0, 100.0);
          Canvas.quadraticCurveTo c ~cp:(100.0, 50.0) ~p:(190.0, 100.0);
*)

(*          Canvas.fill c ~nonzero:false; *)

          true
    ) (function () -> ())



let () =
  let _ = f1 () in
  let _ = f2  in
  let _ = f3  in
  let _ = f4  in
  ()

