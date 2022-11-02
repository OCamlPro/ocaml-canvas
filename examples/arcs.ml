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

let arc c (x, y) t1 t2 r ccw =
  Canvas.clearPath c;
  Canvas.moveTo c (x, y);
  Canvas.arc c ~center:(x, y) ~radius:r ~theta1:t1 ~theta2:t2 ~ccw;
  Canvas.fill c ~nonzero:false

let () =

  Backend.init ();

  let c = Canvas.createFramed "Arcs"
            ~pos:(300, 200) ~size:(900, 900) in

  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(900.0, 900.0);

  Canvas.setFillColor c Color.black;
  Canvas.setStrokeColor c Color.black;

  Canvas.setFont c "Liberation Sans" ~size:24.0
    ~slant:Font.Roman ~weight:Font.bold;

  Canvas.fillText c "theta1 < theta 2, ccw = false" (20.0, 30.0);
  Canvas.fillText c "theta1 > theta 2, ccw = false" (20.0, 880.0);
  Canvas.fillText c "theta1 < theta 2, ccw = true" (470.0, 30.0);
  Canvas.fillText c "theta1 > theta 2, ccw = true" (470.0, 880.0);

  Canvas.clearPath c;
  Canvas.moveTo c (450.0, 50.0);
  Canvas.lineTo c (450.0, 850.0);
  Canvas.stroke c;
  Canvas.clearPath c;
  Canvas.moveTo c (50.0, 450.0);
  Canvas.lineTo c (850.0, 450.0);
  Canvas.stroke c;

  (* Various arcs with theta 1 < theta 2, ccw = false *)

  arc c (100.0, 100.0) 0.0 (Const.pi_2) 40.0 false;
  arc c (200.0, 100.0) (Const.pi_2) Const.pi 40.0 false;
  arc c (300.0, 100.0) Const.pi (3.0 *. Const.pi_2) 40.0 false;
  arc c (400.0, 100.0) (3.0 *. Const.pi_2) (2.0 *. Const.pi) 40.0 false;

  arc c (100.0, 200.0) 0.0 Const.pi 40.0 false;
  arc c (200.0, 200.0) (Const.pi_2) (3.0 *. Const.pi_2) 40.0 false;
  arc c (300.0, 200.0) Const.pi (2.0 *. Const.pi) 40.0 false;
  arc c (400.0, 200.0) (3.0 *. Const.pi_2) (5.0 *. Const.pi_2) 40.0 false;

  arc c (100.0, 300.0) 0.0 (3.0 *. Const.pi_2) 40.0 false;
  arc c (200.0, 300.0) (Const.pi_2) (2.0 *. Const.pi) 40.0 false;
  arc c (300.0, 300.0) Const.pi (5.0 *. Const.pi_2) 40.0 false;
  arc c (400.0, 300.0) (3.0 *. Const.pi_2) (3.0 *. Const.pi) 40.0 false;

  arc c (100.0, 400.0) 0.0 (2.0 *. Const.pi) 40.0 false;
  arc c (200.0, 400.0) (Const.pi_2) (5.0 *. Const.pi_2) 40.0 false;
  arc c (300.0, 400.0) Const.pi (3.0 *. Const.pi) 40.0 false;
  arc c (400.0, 400.0) (3.0 *. Const.pi_2) (7.0 *. Const.pi_2) 40.0 false;

  (* Various arcs with theta 1 > theta 2, ccw = false *)

  arc c (100.0, 500.0) (Const.pi_2) 0.0 40.0 false;
  arc c (200.0, 500.0) Const.pi (Const.pi_2) 40.0 false;
  arc c (300.0, 500.0) (3.0 *. Const.pi_2) Const.pi 40.0 false;
  arc c (400.0, 500.0) (2.0 *. Const.pi) (3.0 *. Const.pi_2) 40.0 false;

  arc c (100.0, 600.0) Const.pi 0.0 40.0 false;
  arc c (200.0, 600.0) (3.0 *. Const.pi_2) (Const.pi_2) 40.0 false;
  arc c (300.0, 600.0) (2.0 *. Const.pi) Const.pi 40.0 false;
  arc c (400.0, 600.0) (5.0 *. Const.pi_2) (3.0 *. Const.pi_2) 40.0 false;

  arc c (100.0, 700.0) (3.0 *. Const.pi_2) 0.0 40.0 false;
  arc c (200.0, 700.0) (2.0 *. Const.pi) (Const.pi_2) 40.0 false;
  arc c (300.0, 700.0) (5.0 *. Const.pi_2) Const.pi 40.0 false;
  arc c (400.0, 700.0) (3.0 *. Const.pi) (3.0 *. Const.pi_2) 40.0 false;

  arc c (100.0, 800.0) (2.0 *. Const.pi) 0.0 40.0 false;
  arc c (200.0, 800.0) (5.0 *. Const.pi_2) (Const.pi_2) 40.0 false;
  arc c (300.0, 800.0) (3.0 *. Const.pi) Const.pi 40.0 false;
  arc c (400.0, 800.0) (7.0 *. Const.pi_2) (3.0 *. Const.pi_2) 40.0 false;

  (* Various arcs with theta 1 < theta 2, ccw = true *)

  arc c (500.0, 100.0) 0.0 (Const.pi_2) 40.0 true;
  arc c (600.0, 100.0) (Const.pi_2) Const.pi 40.0 true;
  arc c (700.0, 100.0) Const.pi (3.0 *. Const.pi_2) 40.0 true;
  arc c (800.0, 100.0) (3.0 *. Const.pi_2) (2.0 *. Const.pi) 40.0 true;

  arc c (500.0, 200.0) 0.0 Const.pi 40.0 true;
  arc c (600.0, 200.0) (Const.pi_2) (3.0 *. Const.pi_2) 40.0 true;
  arc c (700.0, 200.0) Const.pi (2.0 *. Const.pi) 40.0 true;
  arc c (800.0, 200.0) (3.0 *. Const.pi_2) (5.0 *. Const.pi_2) 40.0 true;

  arc c (500.0, 300.0) 0.0 (3.0 *. Const.pi_2) 40.0 true;
  arc c (600.0, 300.0) (Const.pi_2) (2.0 *. Const.pi) 40.0 true;
  arc c (700.0, 300.0) Const.pi (5.0 *. Const.pi_2) 40.0 true;
  arc c (800.0, 300.0) (3.0 *. Const.pi_2) (3.0 *. Const.pi) 40.0 true;

  arc c (500.0, 400.0) 0.0 (2.0 *. Const.pi) 40.0 true;
  arc c (600.0, 400.0) (Const.pi_2) (5.0 *. Const.pi_2) 40.0 true;
  arc c (700.0, 400.0) Const.pi (3.0 *. Const.pi) 40.0 true;
  arc c (800.0, 400.0) (3.0 *. Const.pi_2) (7.0 *. Const.pi_2) 40.0 true;

  (* Various arcs with theta 1 > theta 2, ccw = true *)

  arc c (500.0, 500.0) (Const.pi_2) 0.0 40.0 true;
  arc c (600.0, 500.0) Const.pi (Const.pi_2) 40.0 true;
  arc c (700.0, 500.0) (3.0 *. Const.pi_2) Const.pi 40.0 true;
  arc c (800.0, 500.0) (2.0 *. Const.pi) (3.0 *. Const.pi_2) 40.0 true;

  arc c (500.0, 600.0) Const.pi 0.0 40.0 true;
  arc c (600.0, 600.0) (3.0 *. Const.pi_2) (Const.pi_2) 40.0 true;
  arc c (700.0, 600.0) (2.0 *. Const.pi) Const.pi 40.0 true;
  arc c (800.0, 600.0) (5.0 *. Const.pi_2) (3.0 *. Const.pi_2) 40.0 true;

  arc c (500.0, 700.0) (3.0 *. Const.pi_2) 0.0 40.0 true;
  arc c (600.0, 700.0) (2.0 *. Const.pi) (Const.pi_2) 40.0 true;
  arc c (700.0, 700.0) (5.0 *. Const.pi_2) Const.pi 40.0 true;
  arc c (800.0, 700.0) (3.0 *. Const.pi) (3.0 *. Const.pi_2) 40.0 true;

  arc c (500.0, 800.0) (2.0 *. Const.pi) 0.0 40.0 true;
  arc c (600.0, 800.0) (5.0 *. Const.pi_2) (Const.pi_2) 40.0 true;
  arc c (700.0, 800.0) (3.0 *. Const.pi) Const.pi 40.0 true;
  arc c (800.0, 800.0) (7.0 *. Const.pi_2) (3.0 *. Const.pi_2) 40.0 true;

  Canvas.show c;

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
