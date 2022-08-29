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

let in_rect (x1,y1) (posx, posy) (sizex, sizey) = 
  (float_of_int x1 >= posx) && (float_of_int x1 <= posx +. sizex) &&
  (float_of_int y1 >= posy) && (float_of_int y1 <= posy +. sizey)

let is_digit = function '0' .. '9' -> true | _ -> false

let insert c s pos =
  let left = String.sub s 0 pos and right = String.sub s pos (String.length s - pos) in
  String.cat (String.cat left (String.make 1 (Uchar.to_char c)))  right

let backspace s pos = 
  let left = String.sub s 0 pos and right = String.sub s (pos + 1) (String.length s - pos - 1) in
  String.cat left right

let () =

  Backend.(init default_options);

  let ccm = Canvas.createFramed "Simple Image Manipulation Program"
  ~pos:(500, 500) ~size:(300, 200) in
  Canvas.setFillColor ccm Color.white;
  Canvas.fillRect ccm ~pos:(0., 0.) ~size:(500., 500.);
  Canvas.setFillColor ccm Color.black;
  Canvas.setFont ccm "Monospace" ~size:18.0 ~slant:Font.Roman ~weight:Font.regular;
  Canvas.fillText ccm "Width" (70., 75.);
  Canvas.fillText ccm "Height" (64., 125.);
  Canvas.setStrokeColor ccm Color.black;
  Canvas.setLineWidth ccm 2.;
  Canvas.strokeRect ccm ~pos: (150., 52.) ~size:(100., 30.);
  Canvas.strokeRect ccm ~pos: (150., 102.) ~size:(100., 30.);
  Canvas.show ccm;

  let active = ref 0 in 
  let st1 = ref "" and st2 = ref "" in
  let cursor_1 = ref 0 and cursor_2 = ref 0 in
  let offset_1 = ref 0 and offset_2 = ref 0 in
  let char_size = 12. in 

  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = c; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        if (key = Event.KeyTab) then (
          if (!active = 0) then active := 1;
          if (!active > 0) then (
            active := 3 - !active;
          );
        );
        if (!active > 0) then
        (
          let target_string = match !active with
            | 1 -> st1
            | _ -> st2 in
          let target_cursor = match !active with
            | 1 -> cursor_1
            | _ -> cursor_2 in
          let target_offset = match !active with
            | 1 -> offset_1
            | _ -> offset_2 in
          if (Uchar.is_char c && is_digit (Uchar.to_char c)) then (
            target_string := insert c !target_string !target_cursor;
            target_cursor := !target_cursor + 1;
            if (!target_cursor < !target_offset || !target_cursor > !target_offset + 7) then
              target_offset := !target_offset + 1;
          )
          else if (key = Event.KeyBackspace && !target_cursor > 0) then ( 
            target_string := backspace !target_string (!target_cursor - 1);
            target_cursor := !target_cursor - 1;
            if (!target_offset > 0) then
              target_offset := !target_offset - 1;
          )
          else if (key = Event.KeyLeftArrow && !target_cursor > 0) then (
            target_cursor := !target_cursor - 1;
            if (!target_cursor < !target_offset || !target_cursor > !target_offset + 7) then
              target_offset := !target_offset - 1;
          )
          else if (key = Event.KeyRightArrow && !target_cursor < (String.length !target_string)) then (
            target_cursor := !target_cursor + 1;
            if (!target_cursor < !target_offset || !target_cursor > !target_offset + 7) then
              target_offset := !target_offset + 1; 
          )
        );
        true
    
    | Event.ButtonAction {canvas = _; timestamp = _;
        position = (pos_x, pos_y); button = ButtonLeft;
        state = Down} ->
        
        if (in_rect (pos_x, pos_y) (152., 54.) (97., 27.)) then
          active := 1
        else if (in_rect (pos_x, pos_y) (152., 104.) (97., 27.)) then
          active := 2
        else
          active := 0;        
        true

    | Event.Frame { canvas = _; timestamp = _ } ->
        Canvas.setFillColor ccm Color.white;
        Canvas.fillRect ccm ~pos:(152., 54.) ~size:(97., 27.);
        Canvas.fillRect ccm ~pos:(152., 104.) ~size:(97., 27.);
        Canvas.setFillColor ccm Color.black;
        Canvas.save ccm;
        Canvas.clearPath ccm;
        Canvas.rect ccm ~pos:(152., 54.) ~size:(95., 27.);
        Canvas.clip ccm ~nonzero:false;
        Canvas.translate ccm (0. -. char_size *. (float_of_int !offset_1), 0.);
        if (!active = 1) then
          Canvas.fillRect ccm ~pos:(155. +. char_size *. (float_of_int !cursor_1), 27.) ~size:(2., 55.);
        Canvas.fillText ccm !st1 (155., 75.);
        Canvas.restore ccm;
        Canvas.save ccm;
        Canvas.clearPath ccm;
        Canvas.rect ccm ~pos:(152., 104.) ~size:(95., 27.);
        Canvas.clip ccm ~nonzero:false;
        Canvas.translate ccm (0. -. char_size *. (float_of_int !offset_2), 0.);
        Canvas.fillText ccm !st2 (155., 125.);
        if (!active = 2) then
          Canvas.fillRect ccm ~pos:(155. +. char_size *. (float_of_int !cursor_2), 85.) ~size:(2., 55.);
        Canvas.restore ccm;
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )



