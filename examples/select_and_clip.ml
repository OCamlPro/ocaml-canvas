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
  let width = 800 and height = 600 in
  let c = Canvas.createFramed "Draw"
            ~pos:(300, 200) ~size:(width, height) in
  let draw = Canvas.createOffscreen ~size:(width, height) in
  Canvas.setFillColor c (Color.of_argb 0 0 0 0);
  Canvas.setStrokeColor c Color.white;
  Canvas.setStrokeColor draw Color.white;
  Canvas.setFillColor draw Color.white;
  Canvas.setLineJoin draw Round;
  Canvas.setLineCap draw RoundCap;
  Canvas.save draw;
  let a = Array.create_float 2 in
  Array.set a 0 20.0;
  Array.set a 1 20.0;
  Canvas.setLineDash c (a);
  Canvas.setFillColor draw Color.black;
  Canvas.show c;
  let selecting = ref false and selection = ref (Path.create ()) in
  let first_pos_x = ref (-1) and first_pos_y = ref (-1) in
  let m_pos_x = ref 0. and m_pos_y = ref 0. in
  let old_m_pos_x = ref (0. -. 1.) and old_m_pos_y = ref 0. in
  let pressing_button = ref false in
  let proper_path = ref false in
  let line_width = ref 20. in
  Backend.run (function
    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        if key = Event.KeyS then (
          first_pos_x := -1;
          first_pos_y := -1;
          selection := Path.create ();
          selecting := true;
          proper_path := false;
          Canvas.restore draw;
        );
        if key = Event.KeyD then (
          selection := Path.create ();
          selecting := false;
          proper_path := false;
          Canvas.restore draw;
        );
        if key = Event.KeyF then (
          if (!proper_path && not !selecting) then (
            Canvas.setFillColor draw Color.white;
            Canvas.fillRect draw ~pos:(0., 0.) ~size:(float_of_int width, float_of_int height);
          );
        );
        if key = Event.KeyUpArrow then (
            line_width := !line_width +. 4.;
        );
        if key = Event.KeyDownArrow then (
            if (!line_width > 4.) then (
                line_width := !line_width -. 4.;
          );
        );
        true
    
    | Event.MouseMove { canvas = _; timestamp = _;
                        position = (x,y)} ->
        m_pos_x := float_of_int x;
        m_pos_y := float_of_int y;
        true;
    
    | Event.ButtonAction {canvas = _; timestamp = _;
                          position = (pos_x, pos_y); button = ButtonLeft;
                          state = Down} ->
        pressing_button := true;
        old_m_pos_x := float_of_int pos_x;
        old_m_pos_y := float_of_int pos_y;
        if !selecting then ( 
          if (!first_pos_x < 0) then (
            first_pos_x := pos_x;
            first_pos_y := pos_y;
            Path.lineTo !selection (float_of_int pos_x, float_of_int pos_y);
          ) else if (abs (pos_x - !first_pos_x) < 15) && (abs (pos_y - !first_pos_y) < 15) then (
            Path.close !selection;
            selecting := false;
            proper_path := true;
            Canvas.save draw;
            Canvas.clipPath draw !selection ~nonzero:false;
          ) else
            Path.lineTo !selection (float_of_int pos_x, float_of_int pos_y);
        );
        true
    
    | Event.ButtonAction {canvas = _; timestamp = _;
                          position = _; button = ButtonLeft;
                          state = Up} ->
      pressing_button := false;
      true

    | Event.Frame { canvas = _; timestamp = t } ->
        if (!pressing_button && not !selecting) then (
            Canvas.clearPath draw;
            Canvas.setLineWidth draw !line_width;
            Canvas.moveTo draw (!old_m_pos_x, !old_m_pos_y);
            Canvas.lineTo draw (!m_pos_x, !m_pos_y);
            Canvas.stroke draw;
            old_m_pos_x := !m_pos_x;
            old_m_pos_y := !m_pos_y;
        );
        
        Canvas.setLineDashOffset c (Int64.to_float (Int64.neg t) /. (10000.));
        Canvas.setGlobalCompositeOperation c Copy;
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:draw ~spos:(0, 0) ~size:(width, height);  
        Canvas.setGlobalCompositeOperation c SourceOver;
        Canvas.strokePath c !selection;
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
