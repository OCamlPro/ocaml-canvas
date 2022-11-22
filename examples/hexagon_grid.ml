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
  mutable hex_grid_opt : Canvas.t option;
  mutable counter : float;
}

let state = {
  hex_grid_opt = None;
  counter = 0.0
}

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Hexagon Grid"
            ~pos:(300, 200) ~size:(800, 696) () in

  Canvas.show c;

  let event_hex_grid = Canvas.createOffscreenFromPNG "./assets/hexagon.png" in
  retain_event @@
    React.E.map (fun hex_grid ->
        Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
        Canvas.setFillColor hex_grid Color.orange;
        state.hex_grid_opt <- Some (hex_grid)
      ) event_hex_grid;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  retain_event @@
    React.E.map (fun _ ->
        match state.hex_grid_opt with
        | None ->
            ()
        | Some (hex_grid) ->
            state.counter <- state.counter +. 3.0;
            let g = Gradient.createRadial
                      ~center1:(400.0, 300.0) ~rad1:(state.counter *. 0.4)
                      ~center2:(400.0, 300.0) ~rad2:state.counter in
            Gradient.addColorStop g Color.orange 0.0;
            Gradient.addColorStop g (Color.of_argb 64 0 0 255) 0.25;
            Gradient.addColorStop g Color.transpBlack 0.5;
            Canvas.setFillColor c Color.black;
            Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 696.0);
            Canvas.setFillGradient hex_grid g;
            Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
            Canvas.fillRect hex_grid
              ~pos:(400.0 -. state.counter /. 2.0,
                    300.0 -. state.counter /. 2.0)
              ~size:(100.0, 100.0);
            Canvas.fillRect hex_grid
              ~pos:(400.0 +. state.counter /. 2.0,
                    300.0 +. state.counter /. 2.0)
              ~size:(100.0, 100.0);
            Canvas.blit ~dst:c ~dpos:(0, 0) ~src:hex_grid
              ~spos:(0, 0) ~size:(800, 696)) Event.frame;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
