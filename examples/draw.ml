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
  selecting : bool;
  selection : Path.t;
  first_pos : Point.t;
  m_pos : Point.t;
  old_m_pos : Point.t;
  pressing_button : bool;
  proper_path : bool;
  line_width : float;
  selected_color : Color.t;
  selecting_color : bool;
  color_map_opt : [`Offscreen] Canvas.t option;
}

let () =

  Backend.init ();

  let size_i = (800, 600) in
  let size_f = Point.of_ints size_i in

  let c = Canvas.createFramed "Draw" ~pos:(300, 200) ~size:size_i in
  Canvas.setFillColor c Color.transpBlack;
  Canvas.setStrokeColor c Color.white;
  Canvas.setLineDash c [| 20.0; 20.0 |];

  let draw = Canvas.createOffscreen ~size:size_i in
  Canvas.setStrokeColor draw Color.white;
  Canvas.setLineJoin draw Join.Round;
  Canvas.setLineCap draw Cap.Round;
  Canvas.save draw;
  Canvas.setFillColor draw Color.black;
  Canvas.fillRect draw ~pos:(0.0, 0.0) ~size:size_f;

  let p_color_map = Canvas.createOffscreenFromPNG "assets/colors.png" in
  ignore @@
    Promise.bind p_color_map (fun color_map ->
      Backend.postCustomEvent (CanvasLoaded (color_map));
      Promise.return ());

  Canvas.show c;

  let initial_state = {
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
    color_map_opt = None;
  }
  in

  Backend.run (fun state -> function

    | Event.CanvasClosed _
    | Event.KeyAction { key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | Event.Custom { payload = CanvasLoaded (color_map); _ } ->
        { state with color_map_opt = Some (color_map) }, true

    | Event.KeyAction { key; state = Down; _ }
          when not state.selecting_color ->
        let state =
          match key with
          | Event.KeyS ->
              Canvas.restore draw;
              { state with first_pos = (-1.0, -1.0);
                           selection = Path.create ();
                           selecting = true; proper_path = false }
          | Event.KeyD ->
              Canvas.restore draw;
              { state with selection = Path.create ();
                           selecting = false; proper_path = false }
          | Event.KeyF when state.proper_path && not state.selecting ->
              Canvas.setFillColor draw state.selected_color;
              Canvas.fillRect draw ~pos:(0.0, 0.0) ~size:size_f;
              state
          | Event.KeyUpArrow ->
              { state with line_width = state.line_width +. 4.0 }
          | Event.KeyDownArrow when state.line_width > 4.0 ->
              { state with line_width = state.line_width -. 4.0 }
          | Event.KeyC ->
              { state with selecting_color = true }
          | _ ->
              state
        in
        state, true

    | Event.MouseMove { position; _ } ->
        { state with m_pos = Point.of_ints position }, true

    | Event.ButtonAction { position; button = ButtonLeft; state = Down; _ }
          when state.selecting_color ->
        let selected_color =
          match state.color_map_opt with
          | Some (color_map) -> Canvas.getPixel color_map position
          | _ -> state.selected_color
        in
        { state with selected_color; selecting_color = false;
                     old_m_pos = Point.of_ints position }, true

    | Event.ButtonAction { position; button = ButtonLeft; state = Down; _ }
          when state.selecting ->
        let pos = Point.of_ints position in
        let state =
          if fst state.first_pos < 0.0 then
            begin
              Path.lineTo state.selection pos;
              { state with first_pos = pos }
            end
          else if Point.distance pos state.first_pos >= 15.0 then
            begin
              Path.lineTo state.selection pos;
              state
            end
          else
            begin
              Path.close state.selection;
              Canvas.save draw;
              Canvas.clipPath draw state.selection ~nonzero:false;
              { state with selecting = false; proper_path = true;
                           pressing_button = false }
            end
        in
        state, true

    | Event.ButtonAction { position; button = ButtonLeft; state = Down; _ } ->
        { state with pressing_button = true;
                     old_m_pos = Point.of_ints position }, true

    | Event.ButtonAction { button = ButtonLeft; state = Up; _ } ->
        { state with pressing_button = false }, true

    | Event.Frame { canvas = _; timestamp = t; _ } ->
        let state =
          if state.pressing_button &&
               not (state.selecting || state.selecting_color) then
            begin
              Canvas.clearPath draw;
              Canvas.setStrokeColor draw state.selected_color;
              Canvas.setLineWidth draw state.line_width;
              Canvas.moveTo draw state.old_m_pos;
              Canvas.lineTo draw state.m_pos;
              Canvas.stroke draw;
              { state with old_m_pos = state.m_pos }
            end
          else
            state
        in
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
        end;
        state, true

    | _ ->
        state, false

    ) (fun _state ->
         Printf.printf "Goodbye !\n"
    ) initial_state
