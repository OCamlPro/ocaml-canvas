open OcamlCanvas.V1
open Const

module Color = struct
  include Color

  let (~$) = of_int
  let sun = ~$ 0xffffbd
  let red_sun = ~$ 0xff4d00
  let sky = ~$ 0x82e4f5
  let red_sky = ~$ 0xff9933
  let night_sky = ~$ 0x050f24
  let sand = ~$ 0xfcbf4e
  let building = ~$ 0xfaf2d4
  let shade_building = ~$ 0x857960
  let night_filter = ~$ 0x1a1a51

  let interpolate f c1 c2 =
    let interp x y = int_of_float (float_of_int x *. (1.-.f) +. (float_of_int y) *. f) in
    let r1, g1, b1 = Color.to_rgb c1 in
    let r2, g2, b2 = Color.to_rgb c2 in
    Color.of_rgb (interp r1 r2) (interp g1 g2) (interp b1 b2)

end

module Coord = struct
  type t = { x : float; y : float; z : float }

  let _org = { x = 0.; y = 0.; z = 0. }

  let mk x y z = {x;y;z}

  let add p1 p2 = { x = p1.x +. p2.x; y = p1.y +. p2.y; z = p1.z +. p2.z; }

  let sub p1 p2 = { x = p1.x -. p2.x; y = p1.y -. p2.y; z = p1.z -. p2.z; }

  let mul p s = { x = p.x *. s; y = p.y *. s; z = p.z *. s; }

  let dot p1 p2 = p1.x *. p2.x +. p1.y *. p2.y +. p1.z *. p2.z

  let length p = sqrt (dot p p)

  let normalize p =
    let l = length p in
    mul p (1./.l)

  let of_angles theta phi =
    let x = cos theta *. cos phi in
    let y = sin theta *. cos phi in
    let z = sin phi in
    normalize (mk x y z)

  let to_point va_p va_z : t -> Point.t  =
    let xx, xy = -. sin va_p, -. sin va_z *. cos va_p in
    let yx, yy = cos va_p, -. sin va_z  *. sin va_p in
    let zx, zy = 0., cos va_z in
    fun p ->
      (p.x *. xx +. p.y *. yx +. p.z *. zx),
      (p.x *. xy +. p.y *. yy +. p.z *. zy)

end

module Block = struct

  type t = {
    pos : Coord.t;
    size : Coord.t;
    shape : shape;
  }
  and shape =
    | Full
    | Empty
    | Frag of t array (* 8 parts *)

  let z_rotate (p: Coord.t) ~around ~theta =
    let z_axis = Coord.(around.x, around.y) in
    let (x, y) =
      Point.rotate (p.x,p.y) ~around:z_axis ~theta
    in
    { p with x;y }

  let center b = Coord.(add b.pos (mul b.size 0.5))

  let face_x b view_angle =
    let o = b.pos in
    let p = Coord.add o b.size in
    if cos view_angle > 0. then
       [ { p with z=o.z };
        { p with z=o.z; y=o.y};
        { p with y=o.y };
        p;
      ]
    else
      [ { o with z=p.z };
        { o with z=p.z; y=p.y};
        { o with y=p.y };
        o;
      ]

  let face_y b view_angle =
    let o = b.pos in
    let p = Coord.add o b.size in
    if sin view_angle > 0. then
      [ { p with z=o.z };
        { p with z=o.z; x=o.x};
        { p with x=o.x };
        p;
      ]
    else
      [ { o with z=p.z };
        { o with z=p.z; x=p.x};
        { o with x=p.x };
        o;
      ]

  let face_z b _view_angle =
    let o = b.pos in
    let p = Coord.add o b.size in
    [ { p with x=o.x };
      { p with x=o.x; y=o.y};
      { p with y=o.y };
      p;
    ]

  let outline light_angle =
    let theta = (pi /.2. *. (floor ((light_angle +. pi) /. (pi /.2.)))) in
    fun b ->
    let o = b.pos in
    let p = Coord.add o b.size in
    List.map (z_rotate ~around:(center b) ~theta)
      [ { p with x=o.x };
        { p with x=o.x; y=o.y};
        { p with y=o.y };
        { p with y=o.y; z=o.z};
        { p with z=o.z; };
        { p with x=o.x; z=o.z };
      ]

end

