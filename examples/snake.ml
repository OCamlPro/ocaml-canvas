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

let buildBackground c =
  Canvas.setFillColor c Color.black;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(500.0, 500.0);
  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(500.0, 10.0);
  Canvas.fillRect c ~pos:(0.0, 490.0) ~size:(500.0, 10.0);
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(10.0 ,500.0);
  Canvas.fillRect c ~pos:(490.0, 0.0) ~size:(10.0, 500.0)

let placeBlock c (x, y) col =
  Canvas.setFillColor c col;
  Canvas.fillRect c ~pos:(x *. 10.0, y *. 10.0) ~size:(10.0, 10.0)

let rec drawSnake c s = match s with
  | [] -> ()
  | h :: t -> placeBlock c h Color.orange; drawSnake c t

let rec moveSnake s p = match s with
  | [] -> []
  | h :: t -> p :: (moveSnake t h)

let sumCoord (a, b) (c, d) = (a +. c, b +. d)

let moveSnakeDirection s d = match(s) with
  | [] -> []
  | h :: _t -> moveSnake s (sumCoord d h)

let snakeHitSelf s = match s with
  | [] -> false
  | h :: t -> List.mem h t

let snakeHitWall s =
  let h = List.hd s in
  let (x, y) = h in
  (x < 1.0) || (x > 48.0) || (y < 1.0) || (y > 48.0)

let () =
  Random.self_init ();
  Backend.(init default_options);
  let c =
    Canvas.createFramed "test" ~pos:(960 - 250, 540 - 250) ~size:(500, 500) in
  Canvas.show c;
  let r = ref (-1.0) in
  let snake = ref [ (6.0, 8.0); (6.0, 7.0) ] in
  let currentDirection = ref (0.0, 1.0) in
  let foodLocation = ref (24.0, 24.0) in
  Backend.run (function
      | KeyAction { canvas = _; timestamp = _;
                    key = KeyUpArrow; char = _; flags = _; state = Down } ->
          let (_x, y) = !currentDirection in
          if (y = 0.0) then currentDirection := (0.0, -1.0);
          true
      | KeyAction { canvas = _; timestamp = _;
                   key = KeyDownArrow; char = _; flags = _; state = Down } ->
          let (_x, y) = !currentDirection in
          if (y = 0.0) then currentDirection := (0.0, 1.0);
          true
      | KeyAction { canvas = _; timestamp = _;
                   key = KeyLeftArrow; char = _; flags = _; state = Down } ->
          let (x, _y) = !currentDirection in
          if (x = 0.0) then currentDirection := (-1.0, 0.0);
          true
      | KeyAction { canvas = _; timestamp = _;
                   key = KeyRightArrow; char = _; flags = _; state = Down } ->
          let (x, _y) = !currentDirection in
          if (x = 0.0) then currentDirection := (1.0, 0.0);
          true
      | Event.KeyAction { canvas = _; timestamp = _;
                          key; char = _; flags = _; state = Down } ->
          if key = Event.KeyEscape then
            Backend.stop ();
          true
      | Frame { canvas = c; timestamp = _ } ->
          r := !r +. 1.0 /. 60.0;
          buildBackground c;
          let h = List.hd !snake in
          if (sumCoord h !currentDirection) = !foodLocation then
            (snake := !foodLocation :: !snake;
             foodLocation := (2.0 +. float_of_int (Random.int 47),
                              2.0 +. float_of_int (Random.int 47)));
          if !r > 0.066 then
            (r := 0.0;
             snake := moveSnakeDirection !snake !currentDirection);
          if snakeHitSelf !snake || snakeHitWall !snake then
            Backend.stop();
          drawSnake c !snake;
          placeBlock c !foodLocation Color.green;
          true
      | _ ->
          false
    ) (function () ->
      Printf.printf "Goodbye !\n"
    )
