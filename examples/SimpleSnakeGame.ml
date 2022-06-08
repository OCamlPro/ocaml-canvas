open OcamlCanvas
open Int64
open Random

let buildBackground c =
  Canvas.setFillColor c Color.black;
  Canvas.fillRect c ~pos:(0.,0.) ~size:(500.,500.);
  Canvas.setFillColor c Color.white;
  Canvas.fillRect c ~pos:(0.,0.) ~size:(500.,10.);
  Canvas.fillRect c ~pos:(0.,490.) ~size:(500.,10.);
  Canvas.fillRect c ~pos:(0.,0.) ~size:(10.,500.);
  Canvas.fillRect c ~pos:(490.,0.) ~size:(10.,500.);
  ()

let placeBlock c (x,y) col = 
  Canvas.setFillColor c col; 
  Canvas.fillRect c ~pos:(x *. 10. , y *. 10.) ~size:(10.,10.);
  ()

let rec drawSnake c s = match(s) with
  |[] -> ()
  |h::t -> placeBlock c h Color.orange; drawSnake c t

let rec moveSnake s p = match(s) with
  |[] -> []
  |h::t -> p::(moveSnake t h)

let sumCoord (a,b) (c,d) = (a +. c,b +. d)

let moveSnakeDirection s d = match(s) with
  |[] -> []
  |h::t -> moveSnake s (sumCoord d h)

let snakeHitSelf s = match(s) with
  |[] -> false
  |h::t -> (List.mem h t)
let snakeHitWall s = let h::t = s in let (x,y) = h in (x < 1.) or (x > 48.) or (y < 1.) or (y > 48.)
  

let () =
  Random.self_init();
  Backend.(init default_options);
  let c = Canvas.createFramed "test" ~pos:(960-250,540-250) ~size:(500,500) in
  Canvas.show c;
  let r = ref (-1.) in 
  let snake = ref [(6.,8.);(6.,7.)] and currentDirection = ref (0.,1.) and foodLocation = ref(24.,24.) in
  Backend.run(function 
    |KeyAction { canvas = c; timestamp = _;
    key = KeyUpArrow; char; flags = _; state = Down } ->
      let (x,y) = !currentDirection in
      if (y = 0.) then currentDirection := (0.,-1.);
      true;
    |KeyAction { canvas = c; timestamp = _;
    key = KeyDownArrow; char; flags = _; state = Down } ->
      let (x,y) = !currentDirection in
      if (y = 0.) then currentDirection := (0.,1.);
      true;
    |KeyAction { canvas = c; timestamp = _;
    key = KeyLeftArrow; char; flags = _; state = Down } ->
      let (x,y) = !currentDirection in
      if (x = 0.) then currentDirection := (-1.,0.);
      true;
    |KeyAction { canvas = c; timestamp = _;
    key = KeyRightArrow; char; flags = _; state = Down } ->
      let (x,y) = !currentDirection in
      if (x = 0.) then currentDirection := (1.,0.);
      true;
    
    |Frame { canvas = c; timestamp = _ } ->
      r := !r +. 1. /. 60.;
      buildBackground c;
      let h::t = !snake in
        if ((sumCoord h !currentDirection) = !foodLocation) then (snake := !foodLocation::!snake; foodLocation := (2. +. float_of_int (Random.int 47),2. +. float_of_int (Random.int 47)));
      if !r > 0.033 then (r := 0.; snake := moveSnakeDirection !snake !currentDirection);
      if (snakeHitSelf !snake or snakeHitWall !snake) then Backend.stop();
      drawSnake c !snake;
      placeBlock c !foodLocation Color.green;
      true;
    |_ ->
      false;
  )(function () ->
    Printf.printf "Goodbye !\n"
  )