let gen_block_better init_fuel =
  Random.self_init ();
  let rec aux fuel pos size =
    if Random.int init_fuel >= fuel
    then if Random.bool ()
      then Block.{ pos; size; shape = Full }
      else Block.{ pos; size; shape = Empty }
    else
      let ssize = Coord.mul size 0.5 in
      let frag = Block.[|
          { pos; size = ssize; shape = Full} ;
          { pos = { pos with x = pos.x +. ssize.x };
            size = ssize; shape = Full} ;
          { pos = { pos with y = pos.y +. ssize.y };
            size = ssize; shape = Full} ;
          { pos = { pos with x = pos.x +. ssize.x;
                             y = pos.y +. ssize.y };
            size = ssize; shape = Full} ;
          { pos = { pos with z = pos.z +. ssize.z };
            size = ssize; shape = Full} ;
          { pos = { pos with z = pos.z +. ssize.z ;
                             x = pos.x +. ssize.x };
            size = ssize; shape = Full} ;
          { pos = { pos with z = pos.z +. ssize.z;
                             y = pos.y +. ssize.y };
            size = ssize; shape = Full} ;
          { pos = { z = pos.z +. ssize.z;
                    x = pos.x +. ssize.x;
                    y = pos.y +. ssize.y };
            size = ssize; shape = Full } ;
        |]
      in
      frag.(0) <- aux (fuel -1) frag.(0).pos ssize;
      frag.(1) <- aux (fuel -1) frag.(1).pos ssize;
      frag.(2) <- aux (fuel -1) frag.(2).pos ssize;
      frag.(3) <- aux (fuel -1) frag.(3).pos ssize;
      frag.(4) <- if frag.(0).shape = Full
        then aux (fuel -1) frag.(4).pos ssize
        else Block.{ pos; size; shape = Empty };
      frag.(5) <- if frag.(1).shape = Full
        then aux (fuel -1) frag.(5).pos ssize
        else Block.{ pos; size; shape = Empty };
      frag.(6) <- if frag.(2).shape = Full
        then aux (fuel -1) frag.(6).pos ssize
        else Block.{ pos; size; shape = Empty };
      frag.(7) <- if frag.(3).shape = Full
        then aux (fuel -1) frag.(7).pos ssize
        else Block.{ pos; size; shape = Empty };
      Block.{ pos; size; shape = Frag frag }
  in
  aux init_fuel (Coord.mk (-.50.) (-.50.) (0.)) (Coord.mk 100. 100. 100.)

type scene = {
  blocks : Block.t;
  shadows : Coord.t list list;
  sun_angle_xy : float;
  sun_angle_z : float;
  view_angle_z : float;
  view_angle_xy : float;
  projection : Coord.t -> Point.t;
}

let lightvec_of_scene scene =
  let vec = Coord.of_angles scene.sun_angle_xy scene.sun_angle_z in
  if vec.z >= 0.
  then Coord.mul vec (-.1.)
  else vec

let light_factor_of_scene scene =
    let sa = sin (scene.sun_angle_z +. pi) in
    let evening_thres = 0.3 in
    let dusk_thres = 0.1 in
    if sa > evening_thres then 1.
    else if sa > 0. then sa /. evening_thres
    else if sa > -. dusk_thres then sa /. dusk_thres
    else -. 1.

let shadow_plane scene =
  let sunvec = lightvec_of_scene scene in
  let light_factor = light_factor_of_scene scene in
  let block_outline =
    Block.outline
      (if light_factor < 0.
       then scene.sun_angle_xy +. pi
       else scene.sun_angle_xy)
  in
  let ztransform p =
    let zproj = Coord.{ p with z=0. } in
    let pz = Coord.sub zproj p in
    let d = Coord.dot pz sunvec in
    if d <= 0. then zproj else
      Coord.(add p (mul sunvec (abs_float (p.Coord.z/.sunvec.Coord.z))))
  in
  let rec aux polys block =
    match block.Block.shape with
    | Empty -> polys
    | Full ->
      let outline = block_outline block in
      let ztrans_outline = List.map ztransform outline in
      ztrans_outline::polys
    | Frag frag ->
      Array.fold_left aux polys frag
  in
  aux [] scene.blocks

let draw_poly c clr pl =
  let round (x, y) = floor x, floor y in
  let p = Path.create () in
  List.iter (fun pt -> Path.lineTo p (round pt)) pl;
  Canvas.setFillColor c clr;
  Canvas.fillPath c p ~nonzero:true

let draw_face c scene face color =
  let trans_coord = scene.projection in
  draw_poly c color (List.map trans_coord face)

