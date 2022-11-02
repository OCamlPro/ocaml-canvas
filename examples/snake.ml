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
  mutable r : float;
  mutable snake : (float * float) list;
  mutable cur_dir : float * float;
  mutable food_loc : float * float;
}

let state = {
  r = -1.0;
  snake = [ (6.0, 8.0); (6.0, 7.0) ];
  cur_dir = (0.0, 1.0);
  food_loc = (24.0, 24.0)
}

let () =

  Random.self_init ();

  Backend.init ();

  let c =
    Canvas.createFramed "Snake" ~pos:(300, 200) ~size:(500, 500) in

  Canvas.show c;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        match key with
        | KeyEscape ->
            Backend.stop ()
        | KeyUpArrow ->
            if (snd state.cur_dir = 0.0) then state.cur_dir <- (0.0, -1.0)
        | KeyDownArrow ->
            if (snd state.cur_dir = 0.0) then state.cur_dir <- (0.0, 1.0)
        | KeyLeftArrow ->
            if (fst state.cur_dir = 0.0) then state.cur_dir <- (-1.0, 0.0)
        | KeyRightArrow ->
            if (fst state.cur_dir = 0.0) then state.cur_dir <- (1.0, 0.0)
        | _ ->
            ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.canvas = c; _ } ->
        buildBackground c;
        if sumCoord (List.hd state.snake) state.cur_dir = state.food_loc then
          begin
            state.snake <- state.food_loc :: state.snake;
            state.food_loc <-
              (2.0 +. float_of_int (Random.int 47),
               2.0 +. float_of_int (Random.int 47))
          end;
        state.r <- state.r +. 1.0 /. 60.0;
        if state.r > 0.066 then
          begin
            state.r <- 0.0;
            state.snake <- moveSnakeDirection state.snake state.cur_dir
          end;
        if snakeHitSelf state.snake || snakeHitWall state.snake then
          Backend.stop ();
        drawSnake c state.snake;
        placeBlock c state.food_loc Color.green) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
