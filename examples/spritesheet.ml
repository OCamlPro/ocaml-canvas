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

type Event.payload += CanvasLoaded of [`Offscreen] Canvas.t

type state = {
  sprite_sheet_opt : [`Offscreen] Canvas.t option;
  look_direction : int;
  animation_frame : int;
  clock : Int64.t;
  old_time : Int64.t;
  x : float;
  y : float;
}

let clamp v low high =
  if v < low then low
  else if v > high then high
  else v

let () =

  Backend.init ();

  let c = Canvas.createFramed "Spritesheet"
            ~pos:(300, 200) ~size:(1024, 1024) in
  Canvas.setFillColor c Color.white;

  let p_sprite_sheet =
    Canvas.createOffscreenFromPNG "./assets/spritesheet.png" in
  ignore @@
    Promise.bind p_sprite_sheet (fun sprite_sheet ->
      Backend.postCustomEvent (CanvasLoaded (sprite_sheet));
      Promise.return ());

  Canvas.show c;

  let initial_state = {
    sprite_sheet_opt = None;
    look_direction = 0;
    animation_frame = 0;
    clock = Int64.zero;
    old_time = Int64.zero;
    x = 300.0;
    y = 300.0;
  }
  in

  Backend.run (fun state -> function

    | Event.CanvasClosed _
    | Event.KeyAction { key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | Event.KeyAction { key; state = Down; _ } ->
        let look_direction =
          match key with
          | Event.KeyDownArrow -> 0
          | Event.KeyUpArrow -> 1
          | Event.KeyLeftArrow -> 2
          | Event.KeyRightArrow -> 3
          | _ -> state.look_direction
        in
        { state with look_direction }, true

    | Event.Custom { payload = CanvasLoaded (sprite_sheet); _ } ->
        { state with sprite_sheet_opt = Some (sprite_sheet) }, true

    | Event.Frame { timestamp = t; _ } when state.sprite_sheet_opt <> None ->
        let sprite_sheet =
          match state.sprite_sheet_opt with
          | Some (sprite_sheet) -> sprite_sheet
          | None -> assert false
        in
        let dt = Int64.sub t state.old_time in
        let step_size = Int64.of_int 50000 in
        let speed = 30.0 *. (Int64.to_float dt) /. (Int64.to_float step_size) in
        let clock = Int64.add state.clock dt in
        let old_time = t in
        let (w, h) = Canvas.getSize sprite_sheet in
        let clock, animation_frame =
          if Int64.compare clock step_size > 0 then
            Int64.zero, (state.animation_frame + 1) mod 4
          else
            clock, state.animation_frame
        in
        let x =
          if state.look_direction = 2 then state.x -. speed
          else if state.look_direction = 3 then state.x +. speed
          else state.x
        in
        let y =
          if state.look_direction = 0 then state.y +. speed
          else if state.look_direction = 1 then state.y -. speed
          else state.y
        in
        let x = clamp x 0.0 (1024.0 -. (float_of_int w) /. 4.0) in
        let y = clamp y 0.0 (1024.0 -. (float_of_int h) /. 4.0) in
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(1024.0, 1024.0);
        let pos_x = animation_frame * w / 4 in
        let pos_y = state.look_direction * h / 4 in
        Canvas.blit ~dst:c ~dpos:(int_of_float x,int_of_float y)
          ~src:sprite_sheet ~spos:(pos_x, pos_y) ~size:(w / 4, h / 4);
        { state with animation_frame; clock; old_time; x; y }, true

    | _ ->
        state, false

    ) (fun _state ->
         Printf.printf "Goodbye !\n"
    ) initial_state
