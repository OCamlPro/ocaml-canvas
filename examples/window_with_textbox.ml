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

type state = {
  mutable active : int;
  mutable st1 : string;
  mutable st2 : string;
  mutable cursor_1 : int;
  mutable cursor_2 : int;
  mutable offset_1 : int;
  mutable offset_2 : int;
  mutable char_size : float;
}

let state = {
  active = 0;
  st1 = "";
  st2 = "";
  cursor_1 = 0;
  cursor_2 = 0;
  offset_1 = 0;
  offset_2 = 0;
  char_size = 14.0;
}

let in_rect (x1, y1) (posx, posy) (sizex, sizey) =
  (float_of_int x1 >= posx) && (float_of_int x1 <= posx +. sizex) &&
  (float_of_int y1 >= posy) && (float_of_int y1 <= posy +. sizey)

let is_digit = function '0' .. '9' -> true | _ -> false
let is_alpha = function 'a' .. 'z' | 'A' .. 'Z' -> true | _ -> false

let insert c s pos =
  let left = String.sub s 0 pos in
  let right = String.sub s pos (String.length s - pos) in
  String.concat "" [ left; (String.make 1 (Uchar.to_char c)); right ]

let backspace s pos =
  let left = String.sub s 0 pos in
  let right = String.sub s (pos + 1) (String.length s - pos - 1) in
  left ^ right

let clamp v low high =
  if v < low then low
  else if v > high then high
  else v

let () =

  Backend.init ();

  let ccm = Canvas.createFramed "文章入力欄" ~pos:(500, 500) ~size:(300, 200) in
  Canvas.setFillColor ccm Color.white;
  Canvas.fillRect ccm ~pos:(0.0, 0.0) ~size:(500.0, 500.0);
  Canvas.setFillColor ccm Color.black;
  Canvas.setFont ccm "Monospace" ~size:18.0
    ~slant:Font.Roman ~weight:Font.regular;
  Canvas.fillText ccm "Width" (70.0, 75.0);
  Canvas.fillText ccm "Height" (64.0, 125.0);
  Canvas.setStrokeColor ccm Color.black;
  Canvas.setLineWidth ccm 2.0;
  Canvas.strokeRect ccm ~pos: (150.0, 52.0) ~size:(100.0, 30.0);
  Canvas.strokeRect ccm ~pos: (150.0, 102.0) ~size:(100.0, 30.0);
  Canvas.show ccm;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.canvas = _;
                       data = { Event.key; char = c; _ }; _ } ->
        match key with
        | KeyEscape ->
            Backend.stop ()
        | KeyTab ->
            let active =
              match state.active with
              | 1 -> 2
              | 2 -> 1
              | _ -> 1
            in
            state.active <- active
        | _ when state.active > 0 ->
            let string, cursor, offset =
              match state.active with
              | 1 -> state.st1, state.cursor_1, state.offset_1
              | _ -> state.st2, state.cursor_2, state.offset_2
            in
            let string, cursor, offset =
              if Uchar.is_char c &&
                   (is_digit (Uchar.to_char c) ||
                      is_alpha (Uchar.to_char c)) then
                let string = insert c string cursor in
                let cursor = cursor + 1 in
                let offset =
                  if cursor < offset || cursor > offset + 7 then offset + 1
                  else offset
                in
                string, cursor, offset
              else if key = KeyBackspace && cursor > 0 then
                let string = backspace string (cursor - 1) in
                let cursor = cursor - 1 in
                let offset =
                  if offset > 0 then offset - 1
                  else offset
                in
                string, cursor, offset
              else if key = KeyLeftArrow && cursor > 0 then
                let cursor = cursor - 1 in
                let offset =
                  if cursor < offset || cursor > offset + 7 then offset - 1
                  else offset in
                string, cursor, offset
              else if key = KeyRightArrow &&
                        cursor < String.length string then
                let cursor = cursor + 1 in
                let offset =
                  if cursor < offset || cursor > offset + 7 then offset + 1
                  else offset
                in
                string, cursor, offset
              else
                string, cursor, offset
            in
            begin
              match state.active with
              | 1 ->
                  state.st1 <- string;
                  state.cursor_1 <- cursor;
                  state.offset_1 <- offset
              | _ ->
                  state.st2 <- string;
                  state.cursor_2 <- cursor;
                  state.offset_2 <- offset
            end
        | _ ->
            ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.data = { Event.position = (x, y); button }; _ } ->
        if button = ButtonLeft then
          let active, cursor_1, cursor_2 =
            if in_rect (x, y) (152.0, 54.0) (97.0, 27.0) then
              let cursor_target =
                int_of_float (((float_of_int x) -. 152.0) /.
                                state.char_size) + state.offset_1 in
              1, clamp cursor_target 0 (String.length state.st1), state.cursor_2
            else if in_rect (x, y) (152.0, 104.0) (97.0, 27.0) then
              let cursor_target =
                int_of_float (((float_of_int x) -. 152.0) /.
                                state.char_size) + state.offset_2 in
              2, state.cursor_1, clamp cursor_target 0 (String.length state.st2)
            else
              0, state.cursor_1, state.cursor_2
          in
          state.active <- active;
          state.cursor_1 <- cursor_1;
          state.cursor_2 <- cursor_2
      ) Event.button_down;

  retain_event @@
    React.E.map (fun _ ->

        Canvas.setFillColor ccm Color.white;
        Canvas.fillRect ccm ~pos:(152.0, 54.0) ~size:(97.0, 27.0);
        Canvas.fillRect ccm ~pos:(152.0, 104.0) ~size:(97.0, 27.0);
        Canvas.setFillColor ccm Color.black;

        Canvas.save ccm;
        Canvas.clearPath ccm;
        Canvas.rect ccm ~pos:(152.0, 54.0) ~size:(95.0, 27.0);
        Canvas.clip ccm ~nonzero:false;
        Canvas.translate ccm
          (0.0 -. state.char_size *. (float_of_int state.offset_1), 0.0);
        Canvas.fillText ccm state.st1 (155.0, 75.0);
        if state.active = 1 then
          Canvas.fillRect ccm
            ~pos:(155.0 +. state.char_size *. (float_of_int state.cursor_1),
                  27.0)
            ~size:(2.0, 55.0);
        Canvas.restore ccm;

        Canvas.save ccm;
        Canvas.clearPath ccm;
        Canvas.rect ccm ~pos:(152.0, 104.0) ~size:(95.0, 27.0);
        Canvas.clip ccm ~nonzero:false;
        Canvas.translate ccm
          (0.0 -. state.char_size *. (float_of_int state.offset_2), 0.0);
        Canvas.fillText ccm state.st2 (155.0, 125.0);
        if state.active = 2 then
          Canvas.fillRect ccm
            ~pos:(155.0 +. state.char_size *. (float_of_int state.cursor_2),
                  85.0)
            ~size:(2.0, 55.0);
        Canvas.restore ccm

      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
