
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Hello world"
            ~pos:(300, 200) ~size:(800, 500) () in

  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 500.0);

  Canvas.show c;

  Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        Backend.stop ()
      ) Event.close;

  Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.key; char = _; flags = _ } } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

  Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.position; button } } ->
        let color =
          match button with
          | ButtonLeft -> Color.blue
          | ButtonRight -> Color.green
          | _ -> Color.red
        in
        Canvas.setFillColor c color;

        let center = Point.of_ints position in

        Canvas.clearPath c;
        Canvas.moveTo c center;
        Canvas.arc c ~center ~radius:10.0 ~theta1:0.0
          ~theta2:(2.0 *. Const.pi) ~ccw:false;
        Canvas.fill c ~nonzero:false;
      ) Event.button_down;

  Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        ()
      ) Event.frame;

  Backend.run (fun () -> ())
