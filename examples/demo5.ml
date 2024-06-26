
open OcamlCanvas.V1

let state = ref (0.0, Int64.zero)

let () =

  Backend.init ();

  let c = Canvas.createOnscreen ~title:"Hello world"
            ~pos:(300, 200) ~size:(800, 500) () in

  Canvas.show c;

    Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
        Backend.stop ()
      ) Event.close;

    Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = _;
                       data = { Event.key; char = _; flags = _ } } ->
        if key = KeyEscape then
          Backend.stop ()
      ) Event.key_down;

    Event.hold @@ React.E.map (fun { Event.canvas = _; timestamp = t; data = () } ->
        let theta, last = !state in

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

        state := (theta, t)
      ) Event.frame;

  Backend.run (fun () -> ())