let draw_block c scene =
  let perm, xnorm, ynorm =
    let cosv = cos scene.view_angle_xy in
    let sinv = sin scene.view_angle_xy in
    if cosv >= 0. && sinv >= 0. then
      [|0;1;2;3;4;5;6;7|],
      Coord.mk 1. 0. 0.,
      Coord.mk 0. 1. 0.
    else if cosv >= 0. && sinv < 0. then
      [|2;0;3;1;6;4;7;5|],
      Coord.mk 1. 0. 0.,
      Coord.mk 0. (-.1.) 0.
    else if cosv < 0. && sinv < 0. then
      [|3;2;1;0;7;6;5;4|],
      Coord.mk (-.1.) 0. 0.,
      Coord.mk 0. (-.1.) 0.
    else
      [|1;3;0;2;5;7;4;6|],
      Coord.mk (-.1.) 0. 0.,
      Coord.mk 0. 1. 0.
  in
  let sunvec = lightvec_of_scene scene in
  let light_factor = light_factor_of_scene scene in
  let shade_blend = 0.1 +. abs_float light_factor in
  let night_blend = if light_factor < 0. then -. light_factor *. 0.7 else 0. in
  let color_of_norm n =
    let na = max 0. (-. Coord.dot n sunvec) in
    Color.(interpolate
             night_blend
             (interpolate
                (na *. shade_blend)
                shade_building
                building)
             night_filter)
  in
  let xcolor = color_of_norm xnorm in
  let ycolor = color_of_norm ynorm in
  let zcolor = color_of_norm Coord.(mk 0. 0. 1.) in
  let rec aux b =
  match b.Block.shape with
  | Empty -> ()
  | Full ->
    draw_face c scene (Block.face_y b scene.view_angle_xy) ycolor;
    draw_face c scene (Block.face_x b scene.view_angle_xy) xcolor;
    draw_face c scene (Block.face_z b scene.view_angle_xy) zcolor
  | Frag frag ->
    Array.iter (fun i -> aux frag.(i)) perm
  in
  aux

let draw_sky_and_ground scene c =
  let x, y = Point.of_ints (Canvas.getSize c) in
  let h =
    let baseh = y*.0.6 in
    floor @@ baseh -. (sin scene.view_angle_z *. baseh)
  in
  let light_factor = light_factor_of_scene scene in
  let sun_size = min x y *. 0.05 in
  let sun_pos =
    let perspective_factor = 3. in
    let axy =
      if light_factor > 0.
      then pi +. scene.sun_angle_xy -. scene.view_angle_xy
      else scene.sun_angle_xy -. scene.view_angle_xy
    in
    if cos axy > 0. then -. sun_size, -. sun_size else
      let sx = sin axy *. x *. perspective_factor +. (x *. 0.5) in
      let sy = h -. y *. abs_float (sin scene.sun_angle_z) in
      sx, sy
  in
  let dark_sand = Color.(interpolate 0.5 (~$0x800800) sand) in
  let sand_color =
    if light_factor > 0. then
      Color.(interpolate
               light_factor
               dark_sand
               sand)
    else
      Color.(interpolate (-. light_factor *. 0.8) dark_sand night_sky)
  in
  let shade_sand_color =
    let blend_fact = if light_factor > 0. then 0.5 else 0.3 in
    Color.(interpolate
             ((abs_float light_factor +. 0.1) *. blend_fact)
             sand_color
             black)
  in
  let sky_color =
    if light_factor > 0. then
      Color.(interpolate light_factor red_sky sky)
    else
      Color.(interpolate
               (-. light_factor)
               red_sky night_sky)
  in
  let sun_color =
    if light_factor > 0. then
      Color.(interpolate light_factor red_sun sun)
    else Color.sun
  in
  let draw_ground () =
    Canvas.setFillColor c sand_color;
    Canvas.fillRect c ~pos:(0.,h) ~size:(x,y-.h);
    List.iter
      (fun pl -> draw_poly c shade_sand_color
          (List.map scene.projection pl))
      scene.shadows;
  in
  let draw_sun () =
    Canvas.setFillColor c sun_color;
    Canvas.ellipse c ~center:sun_pos ~radius:(sun_size, sun_size)
      ~rotation:0. ~theta1:0. ~theta2:(pi *.2.) ~ccw:false;
    Canvas.fill c ~nonzero:false;
    Canvas.clearPath c;
    if light_factor < 0. then begin
    Canvas.setFillColor c sky_color;
    Canvas.ellipse c ~center:(fst sun_pos -. sun_size *. 0.5,snd sun_pos)
      ~radius:(sun_size, sun_size) ~rotation:0. ~theta1:0.
      ~theta2:(pi *.2.) ~ccw:false;
    Canvas.fill c ~nonzero:false;
    Canvas.clearPath c;
    end
  in
  let draw_sky () =
    Canvas.setFillColor c sky_color;
    Canvas.fillRect c ~pos:(0.,0.) ~size:(x,h);
    draw_sun ()
  in
  if snd sun_pos > 0. then
    (draw_sky (); draw_ground ())
  else
    (draw_ground (); draw_sky ())

