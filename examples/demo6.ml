
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(800, 500) in

  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 500.0);

  Canvas.show c;

  Backend.run (fun state -> function

    | Event.CanvasClosed { canvas = _; timestamp = _ }
    | Event.KeyAction { canvas = _; timestamp = _;
                        key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | Event.ButtonAction { canvas = _; timestamp = _; position;
                           button = b; state = Down; _ } ->

        let color =
          match b with
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

        state, true

    | Event.Frame { canvas = _; timestamp = _; _ } ->
        state, true

    | _ ->
        state, false

    ) (fun _state -> ()) ()
