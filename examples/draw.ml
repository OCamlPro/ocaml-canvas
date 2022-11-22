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
  mutable selecting : bool;
  mutable selection : Path.t;
  mutable first_pos : Point.t;
  mutable m_pos : Point.t;
  mutable old_m_pos : Point.t;
  mutable pressing_button : bool;
  mutable proper_path : bool;
  mutable line_width : float;
  mutable selected_color : Color.t;
  mutable selecting_color : bool;
  mutable color_map_opt : Canvas.t option;
}

let state = {
  selecting = false;
  selection = Path.create ();
  first_pos = (-1.0, -1.0);
  m_pos = (0.0, 0.0);
  old_m_pos = (0.0 -. 1.0, 0.0);
  pressing_button = false;
  proper_path = false;
  line_width = 20.0;
  selected_color = Color.white;
  selecting_color = false;
  color_map_opt = None
}

let () =

  Backend.init ();

  let size_i = (800, 600) in
  let size_f = Point.of_ints size_i in

  let c = Canvas.createOnscreen ~title:"Draw" ~pos:(300, 200) ~size:size_i () in

  Canvas.show c;

  Canvas.setFillColor c Color.transpBlack;
  Canvas.setStrokeColor c Color.white;
  Canvas.setLineDash c [| 20.0; 20.0 |];

  let draw = Canvas.createOffscreen ~size:size_i () in
  Canvas.setStrokeColor draw Color.white;
  Canvas.setLineJoin draw Join.Round;
  Canvas.setLineCap draw Cap.Round;
  Canvas.save draw;
  Canvas.setFillColor draw Color.black;
  Canvas.fillRect draw ~pos:(0.0, 0.0) ~size:size_f;

  let event_color_map = Canvas.createOffscreenFromPNG "assets/colors.png" in
  retain_event @@
    React.E.map (fun color_map ->
        state.color_map_opt <- Some (color_map)
      ) event_color_map;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
        else if not state.selecting_color then
          match key with
          | KeyS ->
              Canvas.restore draw;
              state.first_pos <- (-1.0, -1.0);
              state.selection <- Path.create ();
              state.selecting <- true;
              state.proper_path <- false
          | KeyD ->
              Canvas.restore draw;
              state.selection <- Path.create ();
              state.selecting <- false;
              state.proper_path <- false
          | KeyF when state.proper_path && not state.selecting ->
              Canvas.setFillColor draw state.selected_color;
              Canvas.fillRect draw ~pos:(0.0, 0.0) ~size:size_f
          | KeyUpArrow ->
              state.line_width <- state.line_width +. 4.0
          | KeyDownArrow when state.line_width > 4.0 ->
              state.line_width <- state.line_width -. 4.0
          | KeyC ->
              state.selecting_color <- true
          | _ ->
              ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun { Event.data = position; _ } ->
        state.m_pos <- Point.of_ints position
      ) Event.mouse_move;

  retain_event @@
    React.E.map (fun { Event.data = { Event.position; button }; _ } ->
        match button with
        | ButtonLeft when state.selecting_color ->
            let selected_color =
              match state.color_map_opt with
              | Some (color_map) -> Canvas.getPixel color_map position
              | _ -> state.selected_color
            in
            state.selected_color <- selected_color;
            state.selecting_color <- false;
            state.old_m_pos <- Point.of_ints position
        | ButtonLeft when state.selecting ->
            let pos = Point.of_ints position in
            if fst state.first_pos < 0.0 then
              begin
                Path.lineTo state.selection pos;
                state.first_pos <- pos
              end
            else if Point.distance pos state.first_pos >= 15.0 then
              Path.lineTo state.selection pos
            else
              begin
                Path.close state.selection;
                Canvas.save draw;
                Canvas.clipPath draw state.selection ~nonzero:false;
                state.selecting <- false;
                state.proper_path <- true;
                state.pressing_button <- false
              end
        | ButtonLeft ->
            state.pressing_button <- true;
            state.old_m_pos <- Point.of_ints position
        | _ ->
            ()
      ) Event.button_down;

  retain_event @@
    React.E.map (fun { Event.data = { Event.position = _; button }; _ } ->
        match button with
        | ButtonLeft ->
            state.pressing_button <- false
        | _ ->
            ()
      ) Event.button_up;

  retain_event @@
    React.E.map (fun { Event.timestamp = t; _ } ->
        if state.pressing_button &&
             not (state.selecting || state.selecting_color) then
          begin
            Canvas.clearPath draw;
            Canvas.setStrokeColor draw state.selected_color;
            Canvas.setLineWidth draw state.line_width;
            Canvas.moveTo draw state.old_m_pos;
            Canvas.lineTo draw state.m_pos;
            Canvas.stroke draw;
            state.old_m_pos <- state.m_pos
          end;
        Canvas.setLineDashOffset c (Int64.to_float (Int64.neg t) /. (10000.0));
        Canvas.setGlobalCompositeOperation c Copy;
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:draw ~spos:(0, 0) ~size:size_i;
        Canvas.setGlobalCompositeOperation c SourceOver;
        Canvas.strokePath c state.selection;
        begin
          match state.selecting_color, state.color_map_opt with
          | true, Some (color_map) ->
              Canvas.setGlobalCompositeOperation c Copy;
              Canvas.blit ~dst:c ~dpos:(0, 0) ~src:color_map
                ~spos:(0, 0) ~size:size_i
          | _ ->
              ()
        end
      ) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