let gen () = gen_block_better 6

let sun_z_of_xy xy =
  let f = sin (xy -. 0.2 +. pi) in
  -. (pi *. 0.2 *. f +. 0.2)

let scene =
  let blocks = gen () in
  let sun_angle_xy = -. pi /. 3. in
  let sun_angle_z = sun_z_of_xy sun_angle_xy in
  let scene = {
    view_angle_xy = pi_4;
    view_angle_z = pi /. 12.;
    sun_angle_xy;
    sun_angle_z;
    blocks;
    shadows = [];
    projection = fun Coord.{x;y;_} -> (x,y);
  }
  in
  let shadows = shadow_plane scene in
  ref { scene with shadows }

let canvas_transform c =
  let x, y = Canvas.getSize c |> Point.of_ints in
  let scl = 0.004*.(min x y) in
  Transform.(id
  |> fun t -> translate t (x/.2., y*.0.6)
  |> fun t -> scale t (scl,-.scl))

let compute_projection c =
  let vaxy = !scene.view_angle_xy in
  let vaz = !scene.view_angle_z in
  let canvas_proj = canvas_transform c in
  let projection coord =
    Point.transform
      (Coord.to_point vaxy vaz coord)
      canvas_proj
  in
  scene := { !scene with projection }

let regen_shadows () =
  let shadows = shadow_plane !scene in
  scene := { !scene with shadows }

let regen () =
  let blocks = gen () in
  scene := { !scene with blocks };
  regen_shadows ()

let draw_scene c =
  draw_sky_and_ground !scene c;
  draw_block c !scene !scene.blocks

let rotate_view c x y =
  let cx, cy = Point.of_ints (Canvas.getSize c) in
  let f = 2.6 in
  let rx = f*. float_of_int x /. cx in
  let ry = f*. float_of_int y /. cy in
  let clamp a =
    max 0. (min a pi_2)
  in
  let view_angle_xy = !scene.view_angle_xy +. rx in
  let view_angle_z = clamp (!scene.view_angle_z +. ry) in
  scene := { !scene with view_angle_xy; view_angle_z };
  compute_projection c

let rotate_light c x _y =
  let cx, _cy = Point.of_ints (Canvas.getSize c) in
  let f = -.6. in
  let dx = f*. float_of_int x /. cx in
  let sun_angle_xy = !scene.sun_angle_xy +. dx in
  let sun_angle_z = sun_z_of_xy sun_angle_xy in
  scene := { !scene with sun_angle_xy; sun_angle_z };
  regen_shadows ()

let stored_ev = ref []

let store ev = stored_ev := ev::!stored_ev

type maction =
  | NoAction
  | ViewRot of int * int
  | LightRot of int * int

let () =
  let size_x, size_y = 1600, 800 in
  Backend.init ();
  let c =
    Canvas.createOnscreen ~title:"Sun Empire"
      ~pos:(0,0) ~size:(size_x, size_y)
      ~target:"can" ~decorated:false ()
  in

  compute_projection c;
  draw_scene c;
  Canvas.show c;

  let ev_regen = React.E.map
    (fun Event.{ data = { Event.key; char = _; flags = _ }; _ } ->
      if key = KeySpacebar then
        (regen (); draw_scene c)
    ) Event.key_down
  in

  let ev_resize = React.E.map
    (fun Event.{ data = size; _ } ->
     (Canvas.setSize c size; compute_projection c; draw_scene c)
    ) Event.resize
  in

  let mpos = ref NoAction in

  let ev_mousedown = React.E.map
    (fun Event.{ data = { position = (x,y); button }; _ } ->
      match button with
      | ButtonLeft -> mpos := ViewRot (x,y)
      | ButtonRight -> mpos := LightRot (x,y)
      | _ -> ()
    ) Event.button_down
  in

  let ev_mouseup = React.E.map
    (fun Event.{ data = { button = _; _ }; _ } ->
      mpos := NoAction
    ) Event.button_up
  in

  let ev_mouse = React.E.map
    (fun Event.{ data = (x, y); _ } ->
      match !mpos with
      | NoAction -> ()
      | ViewRot (ox, oy) ->
        mpos := ViewRot (x,y);
        rotate_view c (ox-x) (y-oy);
        draw_scene c
      | LightRot (ox, oy) ->
        mpos := LightRot (x,y);
        rotate_light c (ox-x) (y-oy);
        draw_scene c)
    Event.mouse_move
  in

  List.iter store [ev_resize; ev_regen; ev_mouse; ev_mousedown; ev_mouseup];

  Backend.run (fun () -> ())
