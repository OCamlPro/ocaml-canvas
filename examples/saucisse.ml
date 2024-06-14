open OcamlCanvas.V1

let option_iter f o = match o with None -> () | Some e -> f e

let sw = 800
let sh = 800

let base_y = 300.
let p1 : Point.t = (200., base_y)
let p2 : Point.t = (600., base_y)

let oeuil_y = base_y -. 200.

let o1 = (300., oeuil_y)
let o2 = (500., oeuil_y)

let saucisse_width = 30.
let saucisse_color = Color.orange

let mid = Point.barycenter 1. p1 1. p2

let p3 : Point.t ref = ref (fst mid, 300.)

let () =
  Backend.init ()

let c =
  Canvas.createOnscreen ~title:"Saucisse"
    ~pos:(0, 0) ~size:(sw, sh) ()

let rot (x, y) = (-.y, x)

let (++) (x1, y1) (x2, y2) = x1 +. x2, y1 +. y2
let (--) (x1, y1) (x2, y2) = x1 -. x2, y1 -. y2
let normalize (x, y) =
  let norm = sqrt (x *. x +. y *. y) in
  x /. norm, y /. norm
let dir p1 p2 =
  normalize (p2 -- p1)
let scale v (x, y) =
  v *. x, v *. y

let disc ?color ?gradient center radius =
  option_iter (Canvas.setFillColor c) color;
  option_iter (Canvas.setFillGradient c) gradient;
  Canvas.clearPath c;
  Canvas.arc c ~center
    ~radius ~theta1:0.0 ~theta2:(2.0 *. Const.pi) ~ccw:false;
  Canvas.fill c ~nonzero:false

let circle ~color center radius =
  Canvas.setStrokeColor c color;
  Canvas.clearPath c;
  Canvas.arc c ~center
    ~radius ~theta1:0.0 ~theta2:(2.0 *. Const.pi) ~ccw:false;
  Canvas.stroke c

let point ?(color=Color.red) center =
  disc ~color center 5.0

let taille_oeuil = 30.
let taille_pupille = 10.
let cursor_z = 100.

let gray n = Color.of_rgb n n n

let sourcil pos dir ratio =
  let w = 50. in
  let p0 = pos ++ (-.w, 0.) in
  let p2 = pos ++ (w, 0.) in
  let dy =
    ratio *. 20.
  in
  let p0, p2 =
    if dir then
      p0, p2 ++ (0., dy)
    else
      p0 ++ (0., dy), p2
  in
  let ratio = if dir then ratio else 1. -. ratio in
  let b = Point.barycenter ratio p0 (1. -. ratio) p2 in
  let p1 = b ++ (0., -.20.) in
  let p3 = b ++ (0., -.5.) in
  let pa = Path.create () in
  Path.moveTo pa p0;
  Path.lineTo pa p1;
  Path.lineTo pa p2;
  Path.lineTo pa p3;
  Path.close pa;
  Canvas.setFillColor c Color.black;
  Canvas.setStrokeColor c Color.black;
  Canvas.fillPath c pa ~nonzero:false;
  Canvas.strokePath c pa;
  ()

let oeuil pos dir is_left sourcil_ratio =
  let g =
    Gradient.createRadial
      ~center1:(pos ++ (-.15., 10.))
      ~rad1:1.
      ~center2:pos
      ~rad2:30.
  in
  Gradient.addColorStop g Color.white 0.;
  Gradient.addColorStop g (gray 235) 0.2;
  Gradient.addColorStop g (gray 230) 0.85;
  Gradient.addColorStop g (gray 100) 1.;
  disc ~gradient:g pos taille_oeuil;
  circle ~color:Color.black pos taille_oeuil;
  let pup =
    let (x, y) = dir -- pos in
    let ax = atan2 cursor_z x in
    let ay = atan2 cursor_z y in
    let x = cos ax in
    let y = cos ay in
    let o = (x, y) in
    scale 20. o ++ pos
  in
  disc ~color:Color.black pup taille_pupille;
  sourcil (pos -- (0., 50.)) is_left sourcil_ratio

let controls a b =
  let v = scale saucisse_width (dir a b) in
  let p1 = b ++ rot v in
  let p2 = b ++ v ++ rot v in
  let p3 = b ++ v in
  let p4 = b ++ v -- rot v in
  let p5 = b -- rot v in
  v, p1, p2, p3, p4, p5

let bout path a b =
  let _v, _p1, p2, p3, p4, p5 = controls a b in
  Path.arcTo path ~p1:p2 ~p2:p3 ~radius:saucisse_width;
  Path.arcTo path ~p1:p4 ~p2:p5 ~radius:saucisse_width;
  ()

let bouboule_radius = 10.

let bouboule v p =
  let v = normalize v in
  let center = p ++ (scale (saucisse_width +. bouboule_radius) v) in
  disc ~color:saucisse_color center bouboule_radius;
  circle ~color:Color.black center bouboule_radius

