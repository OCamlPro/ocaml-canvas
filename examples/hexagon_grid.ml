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

let () =

  Backend.(init default_options);

  let c = Canvas.createFramed "Hexagon Grid"
            ~pos:(300, 200) ~size:(800, 696) in

  let p_hex_grid = Canvas.createOffscreenFromPNG "./assets/hexagon.png" in
  ignore @@
    Promise.bind p_hex_grid (fun hex_grid ->
      Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
      Canvas.setFillColor hex_grid Color.orange;
      Backend.sendCustomEvent (CanvasLoaded (hex_grid));
      Promise.return ());

  Canvas.show c;

  Backend.run (fun ((hex_grid_opt, counter) as state) -> function

    | Event.CanvasClosed _
    | Event.KeyAction { key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | Event.Custom { payload = CanvasLoaded (hex_grid); _ } ->
        (Some (hex_grid), counter), true

    | Event.Frame _ when fst state <> None ->
        let hex_grid =
          match fst state with
          | Some (hex_grid) -> hex_grid
          | None -> assert false
        in
        let counter = counter +. 3.0 in
        let g = Canvas.createRadialGradient hex_grid
                  ~center1:(400.0, 300.0) ~rad1:(counter *. 0.4)
                  ~center2:(400.0, 300.0) ~rad2:counter in
        Gradient.addColorStop g Color.orange 0.0;
        Gradient.addColorStop g (Color.of_argb 64 0 0 255) 0.25;
        Gradient.addColorStop g Color.transpBlack 0.5;
        Canvas.setFillColor c Color.black;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 696.0);
        Canvas.setFillGradient hex_grid g;
        Canvas.setGlobalCompositeOperation hex_grid SourceAtop;
        Canvas.fillRect hex_grid
          ~pos:(400.0 -. counter /. 2.0, 300.0 -. counter /. 2.0)
          ~size:(100.0, 100.0);
        Canvas.fillRect hex_grid
          ~pos:(400.0 +. counter /. 2.0, 300.0 +. counter /. 2.0)
          ~size:(100.0, 100.0);
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:hex_grid
          ~spos:(0, 0) ~size:(800, 696);
        (hex_grid_opt, counter), true

    | _ ->
        state, false

    ) (fun _state ->
         Printf.printf "Goodbye !\n"
    ) (None, 0.0)
