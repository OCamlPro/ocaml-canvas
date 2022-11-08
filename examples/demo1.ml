
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Hello world"
            ~pos:(300, 200) ~size:(800, 500) () in

  Canvas.show c;

  let e =
    React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        Backend.stop ()
      ) Event.close
  in

  Backend.run (fun () -> ignore e)
