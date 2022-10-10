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

type state = {
  r : float;
  snake : (float * float) list;
  cur_dir : float * float;
  food_loc : float * float;
}

let () =

  Random.self_init ();

  Backend.init ();

  let c =
    Canvas.createFramed "Snake" ~pos:(960 - 250, 540 - 250) ~size:(500, 500) in

  Canvas.show c;

  Backend.run (fun state -> function

      | Event.CanvasClosed { canvas = _; timestamp = _ } ->
          Backend.stop ();
          state, true

      | Event.KeyAction { canvas = _; timestamp = _;
                    key = KeyUpArrow; char = _; flags = _; state = Down } ->
          let state =
            if (snd state.cur_dir <> 0.0) then state
            else { state with cur_dir = (0.0, -1.0) }
          in
          state, true

      | Event.KeyAction { canvas = _; timestamp = _;
                   key = KeyDownArrow; char = _; flags = _; state = Down } ->
          let state =
            if (snd state.cur_dir <> 0.0) then state
            else { state with cur_dir = (0.0, 1.0) }
          in
          state, true

      | Event.KeyAction { canvas = _; timestamp = _;
                   key = KeyLeftArrow; char = _; flags = _; state = Down } ->
          let state =
            if (fst state.cur_dir <> 0.0) then state
            else { state with cur_dir = (-1.0, 0.0) }
          in
          state, true

      | Event.KeyAction { canvas = _; timestamp = _;
                   key = KeyRightArrow; char = _; flags = _; state = Down } ->
          let state =
            if (fst state.cur_dir <> 0.0) then state
            else { state with cur_dir = (1.0, 0.0) }
          in
          state, true

      | Event.KeyAction { canvas = _; timestamp = _;
                          key; char = _; flags = _; state = Down } ->
          if key = Event.KeyEscape then
            Backend.stop ();
          state, true

      | Event.Frame { canvas = c; timestamp = _ } ->
          buildBackground c;
          let snake, food_loc =
            if sumCoord (List.hd state.snake) state.cur_dir =
                 state.food_loc then
              state.food_loc :: state.snake,
              (2.0 +. float_of_int (Random.int 47),
               2.0 +. float_of_int (Random.int 47))
            else
              state.snake, state.food_loc
          in
          let r = state.r +. 1.0 /. 60.0 in
          let r, snake =
            if r <= 0.066 then r, snake
            else 0.0, moveSnakeDirection snake state.cur_dir
          in
          if snakeHitSelf snake || snakeHitWall snake then
            Backend.stop();
          drawSnake c snake;
          placeBlock c food_loc Color.green;
          { state with r; snake; food_loc }, true

      | _ ->
          state, false

    ) (function _state ->
      Printf.printf "Goodbye !\n"
    ) { r = -1.0;
        snake = [ (6.0, 8.0); (6.0, 7.0) ];
        cur_dir = (0.0, 1.0);
        food_loc = (24.0, 24.0) }
