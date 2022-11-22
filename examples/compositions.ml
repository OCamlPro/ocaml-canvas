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
  mutable cat_image_opt : Canvas.t option;
  mutable spectrum_image_opt : Canvas.t option;
}

let state = {
  cat_image_opt = None;
  spectrum_image_opt = None;
}

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Compositions"
            ~pos:(300, 200) ~size:(400, 267) () in

  let event_catImage = Canvas.createOffscreenFromPNG "./assets/dragon.png" in
  let event_spectrumImage =
    Canvas.createOffscreenFromPNG "./assets/fabric.png" in

  let try_blit () =
    match state.cat_image_opt, state.spectrum_image_opt with
    | Some (cat_image), Some (spectrum_image) ->
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:spectrum_image
          ~spos:(0, 0) ~size:(400, 400);
        Canvas.setGlobalCompositeOperation c CompositeOp.ColorDodge;
        Canvas.blit ~dst:c ~dpos:(0, 0) ~src:cat_image
          ~spos:(0, 0) ~size:(400, 400);
        Canvas.show c
    | _ ->
        ()
  in

  retain_event @@
    React.E.map (fun catImage ->
        state.cat_image_opt <- Some (catImage);
        try_blit ()
      ) event_catImage;

  retain_event @@
    React.E.map (fun spectrumImage ->
        state.spectrum_image_opt <- Some (spectrumImage);
        try_blit ()
      ) event_spectrumImage;

  retain_event @@
    React.E.map (fun _ ->
        Backend.stop ()
      ) Event.close;

  retain_event @@
    React.E.map (fun { Event.data = { Event.key; _ }; _ } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  Backend.run (fun () ->
      clear_events ();
      Printf.printf "Goodbye !\n")
