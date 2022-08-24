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

  let c = Canvas.createFramed "Spritesheet"
            ~pos:(300, 200) ~size:(1024, 1024) in
  let sprite_sheet = Canvas.createOffscreenFromPNG "./assets/spritesheet.png" in
  let look_direction = ref 0 and animation_frame = ref 0 and clock = ref Int64.zero in
  let old_time = ref Int64.zero in
  let x = ref 300. and y = ref 300. in
  Canvas.setFillColor c Color.white;
  Canvas.show c;
  Backend.run (function
    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        if key = Event.KeyDownArrow then
          look_direction := 0;
        if key = Event.KeyUpArrow then
          look_direction := 1;
        if key = Event.KeyLeftArrow then
          look_direction := 2;
        if key = Event.KeyRightArrow then
          look_direction := 3;
        true

    | Event.Frame { canvas = _; timestamp = t } ->
        let dt = Int64.sub t !old_time in
        let step_size = Int64.of_int 50000 in
        let speed = 30. *. (Int64.to_float dt) /. (Int64.to_float step_size)  in
        clock := Int64.add !clock dt;
        old_time := t;
        let (w,h) = Canvas.getSize sprite_sheet in
        if (Int64.compare !clock step_size > 0) then
             (clock := Int64.zero;
              animation_frame := (!animation_frame + 1) mod 4;);
        if (!look_direction = 0) then 
           (y := !y +. speed);
        if (!look_direction = 1) then 
           (y := !y -. speed);
        if (!look_direction = 2) then 
           (x := !x -. speed);
        if (!look_direction = 3) then 
           (x := !x +. speed);
        if (!x < 0.) then x := 0.;
        if (!x > 1024. -. (float_of_int w) /. 4.) then (x := 1024. -. (float_of_int w) /. 4.);
        if (!y < 0.) then y := 0.;
        if (!y > 1024. -. (float_of_int h) /. 4.) then (y := 1024. -. (float_of_int h) /. 4.);

        Canvas.fillRect c ~pos:(0., 0.) ~size:(1024., 1024.);
        let pos_x = !animation_frame * w / 4 and pos_y = !look_direction * h / 4 in
        Canvas.blit ~dst:c ~dpos:(int_of_float !x,int_of_float !y)
                	~src:sprite_sheet ~spos:(pos_x, pos_y) 
                    ~size:(w/4, h/4);
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
