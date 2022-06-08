open OcamlCanvas
open Float


let interpInt x1 x2 t =
  int_of_float ((1. -. t)*.(float_of_int x1) +. t*.(float_of_int x2))
let interpColor c1 c2 t =
    let r1, g1, b1 = Color.to_rgb c1 and r2, g2, b2 = Color.to_rgb c2 in
    Color.of_rgb (interpInt r1 r2 t) (interpInt g1 g2 t) (interpInt b1 b2 t)
let hsv_to_rgb h s v = 
  let c = v *. s in 
  let m = v -. c in
  let x = c *. (1. -. abs( ( (rem (h /. 60.) 2.) -. 1.))) in
  let r,g,b = match(h) with
    |a when a < 60. -> c,x,0.
    |a when a < 120. -> x,c,0.
    |a when a < 180. -> 0.,c,x
    |a when a < 240. -> 0.,x,c
    |a when a < 300. -> x,0.,c
    |_ -> c,0.,x 
  in Color.of_rgb (int_of_float((r +. m)*.255.)) (int_of_float((g +. m)*.255.)) (int_of_float((b +. m)*.255.))

open Int64

let () =
  Backend.(init default_options);
  let c = Canvas.createFramed "test" ~pos:(960-640,540-360) ~size:(1280,720) in
  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.,0.) ~size:(1280.,720.);
  Canvas.show c;
  let r = ref (-1.) in 
  Backend.run(function 
    |Frame { canvas = c; timestamp = _ } ->
      r := !r +. 1. /. 60.;
      Canvas.setFillColor c (hsv_to_rgb (!r *. 36.)  1. 1.);
      Canvas.fillRect c ~pos:(128. *. !r,0.) ~size:(128., 360.);
      Canvas.setFillColor c (interpColor Color.black Color.white (!r *. 0.1));
      Canvas.fillRect c ~pos:(128. *. !r,360.) ~size:(128., 360.);
      true;
    |_ ->
      false;
  )(function () ->
    Printf.printf "Goodbye !\n"
  )