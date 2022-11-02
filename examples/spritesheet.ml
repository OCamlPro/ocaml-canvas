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

type look_dir =
  | Down
  | Up
  | Left
  | Right

type state = {
  mutable sprite_sheet_opt : [`Offscreen] Canvas.t option;
  mutable look_direction : look_dir;
  mutable animation_frame : int;
  mutable clock : Int64.t;
  mutable old_time : Int64.t;
  mutable x : float;
  mutable y : float;
}

let state = {
  sprite_sheet_opt = None;
  look_direction = Up;
  animation_frame = 0;
  clock = Int64.zero;
  old_time = Int64.zero;
  x = 300.0;
  y = 300.0;
}

let int_of_look_dir = function
  | Down -> 0
  | Up -> 1
  | Left -> 2
  | Right -> 3

let clamp v low high =
  if v < low then low
  else if v > high then high
  else v

let () =

  Backend.init ();

  let c = Canvas.createFramed "Spritesheet"
            ~pos:(300, 200) ~size:(1024, 1024) in

  Canvas.setFillColor c Color.white;

  Canvas.show c;

  let event_sprite_sheet =
    Canvas.createOffscreenFromPNG "./assets/spritesheet.png" in
  retain_event @@
    React.E.map (fun sprite_sheet ->
        state.sprite_sheet_opt <- Some (sprite_sheet)
      ) event_sprite_sheet;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
        else
          state.look_direction <-
            match key with
            | KeyDownArrow -> Down
            | KeyUpArrow -> Up
            | KeyLeftArrow -> Left
            | KeyRightArrow -> Right
            | _ -> state.look_direction
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.timestamp = t; _ } ->
        match state.sprite_sheet_opt with
        | None ->
            ()
        | Some (sprite_sheet) ->
            let (w, h) = Canvas.getSize sprite_sheet in
            let step_size = Int64.of_int 50000 in
            let dt = Int64.sub t state.old_time in
            let speed =
              30.0 *. (Int64.to_float dt) /. (Int64.to_float step_size) in
            state.clock <- Int64.add state.clock dt;
            state.old_time <- t;
            if Int64.compare state.clock step_size > 0 then
              begin
                state.clock <- Int64.zero;
                state.animation_frame <- (state.animation_frame + 1) mod 4
              end;
            begin
              match state.look_direction with
              | Down -> state.y <- state.y +. speed
              | Up -> state.y <- state.y -. speed
              | Left -> state.x <- state.x -. speed
              | Right -> state.x <- state.x +. speed
            end;
            state.x <- clamp state.x 0.0 (1024.0 -. (float_of_int w) /. 4.0);
            state.y <- clamp state.y 0.0 (1024.0 -. (float_of_int h) /. 4.0);
            Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(1024.0, 1024.0);
            let pos_x = state.animation_frame * w / 4 in
            let pos_y = (int_of_look_dir state.look_direction) * h / 4 in
            Canvas.blit ~dst:c ~dpos:(int_of_float state.x,int_of_float state.y)
              ~src:sprite_sheet ~spos:(pos_x, pos_y) ~size:(w / 4, h / 4)
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
