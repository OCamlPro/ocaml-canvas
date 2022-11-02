
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

  Canvas.setFont c "Liberation Sans" ~size:54.0
    ~slant:Font.Roman ~weight:Font.bold;

  Canvas.setFillColor c (Color.of_rgb 0 192 0);
  Canvas.fillText c "Hello world !" (200.0, 250.0);

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

  Backend.run (fun () -> ignore e1; ignore e2)
