
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(800, 500) in

  Canvas.show c;

  Backend.run (fun state event ->

    match event with

    | Event.CanvasClosed { canvas = _; timestamp = _ } ->
        Backend.stop ();
        state, true

    | _ ->
        state, false

    ) (fun _state -> ()) ()
