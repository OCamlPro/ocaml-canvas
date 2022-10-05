
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(800, 500) in

  Canvas.show c;

  Backend.run (fun ((theta, last) as state) -> function

    | Event.CanvasClosed { canvas = _; timestamp = _ }
    | Event.KeyAction { canvas = _; timestamp = _;
                        key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | Event.Frame { canvas = _; timestamp = t; _ } ->

        let theta = theta +. (Int64.to_float (Int64.sub t last)) *. -0.000005 in

        Canvas.setFillColor c Color.orange;
        Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 500.0);

        Canvas.save c;

        Canvas.translate c (400.0, 250.0);
        Canvas.rotate c theta;
        Canvas.translate c (-400.0, -250.0);

        Canvas.setStrokeColor c Color.blue;
        Canvas.setLineWidth c 10.0;
        Canvas.clearPath c;
        Canvas.moveTo c (150.0, 350.0);
        Canvas.bezierCurveTo c ~cp1:(350.0, -100.0)
          ~cp2:(450.0, 600.0) ~p:(650.0, 150.0);
        Canvas.stroke c;

        Canvas.restore c;

        (theta, t), true

    | _ ->
        state, false

    ) (fun _state -> ()) (0.0, Backend.getCurrentTimestamp ())
