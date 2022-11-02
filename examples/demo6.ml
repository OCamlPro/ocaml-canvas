
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(800, 500) in

  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 500.0);

  Canvas.show c;

  let e1 =
    React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        Backend.stop ()
      ) Event.close
  in

  let e2 =
    React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.key; char = _; flags = _ } } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down
  in

  let e3 =
    React.E.map (fun { Event.canvas = _; timestamp = _;
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
      ) Event.button_down
  in

  let e4 =
    React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        ()
      ) Event.frame
  in

  Backend.run (fun () -> ignore e1; ignore e2; ignore e3; ignore e4)
