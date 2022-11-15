(**************************************************************************)
(*                                                                        *)
(*    Copyright 2022 OCamlPro                                             *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

module V1 = struct

  let valid_canvas_size (width, height) =
    0 < width && width <= 32767 && 0 < height && height <= 32767

  module Exception = struct

    exception Not_initialized
    exception File_not_found of string
    exception Read_png_failed of string
    exception Write_png_failed of string

    let () =
      Callback.register_exception "Not_initialized" Not_initialized;
      Callback.register_exception "File_not_found" (File_not_found "");
      Callback.register_exception "Read_png_failed" (Read_png_failed "");
      Callback.register_exception "Write_png_failed" (Write_png_failed "")

  end

  module Const = struct

    let pi =       3.14159265358979323846
    let pi_2 =     1.57079632679489661923
    let pi_4 =     0.785398163397448309616
    let pi_8 =     0.392699081698724154808

    let inv_pi =   0.318309886183790671538
    let inv_pi_2 = 0.636619772367581343076
    let inv_pi_4 = 1.27323954473516268615
    let inv_pi_8 = 2.54647908947032537230

    let e =        2.71828182845904523536
    let inv_e =    0.367879441171442321596

    let log2_e =   1.44269504088896340736
    let log10_e =  0.434294481903251827651

    let ln_2 =     0.693147180559945309417
    let ln_10 =    2.30258509299404568402

  end

  module Vector = struct

    type t = (float * float)

    let zero =
      (0.0, 0.0)

    let unit =
      (1.0, 1.0)

    let add (x1, y1) (x2, y2) =
      x1 +. x2, y1 +. y2

    let sub (x1, y1) (x2, y2) =
      x1 -. x2, y1 -. y2

    let mul (x, y) k =
      x *. k, y *. k

    let dot (x1, y1) (x2, y2) =
      x1 *. x2 +. y1 *. y2

    let norm (x, y) =
      sqrt (x *. x +. y *. y)

  end

  module Transform = struct

    type t = {
      a : float;
      b : float;
      c : float;
      d : float;
      e : float;
      f : float;
    }

    let id = { a = 1.0; b = 0.0;
               c = 0.0; d = 1.0;
               e = 0.0; f = 0.0 }

    let create (a, b, c, d, e, f) =
      { a; b; c; d; e; f }

    let mul t1 t2 =
      { a = t1.a *. t2.a +. t1.c *. t2.b;
        b = t1.b *. t2.a +. t1.d *. t2.b;
        c = t1.a *. t2.c +. t1.c *. t2.d;
        d = t1.b *. t2.c +. t1.d *. t2.d;
        e = t1.e +. t1.a *. t2.e +. t1.c *. t2.f;
        f = t1.f +. t1.b *. t2.e +. t1.d *. t2.f }

    let translate t (x, y) =
      { t with
        e = x *. t.a +. y *. t.c;
        f = x *. t.b +. y *. t.d }

    let scale t (x,y) =
      { t with
        a = t.a *. x; b = t.b *. x;
        c = t.c *. y; d = t.d *. y }

    let shear t (x, y) =
      { t with
        a = t.a +. t.c *. y; b = t.b +. t.d *. y;
        c = t.c +. t.a *. x; d = t.d +. t.b *. x }

    let rotate t a =
      let cos_a = cos (-. a) in
      let sin_a = sin (-. a) in
      { t with
        a = t.a *. cos_a -. t.c *. sin_a;
        b = t.b *. cos_a -. t.d *. sin_a;
        c = t.c *. cos_a +. t.a *. sin_a;
        d = t.d *. cos_a +. t.b *. sin_a }

    let inverse t =
      let det = t.a *. t.d -. t.b *. t.c in
      if det = 0.0 then
        invalid_arg "Transform.inverse: the matrix is not inversible";
      let invdet = 1.0 /. det in
      { a =    t.d *. invdet; b = -. t.b *. invdet;
        c = -. t.c *. invdet; d =    t.a *. invdet;
        e = (t.c *. t.f -. t.d *. t.e) *. invdet;
        f = (t.b *. t.e -. t.a *. t.f) *. invdet }

  end

  module Point = struct

    type t = (float * float)

    let of_ints (x, y) =
      (float_of_int x, float_of_int y)

    let sub (x1, y1) (x2, y2) =
      x1 -. x2, y1 -. y2

    let translate (x, y) ~by:(a, b) =
      (x +. a, y +. b)

    let rotate (x, y) ~around:(cx, cy) ~theta =
      ((x -. cx) *. (cos (-. theta)) +. (y -. cy) *. (sin (-. theta)) +. cx,
       (y -. cy) *. (cos (-. theta)) -. (x -. cx) *. (sin (-. theta)) +. cy)

    let transform (x, y) t =
      let open Transform in
      (x *. t.a +. y *. t.c +. t.e,
       x *. t.b +. y *. t.d +. t.f)

    let barycenter a (x1, y1) b (x2, y2) =
      let sum_ab = a +. b in
      if sum_ab = 0.0 then
        invalid_arg "Point.barycenter: a + b must be non-nul";
      (a *. x1 +. b *. x2) /. sum_ab,
      (a *. y1 +. b *. y2) /. sum_ab

    let distance (x1, y1) (x2, y2) =
      sqrt ((x2 -. x1) ** 2.0 +. (y2 -. y1) ** 2.0)

  end

  module Color = struct

    type t = Int32.t

    let clip_8 i =
      if i < 0 then 0
      else if i > 0xFF then 0xFF
      else i

    let of_rgb r g b =
      Int32.add 0xFF000000l
        (Int32.of_int (clip_8 r lsl 16 + clip_8 g lsl 8 + clip_8 b))

    let to_rgb c =
      Int32.(to_int (logand (shift_right_logical c 16) 0xFFl)),
      Int32.(to_int (logand (shift_right_logical c 8) 0xFFl)),
      Int32.(to_int (logand c 0xFFl))

    let of_argb a r g b =
      Int32.add
        (Int32.shift_left (Int32.of_int (clip_8 a)) 24)
        (Int32.of_int (clip_8 r lsl 16 + clip_8 g lsl 8 + clip_8 b))

    let to_argb c =
      Int32.(to_int (shift_right_logical c 24)),
      Int32.(to_int (logand (shift_right_logical c 16) 0xFFl)),
      Int32.(to_int (logand (shift_right_logical c 8) 0xFFl)),
      Int32.(to_int (logand c 0xFFl))

    let of_int i =
      if i < 0 then 0l
      else if i > 0x00FFFFFF then 0xFFFFFFFFl
      else Int32.add 0xFF000000l (Int32.of_int i)

    let to_int c =
      Int32.(to_int (logand c 0x00FFFFFFl))

    let of_int32 i =
      i

    let to_int32 c =
      c

    module StringMap = Map.Make(String)
    let colors = ref StringMap.empty
    let define_color name c =
      colors := StringMap.add (String.lowercase_ascii name) c !colors;
      c

    let transpBlack = define_color "transparent_black" 0x00000000l
    let transpWhite = define_color "transparent_white" 0x00FFFFFFl
    let black = define_color "black" 0xFF000000l
    let white = define_color "white" 0xFFFFFFFFl
    let blue = define_color "blue" 0xFF0000FFl
    let cyan = define_color "cyan" 0xFF00FFFFl
    let green = define_color "green" 0xFF008000l
    let lime = define_color "lime" 0xFF00FF00l
    let orange = define_color "orange" 0xFFFFA500l
    let pink = define_color "pink" 0xFFFFC0CBl
    let red = define_color "red" 0xFFFF0000l

    let of_string s =
      if String.length s < 1 then
        black
      else if s.[0] = '#' then
        let s = "0x" ^ (String.sub s 1 (String.length s - 1)) in
        of_int (int_of_string s)
      else
        try StringMap.find (String.lowercase_ascii s) !colors
        with Not_found -> black

  end

  module Font = struct

    type size = float

    type slant =
      | Roman
      | Italic
      | Oblique

    type weight = int

    let thin       =  100
    let extraLight =  200
    let light      =  300
    (*let semiLight  =  315 *)
    (*let book       =  370 *)
    let regular    =  400
    let medium     =  500
    let semiBold   =  600
    let bold       =  700
    let extraBold  =  800
    let black      =  900
    (*let extraBlack = 1000 *)

  end

  type 'kind canvas

  type image_data =
    (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t

  module Pending = struct

    type t =
      | ImageData :
          image_data React.event *
            (?step:React.step -> image_data -> unit) * image_data -> t
      | Canvas :
          'kind canvas React.event *
            (?step:React.step -> 'kind canvas -> unit) * 'kind canvas -> t

    let list : t list ref = ref []

    let add_image_data event send_event c =
      list := ImageData (event, send_event, c) :: !list

    let add_canvas event send_event c =
      list := Canvas (event, send_event, c) :: !list

    let process () =
      match !list with
      | [] -> ()
      | _ ->
          List.iter (function
              | ImageData (event, send_event, id) ->
                  send_event ?step:None id;
                  React.E.stop event
              | Canvas (event, send_event, c) ->
                  send_event ?step:None c;
                  React.E.stop event
            ) (List.rev !list);
          list := []

  end

  module ImageData = struct

    type t = image_data

    let create (width, height) =
      if not (valid_canvas_size (width, height)) then
        invalid_arg "ImageData.create: invalid image dimensions";
      let a = Bigarray.Array3.create Bigarray.int8_unsigned
                Bigarray.c_layout height width 4 in
      Bigarray.Array3.fill a 0;
      a

    external createFromPNG_internal : string -> (t -> unit) -> unit
      = "ml_canvas_image_data_create_from_png"

    let createFromPNG filename : t React.event =
      let event, send_event = React.E.create () in
      createFromPNG_internal filename
        (fun id -> Pending.add_image_data event send_event id);
      event

    external getSize : t -> (int * int)
      = "ml_canvas_image_data_get_size"

    external fill : t -> Color.t -> unit
      = "ml_canvas_image_data_fill"

    external sub : t -> pos:(int * int) -> size:(int * int) -> t
      = "ml_canvas_image_data_sub"

    external blit :
      dst:t -> dpos:(int * int) ->
      src:t -> spos:(int * int) -> size:(int * int) -> unit
      = "ml_canvas_image_data_blit"

    external getPixel : t -> (int * int) -> Color.t
      = "ml_canvas_image_data_get_pixel"

    external putPixel : t -> (int * int) -> Color.t -> unit
      = "ml_canvas_image_data_put_pixel"

    external importPNG_internal :
      t -> pos:(int * int) -> string -> (t -> unit) -> unit
      = "ml_canvas_image_data_import_png"

    let importPNG id ~pos filename : t React.event =
      let event, send_event = React.E.create () in
      importPNG_internal id ~pos filename
        (fun id -> Pending.add_image_data event send_event id);
      event

    external exportPNG : t -> string -> unit
      = "ml_canvas_image_data_export_png"

    type t_repr = image_data

    let of_bigarray (ba : t_repr) =
      if not (valid_canvas_size (Bigarray.Array3.dim2 ba,
                                 Bigarray.Array3.dim1 ba)) then
        invalid_arg "ImageData.of_bigarray: invalid image dimensions";

      if Bigarray.Array3.dim3 ba <> 4 then
        invalid_arg "ImageData.of_bigarray: third dimension must be 4";
      ba

    external to_bigarray : t -> t_repr = "%identity"

  end

  module Gradient = struct

    type t

    external createLinear : pos1:Point.t -> pos2:Point.t -> t
      = "ml_canvas_gradient_create_linear"

    external createRadial :
      center1:Point.t -> rad1:float -> center2:Point.t -> rad2:float -> t
      = "ml_canvas_gradient_create_radial"

    external createConic : center:Point.t -> angle:float -> t
      = "ml_canvas_gradient_create_conic"

    external addColorStop : t -> Color.t -> float -> unit
      = "ml_canvas_gradient_add_color_stop"

  end

  module Pattern = struct

    type t

    type repeat =
      | NoRepeat
      | RepeatX
      | RepeatY
      | RepeatXY

    external create : ImageData.t -> repeat -> t
      = "ml_canvas_pattern_create"

  end

  module Path = struct

    type t

    external create : unit -> t
      = "ml_canvas_path_create"

    external close : t -> unit
      = "ml_canvas_path_close"

    external moveTo : t -> Point.t -> unit
      = "ml_canvas_path_move_to"

    external lineTo : t -> Point.t -> unit
      = "ml_canvas_path_line_to"

    external arc :
      t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_path_arc" "ml_canvas_path_arc_n"

    external arcTo : t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
      = "ml_canvas_path_arc_to"

    external quadraticCurveTo : t -> cp:Point.t -> p:Point.t -> unit
      = "ml_canvas_path_quadratic_curve_to"

    external bezierCurveTo :
      t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
      = "ml_canvas_path_bezier_curve_to"

    external rect : t -> pos:Point.t -> size:Vector.t -> unit
      = "ml_canvas_path_rect"

    external ellipse :
      t -> center:Point.t -> radius:Vector.t ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_path_ellipse" "ml_canvas_path_ellipse_n"

    external add : t -> t -> unit
      = "ml_canvas_path_add"

    external addTransformed : t -> t -> Transform.t -> unit
      = "ml_canvas_path_add_transformed"

  end

  module Join = struct

    type t =
      | Round
      | Miter
      | Bevel

  end

  module Cap = struct

    type t =
      | Butt
      | Square
      | Round

  end

  module Style = struct

    type t =
      | Color of Color.t
      | Gradient of Gradient.t
      | Pattern of Pattern.t

  end

  module CompositeOp = struct

    type t =
      | SourceOver
      | SourceIn
      | SourceOut
      | SourceAtop
      | DestinationOver
      | DestinationIn
      | DestinationOut
      | DestinationAtop
      | Lighter
      | Copy
      | XOR
      | Multiply
      | Screen
      | Overlay
      | Darken
      | Lighten
      | ColorDodge
      | ColorBurn
      | HardLight
      | SoftLight
      | Difference
      | Exclusion
      | Hue
      | Saturation
      | Color
      | Luminosity

  end

  module Canvas = struct

    type 'kind t = 'kind canvas

    (* Comparison and hash functions *)

    let () =
      Callback.register "Hashtbl.hash" Hashtbl.hash

    external hash : 'kind t -> int
      = "ml_canvas_hash"

    external compare : 'kind1 t -> 'kind2 t -> int
      = "ml_canvas_compare"

    let equal c1 c2 = compare c1 c2 = 0

    let (=) c1 c2 = compare c1 c2 = 0
    let (<>) c1 c2 = compare c1 c2 <> 0
    let (<) c1 c2 = compare c1 c2 < 0
    let (>) c1 c2 = compare c1 c2 > 0
    let (<=) c1 c2 = compare c1 c2 <= 0
    let (>=) c1 c2 = compare c1 c2 >= 0

    external (==) : 'kind1 t -> 'kind2 t -> bool = "%eq"
    external (!=) : 'kind1 t -> 'kind2 t -> bool = "%noteq"

    (* Creation *)

    external createOnscreen :
      ?decorated:bool -> ?resizeable:bool -> ?minimize:bool ->
      ?maximize:bool -> ?close:bool -> ?title:string ->
      ?pos:(int * int) -> size:(int * int) -> unit -> [> `Onscreen] t
      = "ml_canvas_create_onscreen" "ml_canvas_create_onscreen_n"

    external createOffscreen : size:(int * int) -> [> `Offscreen] t
      = "ml_canvas_create_offscreen"

    external createOffscreenFromImageData : ImageData.t -> [> `Offscreen] t
      = "ml_canvas_create_offscreen_from_image_data"

    external createOffscreenFromPNG_internal :
      string -> ([> `Offscreen] t -> unit) -> unit
      = "ml_canvas_create_offscreen_from_png"

    let createOffscreenFromPNG filename : [> `Offscreen] t React.event =
      let event, send_event = React.E.create () in
      createOffscreenFromPNG_internal filename
        (fun c -> Pending.add_canvas event send_event c);
      event

    (* Visibility *)

    external show : [< `Onscreen] t -> unit
      = "ml_canvas_show"

    external hide : [< `Onscreen] t -> unit
      = "ml_canvas_hide"

    external close : [< `Onscreen] t -> unit
      = "ml_canvas_close"

    (* Configuration *)

    external getId : 'kind t -> int
      = "ml_canvas_get_id"

    external getSize : 'kind t -> (int * int)
      = "ml_canvas_get_size"

    external setSize : 'kind t -> (int * int) -> unit
      = "ml_canvas_set_size"

    external getPosition : [< `Onscreen] t -> (int * int)
      = "ml_canvas_get_position"

    external setPosition : [< `Onscreen] t -> (int * int) -> unit
      = "ml_canvas_set_position"

    (* State *)

    external save : 'kind t -> unit
      = "ml_canvas_save"

    external restore : 'kind t -> unit
      = "ml_canvas_restore"

    (* Transformations *)
    external setTransform : 'kind t -> Transform.t -> unit
      = "ml_canvas_set_transform"

    external transform : 'kind t -> Transform.t -> unit
      = "ml_canvas_transform"

    external translate : 'kind t -> Vector.t -> unit
      = "ml_canvas_translate"

    external scale : 'kind t -> Vector.t -> unit
      = "ml_canvas_scale"

    external shear : 'kind t -> Vector.t -> unit
      = "ml_canvas_shear"

    external rotate : 'kind t -> float -> unit
      = "ml_canvas_rotate"

    (* Style / config *)

    external getLineWidth : 'kind t -> float
      = "ml_canvas_get_line_width"

    external setLineWidth : 'kind t -> float -> unit
      = "ml_canvas_set_line_width"

    external getLineJoin : 'kind t -> Join.t
      = "ml_canvas_get_line_join"

    external setLineJoin : 'kind t -> Join.t -> unit
      = "ml_canvas_set_line_join"

    external getLineCap : 'kind t -> Cap.t
      = "ml_canvas_get_line_cap"

    external setLineCap : 'kind t -> Cap.t -> unit
      = "ml_canvas_set_line_cap"

    external getMiterLimit : 'kind t -> float
      = "ml_canvas_get_miter_limit"

    external setMiterLimit : 'kind t -> float -> unit
      = "ml_canvas_set_miter_limit"

    external getLineDashOffset : 'kind t -> float
      = "ml_canvas_get_line_dash_offset"

    external setLineDashOffset : 'kind t -> float -> unit
      = "ml_canvas_set_line_dash_offset"

    external getLineDash : 'kind t -> float array
      = "ml_canvas_get_line_dash"

    external setLineDash : 'kind t -> float array -> unit
      = "ml_canvas_set_line_dash"

    external getStrokeColor : 'kind t -> Color.t
      = "ml_canvas_get_stroke_color"

    external setStrokeColor : 'kind t -> Color.t -> unit
      = "ml_canvas_set_stroke_color"

    external setStrokeGradient : 'kind t -> Gradient.t -> unit
      = "ml_canvas_set_stroke_gradient"

    external setStrokePattern : 'kind t -> Pattern.t -> unit
      = "ml_canvas_set_stroke_pattern"

    external getStrokeStyle : 'kind t -> Style.t
      = "ml_canvas_get_stroke_style"

    external setStrokeStyle : 'kind t -> Style.t -> unit
      = "ml_canvas_set_stroke_style"

    external getFillColor : 'kind t -> Color.t
      = "ml_canvas_get_fill_color"

    external setFillColor : 'kind t -> Color.t -> unit
      = "ml_canvas_set_fill_color"

    external setFillGradient : 'kind t -> Gradient.t -> unit
      = "ml_canvas_set_fill_gradient"

    external setFillPattern : 'kind t -> Pattern.t -> unit
      = "ml_canvas_set_fill_pattern"

    external getFillStyle : 'kind t -> Style.t
      = "ml_canvas_get_fill_style"

    external setFillStyle : 'kind t -> Style.t -> unit
      = "ml_canvas_set_fill_style"

    external getGlobalAlpha : 'kind t -> float
      = "ml_canvas_get_global_alpha"

    external setGlobalAlpha : 'kind t -> float -> unit
      = "ml_canvas_set_global_alpha"

    external getGlobalCompositeOperation : 'kind t -> CompositeOp.t
      = "ml_canvas_get_global_composite_operation"

    external setGlobalCompositeOperation : 'kind t -> CompositeOp.t -> unit
      = "ml_canvas_set_global_composite_operation"

    external getShadowColor : 'kind t -> Color.t
      = "ml_canvas_get_shadow_color"

    external setShadowColor : 'kind t -> Color.t -> unit
      = "ml_canvas_set_shadow_color"

    external getShadowBlur : 'kind t -> float
      = "ml_canvas_get_shadow_blur"

    external setShadowBlur : 'kind t -> float -> unit
      = "ml_canvas_set_shadow_blur"

    external getShadowOffset : 'kind t -> Vector.t
      = "ml_canvas_get_shadow_offset"

    external setShadowOffset : 'kind t -> Vector.t -> unit
      = "ml_canvas_set_shadow_offset"

    external setFont :
      'kind t -> string -> size:Font.size -> slant:Font.slant ->
      weight:Font.weight -> unit
      = "ml_canvas_set_font"

    (* Paths *)

    external clearPath : 'kind t -> unit
      = "ml_canvas_clear_path"

    external closePath : 'kind t -> unit
      = "ml_canvas_close_path"

    external moveTo : 'kind t -> Point.t -> unit
      = "ml_canvas_move_to"

    external lineTo : 'kind t -> Point.t -> unit
      = "ml_canvas_line_to"

    external arc :
      'kind t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_arc" "ml_canvas_arc_n"

    external arcTo : 'kind t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
      = "ml_canvas_arc_to"

    external quadraticCurveTo : 'kind t -> cp:Point.t -> p:Point.t -> unit
      = "ml_canvas_quadratic_curve_to"

    external bezierCurveTo :
      'kind t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
      = "ml_canvas_bezier_curve_to"

    external rect : 'kind t -> pos:Point.t -> size:Vector.t -> unit
      = "ml_canvas_rect"

    external ellipse :
      'kind t -> center:Point.t -> radius:Vector.t ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_ellipse" "ml_canvas_ellipse_n"

    external fill : 'kind t -> nonzero:bool -> unit
      = "ml_canvas_fill"

    external fillPath : 'kind t -> Path.t -> nonzero:bool -> unit
      = "ml_canvas_fill_path"

    external stroke : 'kind t -> unit
      = "ml_canvas_stroke"

    external strokePath : 'kind t -> Path.t -> unit
      = "ml_canvas_stroke_path"

    external clip : 'kind t -> nonzero:bool -> unit
      = "ml_canvas_clip"

    external clipPath : 'kind t -> Path.t -> nonzero:bool -> unit
      = "ml_canvas_clip_path"

    (* Immediate drawing *)

    external fillRect : 'kind t -> pos:Point.t -> size:Vector.t -> unit
      = "ml_canvas_fill_rect"

    external strokeRect : 'kind t -> pos:Point.t -> size:Vector.t -> unit
      = "ml_canvas_stroke_rect"

    external fillText : 'kind t -> string -> Point.t -> unit
      = "ml_canvas_fill_text"

    external strokeText : 'kind t -> string -> Point.t -> unit
      = "ml_canvas_stroke_text"

    external blit :
      dst:'kind1 t -> dpos:(int * int) ->
      src:'kind2 t -> spos:(int * int) -> size:(int * int) -> unit
      = "ml_canvas_blit"

    (* Direct pixel access *)

    external getPixel : 'kind t -> (int * int) -> Color.t
      = "ml_canvas_get_pixel"

    external putPixel : 'kind t -> (int * int) -> Color.t -> unit
      = "ml_canvas_put_pixel"

    external getImageData :
      'kind t -> pos:(int * int) -> size:(int * int) -> ImageData.t
      = "ml_canvas_get_image_data"

    external putImageData :
      'kind t -> dpos:(int * int) -> ImageData.t ->
      spos:(int * int) -> size:(int * int) -> unit
      = "ml_canvas_put_image_data"

    external importPNG_internal :
      'kind t -> pos:(int * int) -> string -> ('kind t -> unit) -> unit
      = "ml_canvas_import_png"

    let importPNG (c : 'kind t) ~pos filename : 'kind t React.event =
      let event, send_event = React.E.create () in
      importPNG_internal c ~pos filename
        (fun c -> Pending.add_canvas event send_event c);
      event

    external exportPNG : 'kind t -> string -> unit
      = "ml_canvas_export_png"

  end

  module Event = struct

    type timestamp = Int64.t

    type 'a canvas_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      data: 'a;
    }

    type position = int * int

    type size = int * int

    type key =
      (* Function *)
      | KeyEscape
      | KeyF1
      | KeyF2
      | KeyF3
      | KeyF4
      | KeyF5
      | KeyF6
      | KeyF7
      | KeyF8
      | KeyF9
      | KeyF10
      | KeyF11
      | KeyF12
      | KeyPrintScreen (* absent from Mac Keyboards *)
      | KeyScrollLock (* absent from Mac Keyboards *)
      | KeyPause (* absent from Mac Keyboards *)

      (* Alphanumeric, first row *)
      | KeyGraveTilde
      | Key1Exclamation
      | Key2At
      | Key3Number
      | Key4Dollar
      | Key5Percent
      | Key6Caret
      | Key7Ampersand
      | Key8Asterisk
      | Key9LParenthesis
      | Key0RParenthesis
      | KeyMinusUndersclre
      | KeyEqualPlus
      | KeyBackspace

      (* Alphanumeric, second row *)
      | KeyTab
      | KeyQ
      | KeyW
      | KeyE
      | KeyR
      | KeyT
      | KeyY
      | KeyU
      | KeyI
      | KeyO
      | KeyP
      | KeyLBracketCurly
      | KeyRBracketCurly
      | KeyBackslashPipe (* replaced by KeyNonUSNumberTilde on ISO KB *)

      (* Alphanumeric, third row *)
      | KeyCapsLock
      | KeyA
      | KeyS
      | KeyD
      | KeyF
      | KeyG
      | KeyH
      | KeyJ
      | KeyK
      | KeyL
      | KeySemicolonColon
      | KeyQuoteDoublequote
      | KeyNonUSNumberTilde (* extra key left of Return on ISO KB *)
      (* although generally mapped to KeyBackslashPipe *)
      | KeyReturn

      (* Alphanumeric, fourth row *)
      | KeyLShift
      | KeyNonUSBackslashPipe (* extra key right of LShift on ISO KB *)
      | KeyZ
      | KeyX
      | KeyC
      | KeyV
      | KeyB
      | KeyN
      | KeyM
      | KeyCommaLess
      | KeyPeriodGreater
      | KeySlashQuestion
      | KeyRShift

      (* Alphanumeric, fifth row *)
      | KeyLControl
      | KeyLMeta (* left Windows / Command key *)
      | KeyLAlt
      | KeySpacebar
      | KeyRAlt
      | KeyRMeta (* right Windows / Command key *)
      | KeyMenu
      | KeyRControl

      (* Control pad *)
      | KeyInsert (* replaced by a Fn key on Mac (with a different code) *)
      | KeyHome
      | KeyPageUp
      | KeyDeleteForward
      | KeyEend
      | KeyPageDown

      (* Arrow pad *)
      | KeyUpArrow
      | KeyLeftArrow
      | KeyDownArrow
      | KeyRightArrow

      (* Numeric pad *)
      | KeyPadNumlockClear (* on Mac, Clear replaces NumLock *)
      | KeyPadEquals (* on Mac keyboards only *)
      | KeyPadDivide
      | KeyPadMultiply
      | KeyPadMinus
      | KeyPad7Home
      | KeyPad8UpArrow
      | KeyPad9PageUp
      | KeyPadPlus
      | KeyPad4LeftArrow
      | KeyPad5
      | KeyPad6RightArrow
      | KeyPadComma (* specific to Brazilian keyboards *)
      | KeyPad1End
      | KeyPad2DownArrow
      | KeyPad3PageDown
      | KeyPad0Insert
      | KeyPadDecimalDelete
      | KeyPadEnter

      (* Extra function keys *)
      | KeyF13
      | KeyF14
      | KeyF15
      | KeyF16
      | KeyF17
      | KeyF18
      | KeyF19
      | KeyF20
      | KeyF21
      | KeyF22
      | KeyF23
      | KeyF24

      (* International & LANG keys *)
      | KeyInternational1 (* extra key left of RShift on JIS and Brazilian KB *)
      | KeyInternational2 (* Katakana/Hiragana key right of Space on JIS KB *)
      | KeyInternational3 (* extra key left of Backspace on JIS KB *)
      | KeyInternational4 (* Henkan key right of Space on JIS KB *)
      | KeyInternational5 (* Muhenkan key left of Space on JIS KB *)
      | KeyInternational6 (* Kanma (comma) key right of KP0 on JIS KB *)
      | KeyInternational7 (* Double-Byte/Single-Byte toggle key *)
      | KeyInternational8 (* Undefined *)
      | KeyInternational9 (* Undefined *)
      | KeyLang1 (* Hangul/English toggle key (Korean) *)
      | KeyLang2 (* Hanja conversion key (Korean) *)
      | KeyLang3 (* Katakana key (Japanese) *)
      | KeyLang4 (* Hiragana key (Japanese) *)
      | KeyLand5 (* Zenkaku/Hankaku key (Japanese) *)

      (* Extensions *)
      | KeyHelp
      | KeyMute
      | KeyVolumeUp
      | KeyVolumeDown

    type flags = {
      flag_shift : bool;
      flag_alt : bool;
      flag_control : bool;
      flag_meta : bool;
      flag_capslock : bool;
      flag_numlock : bool;
      flag_dead : bool;
    }

    type key_data = {
      key: key;
      char: Uchar.t;
      flags: flags;
    }

    type button =
      | ButtonNone
      | ButtonLeft
      | ButtonMiddle
      | ButtonRight
      | ButtonWheelUp
      | ButtonWheelDown

    type button_data = {
      position: position;
      button: button;
    }

    type 'a ev = 'a React.event * (?step:React.step -> 'a -> unit)

    let frame, send_frame =
      (React.E.create () : unit canvas_event ev)
    let focus_in, send_focus_in =
      (React.E.create () : unit canvas_event ev)
    let focus_out, send_focus_out =
      (React.E.create () : unit canvas_event ev)
    let resize, send_resize =
      (React.E.create () : size canvas_event ev)
    let move, send_move =
      (React.E.create () : position canvas_event ev)
    let close, send_close =
      (React.E.create () : unit canvas_event ev)
    let key_down, send_key_down =
      (React.E.create () : key_data canvas_event ev)
    let key_up, send_key_up =
      (React.E.create () : key_data canvas_event ev)
    let button_down, send_button_down =
      (React.E.create () : button_data canvas_event ev)
    let button_up, send_button_up =
      (React.E.create () : button_data canvas_event ev)
    let mouse_move, send_mouse_move =
      (React.E.create () : position canvas_event ev)
    (* let backend_stop, send_backend_stop =
       (React.E.create () : backend_stop_event ev) *)

    let event_timestamp, set_event_timestamp =
      React.S.create 0L

    external int_of_key : key -> int
      = "ml_canvas_int_of_key"

    external key_of_int : int -> key
      = "ml_canvas_key_of_int"

  end

  module InternalEvent = struct

    (* We declare constructors that are only used from the C code *)
    [@@@warning "-37"]

    open Event

    type frame_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
    }

    type focus_direction =
      | Out
      | In

    type canvas_focused_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      focus: focus_direction;
    }

    type canvas_resized_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      size: int * int;
    }

    type canvas_moved_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      position: int * int;
    }

    type canvas_closed_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
    }

    type state =
      | Up
      | Down

    type key_action_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      key: key;
      char: Uchar.t;
      flags: flags;
      state: state;
    }

    type button_action_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      position: int * int;
      button: button;
      state: state;
    }

    type mouse_move_event = {
      canvas: [`Onscreen] Canvas.t;
      timestamp: timestamp;
      position: int * int;
    }

    type t =
      | Frame of frame_event
      | CanvasFocused of canvas_focused_event
      | CanvasResized of canvas_resized_event
      | CanvasMoved of canvas_moved_event
      | CanvasClosed of canvas_closed_event
      | KeyAction of key_action_event
      | ButtonAction of button_action_event
      | MouseMove of mouse_move_event

  end

  module Backend = struct

    external init : unit -> unit
      = "ml_canvas_init"

    external run_internal :
      (InternalEvent.t -> unit) -> (unit -> 'dummy1) -> 'dummy2
      = "ml_canvas_run"

    external stop : unit -> unit
      = "ml_canvas_stop"

    external getCurrentTimestamp : unit -> Event.timestamp
      = "ml_canvas_get_current_timestamp"

    external getCanvas : int -> 'kind Canvas.t
      = "ml_canvas_get_canvas"

    let run k =
      let open InternalEvent in
      let open Event in
      let h e =
        (match e with
        | Frame { canvas; timestamp } ->
            let e = { canvas; timestamp; data = () } in
            set_event_timestamp e.timestamp; send_frame e
        | CanvasFocused { canvas; timestamp; focus = In } ->
            let e = { canvas; timestamp; data = () } in
            set_event_timestamp e.timestamp; send_focus_in e
        | CanvasFocused { canvas; timestamp; focus = Out } ->
            let e = { canvas; timestamp; data = () } in
            set_event_timestamp e.timestamp; send_focus_out e
        | CanvasResized { canvas; timestamp; size } ->
            let e = { canvas; timestamp; data = size } in
            set_event_timestamp e.timestamp; send_resize e
        | CanvasMoved { canvas; timestamp; position } ->
            let e = { canvas; timestamp; data = position } in
            set_event_timestamp e.timestamp; send_move e
        | CanvasClosed { canvas; timestamp } ->
            let e = { canvas; timestamp; data = () } in
            set_event_timestamp e.timestamp; send_close e
        | KeyAction { canvas; timestamp; key; char; flags; state = Down } ->
            let e = { canvas; timestamp; data = { key; char; flags } } in
            set_event_timestamp e.timestamp; send_key_down e
        | KeyAction { canvas; timestamp; key; char; flags; state = Up } ->
            let e = { canvas; timestamp; data = { key; char; flags } } in
            set_event_timestamp e.timestamp; send_key_up e
        | ButtonAction { canvas; timestamp; position; button; state = Down } ->
            let e = { canvas; timestamp; data = { position; button } } in
            set_event_timestamp e.timestamp; send_button_down e
        | ButtonAction { canvas; timestamp; position; button; state = Up } ->
            let e = { canvas; timestamp; data = { position; button } } in
            set_event_timestamp e.timestamp; send_button_up e
        | MouseMove { canvas; timestamp; position } ->
            let e = { canvas; timestamp; data = position } in
            set_event_timestamp e.timestamp; send_mouse_move e);
        Pending.process ()
      in
      run_internal h k

  end

end
