
open OcamlCanvas.V1

let () =

  Backend.init ();

  let c = Canvas.createFramed "Hello world"
            ~pos:(300, 200) ~size:(800, 500) in

  Canvas.setFillColor c Color.orange;
  Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(800.0, 500.0);

  Canvas.setStrokeColor c Color.blue;
  Canvas.setLineWidth c 10.0;
  Canvas.moveTo c (50.0, 50.0);
  Canvas.lineTo c (750.0, 50.0);
  Canvas.lineTo c (750.0, 450.0);
  Canvas.lineTo c (50.0, 450.0);
  Canvas.closePath c;
  Canvas.stroke c;

  Canvas.clearPath c;
  Canvas.moveTo c (150.0, 350.0);
  Canvas.bezierCurveTo c ~cp1:(350.0, -100.0)
    ~cp2:(450.0, 600.0) ~p:(650.0, 150.0);
  Canvas.stroke c;

  Canvas.show c;

  Backend.run (fun state -> function

    | Event.CanvasClosed { canvas = _; timestamp = _ }
    | Event.KeyAction { canvas = _; timestamp = _;
                        key = KeyEscape; state = Down; _ } ->
        Backend.stop ();
        state, true

    | _ ->
        state, false

    ) (fun _state -> ()) ()
