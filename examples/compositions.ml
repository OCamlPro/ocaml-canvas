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

let () =

  Backend.init ();

  let c = Canvas.createFramed "Compositions"
            ~pos:(300, 200) ~size:(400, 267) in
  let p_catImage = Canvas.createOffscreenFromPNG "./assets/dragon.png" in
  let p_spectrumImage = Canvas.createOffscreenFromPNG "./assets/fabric.png" in

  ignore @@
    Promise.bind p_catImage (fun catImage ->
        Promise.bind p_spectrumImage (fun spectrumImage ->
            Canvas.blit ~dst:c ~dpos:(0,0) ~src:spectrumImage
              ~spos:(0,0) ~size:(400,400);
            Canvas.setGlobalCompositeOperation c CompositeOp.ColorDodge;
            Canvas.blit ~dst:c ~dpos:(0,0) ~src:catImage
              ~spos:(0,0) ~size:(400,400);
            Canvas.show c;
            Promise.return ()));

  Backend.run (fun state -> function

    | Event.KeyAction { canvas = _; timestamp = _;
                        key; char = _; flags = _; state = Down } ->
        if key = Event.KeyEscape then
          Backend.stop ();
        state, true

    | Event.CanvasClosed { canvas = _; timestamp = _ } ->
        Backend.stop ();
        state, true

    | Event.Frame { canvas = _; timestamp = _ } ->
        state, true

    | _ ->
        state, false

    ) (function _state ->
         Printf.printf "Goodbye !\n"
    ) ()
