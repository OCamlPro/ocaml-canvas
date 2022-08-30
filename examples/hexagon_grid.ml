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

  let c = Canvas.createFramed "Hexagon Grid"
            ~pos:(300, 200) ~size:(800, 696) in
  let hex_grid = Canvas.createOffscreenFromPNG "./assets/hexagon.png" in
  Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
  Canvas.setFillColor hex_grid Color.orange; 

  Canvas.show c;
  let counter = ref 0. in
  Backend.run (function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        true

    | Event.Frame { canvas = _; timestamp = _ } ->
        counter := !counter +. 3.;
        let g = Canvas.createRadialGradient hex_grid ~center1:(400., 300.) ~rad1:(!counter *. 0.4) ~center2:(400., 300.) ~rad2:(!counter)  in
        Gradient.addColorStop g Color.orange 0.0;
        Gradient.addColorStop g (Color.of_argb 64 0 0 255) 0.25;
        Gradient.addColorStop g (Color.of_argb 0 0 0 0) 0.5;
        Canvas.setFillColor c Color.black;
        Canvas.fillRect c ~pos:(0. , 0.) ~size:(800., 696.);
        Canvas.setFillGradient hex_grid g;
        Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
        Canvas.fillRect hex_grid ~pos:(400. -. !counter/. 2., 300. -. !counter/. 2.) ~size:(100., 100.);
        Canvas.fillRect hex_grid ~pos:(400. +. !counter/. 2., 300. +. !counter/. 2.) ~size:(100., 100.);
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:hex_grid ~spos:(0, 0) ~size:(800, 696);  
        true

    | _ ->
        false

    ) (function () ->
         Printf.printf "Goodbye !\n"
    )