let tri v p =
  let v = normalize v in
  let center = p ++ (scale (saucisse_width +. bouboule_radius) v) in
  let size_bout = 30. in
  let p1 = center ++ (scale size_bout v) ++ (scale 15. (rot v)) in
  let p2 = center ++ (scale size_bout v) -- (scale 15. (rot v)) in
  let pa = Path.create () in
  Path.moveTo pa center;
  Path.lineTo pa p1;
  Path.lineTo pa p2;
  Path.close pa;
  Canvas.setFillColor c saucisse_color;
  Canvas.fillPath c pa ~nonzero:false;
  Canvas.setStrokeColor c Color.black;
  Canvas.strokePath c pa;

  bouboule v p;
  ()

module F = struct
  type 'a t = {
    c : 'a list;
    s : int;
  }

  let make v s = { c = [v]; s }

  let push v s =
    let l = List.length s.c in
    let c =
      if l >= s.s then
        List.rev (List.tl (List.rev s.c))
      else
        s.c
    in
    { s with c = v :: c }

  let mean s =
    List.fold_left (fun acc v -> acc +. v) 0. s.c /. (float s.s)
end

let time () =
  (Int64.to_float @@ Backend.getCurrentTimestamp ()) /. 1_000_000.

let last_frame_time = ref (time ())
let frame_time : float F.t ref = ref (F.make 100. 30)
let push_time t =
  let dt = t -. !last_frame_time in
  last_frame_time := t;
  frame_time := F.push dt !frame_time
let fps () = 1. /. F.mean !frame_time

let draw () =

  let t = time () in

  let truc = 20. in

  let p1 = p1 ++ (truc *. 2.1 *. sin (3.1 *. t) , truc *. 2. *. cos (4. *. t)) in
  let p2 = p2 ++ (truc *. 2.1 *. sin (5.1 *. t) , truc *. 2.3 *. cos (4.2 *. t)) in

  let p3 = !p3 in
  let p3_oeuil = p3 in
  let p3 =
    let o = 40. in
    let x = max (fst p1 +. o) (min (fst p2 -. o) (fst p3)) in
    (x, snd p3)
  in
  Canvas.setFillColor c saucisse_color;
  let path = Path.create () in
  let v1, x, _, _, _, a = controls p3 p1 in
  Path.moveTo path x;
  bout path p3 p1;
  let v2, b, _, _, _, a2 = controls p3 p2 in
  let v = (0., saucisse_width) in

  let ratio = 0.8 +. 0.3 *. sin (2. *. t) in

  let pos a cont =
    a ++ (scale ratio (cont -- a))
  in

  let d = p3 ++ v in
  Path.bezierCurveTo path
    ~cp1:(pos a d)
    ~cp2:(pos b d)
    ~p:b;

  bout path p3 p2;

  let u = p3 -- v in
  Path.bezierCurveTo path
    ~cp1:(pos a2 u)
    ~cp2:(pos x u)
    ~p:x;
  Path.close path;
  Canvas.setFillColor c saucisse_color;
  Canvas.fillPath c path ~nonzero:false;
  Canvas.setStrokeColor c Color.black;
  Canvas.strokePath c path;

  tri v1 p1;
  tri v2 p2;

  let sourcil_ratio =
    let r =
      let dy = snd p3 -. (base_y +. 200.) in
      -. (dy /. 400.)
    in
    min 0.95 (max 0.05 r)
  in

  oeuil o1 p3_oeuil true sourcil_ratio;
  oeuil o2 p3_oeuil false sourcil_ratio;

  push_time (time ());

  let fps_s = Printf.sprintf "%0.2f" (fps ()) in
  Canvas.setFillColor c Color.black;
  Canvas.fillText c fps_s (10., 10.);

  Canvas.show c

let () =
  draw ()

let e_move =
  React.E.map (fun { Event.canvas = _; timestamp = _; data = (x, y) } ->
      p3 := (float_of_int x, float_of_int y)
    ) Event.mouse_move

let e1 =
  React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
      Backend.stop ()
    ) Event.close

let e2 =
  React.E.map (fun { Event.canvas = _; timestamp = _;
                     data = { Event.key; char = _; flags = _ }; _ } ->
                if key = KeyEscape then
                  Backend.stop ()
              ) Event.key_down

let e3 =
  React.E.map (fun { Event.canvas = _; timestamp = _;
                     data = { Event.position = (x, y); _ } } ->
                point (float_of_int x, float_of_int y);
              ) Event.button_down

let frames = ref 0L

let e_frame =
  React.E.map (fun { Event.canvas = _; timestamp = _; _ } ->
      Canvas.setFillColor c Color.white;
      Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(float_of_int sw, float_of_int sh);

      draw ();
      frames := Int64.add !frames Int64.one
    ) Event.frame

let () =
  if Array.length Sys.argv >= 2 && Sys.argv.(1) = "bench" then
    for _ = 0 to 1_000 do
      draw ();
    done
  else
    Backend.run (fun () ->
        ignore (Sys.opaque_identity e_frame);
        ignore (Sys.opaque_identity e_move);
        ignore (Sys.opaque_identity (e1, e2));
        ignore (Sys.opaque_identity (e3));
        Printf.printf "\nDisplayed %Ld frames. Goodbye !\n" !frames)

