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
      let invdet = 1.0 /. (t.a *. t.d -. t.b *. t.c) in
      { a =    t.d *. invdet; b = -. t.b *. invdet;
        c = -. t.c *. invdet; d =    t.a *. invdet;
        e = (t.c *. t.f -. t.d *. t.e) *. invdet;
        f = (t.b *. t.e -. t.a *. t.f) *. invdet }

  end

  module Point = struct

    type t = (float * float)

    let translate (x, y) ~by:(a, b) =
      (x +. a, y +. b)

    let rotate (x, y) ~around:(cx, cy) ~theta =
      ((x -. cx) *. (cos (-. theta)) +. (y -. cy) *. (sin (-. theta)) +. cx,
       (y -. cy) *. (cos (-. theta)) -. (x -. cx) *. (sin (-. theta)) +. cy)

    let transform (x, y) (t: Transform.t) =
      (x *. t.a +. y *. t.c +. t.e,
       x *. t.b +. y *. t.d +. t.f)

    let barycenter a (x1, y1) b (x2, y2) =
      (a *. x1 +. b *. x2) /. (a +. b),
      (a *. y1 +. b *. y2) /. (a +. b)

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

  module ImageData = struct

    type t =
      (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t

    let create (width, height) =
      let a = Bigarray.Array3.create Bigarray.int8_unsigned
                Bigarray.c_layout height width 4 in
      Bigarray.Array3.fill a 0;
      a

    external createFromPNG : string -> t
      = "ml_canvas_image_data_create_from_png"

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

    external importPNG : t -> pos:(int * int) -> string -> unit
      = "ml_canvas_image_data_import_png"

    external exportPNG : t -> string -> unit
      = "ml_canvas_image_data_export_png"

  end

  module Gradient = struct

    type t

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

    external arcTo :
      t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
      = "ml_canvas_path_arc_to"

    external quadraticCurveTo :
      t -> cp:Point.t -> p:Point.t -> unit
      = "ml_canvas_path_quadratic_curve_to"

    external bezierCurveTo :
      t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
      = "ml_canvas_path_bezier_curve_to"

    external rect :
      t -> pos:Point.t -> size:(float * float) -> unit
      = "ml_canvas_path_rect"

    external ellipse :
      t -> center:Point.t -> radius:(float * float) ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_path_ellipse" "ml_canvas_path_ellipse_n"

    external add : t -> t -> unit
      = "ml_canvas_path_add"

    external addTransformed : t -> t -> Transform.t -> unit
      = "ml_canvas_path_add_transformed"

  end

  module Canvas = struct

    type 'a t

    type line_join =
      | Round
      | Miter
      | Bevel

    type line_cap =
      | Butt
      | Square
      | RoundCap

    type style =
      | Color of Color.t
      | Gradient of Gradient.t
      | Pattern of Pattern.t

    type composite_op =
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

    (* Gradients *)

    external createLinearGradient :
      'a t -> pos1:Point.t -> pos2:Point.t -> Gradient.t
      = "ml_canvas_create_linear_gradient"

    external createRadialGradient :
      'a t -> center1:Point.t -> rad1:float ->
      center2:Point.t -> rad2:float -> Gradient.t
      = "ml_canvas_create_radial_gradient"

    external createConicGradient :
      'a t -> center:Point.t -> angle:float -> Gradient.t
      = "ml_canvas_create_conic_gradient"

    (* Patterns *)

    external createPattern :
      'a t -> ImageData.t -> Pattern.repeat -> Pattern.t
      = "ml_canvas_create_pattern"

    (* Comparison *)

    external hash : 'a t -> int
      = "ml_canvas_hash"

    external compare : 'a t -> 'b t -> int
      = "ml_canvas_compare"

    let equal c1 c2 = compare c1 c2 = 0

    let (=) c1 c2 = compare c1 c2 = 0
    let (<>) c1 c2 = compare c1 c2 <> 0
    let (<) c1 c2 = compare c1 c2 < 0
    let (>) c1 c2 = compare c1 c2 > 0
    let (<=) c1 c2 = compare c1 c2 <= 0
    let (>=) c1 c2 = compare c1 c2 >= 0

    external (==) : 'a t -> 'b t -> bool = "%eq"
    external (!=) : 'a t -> 'b t -> bool = "%noteq"

    (* Creation *)

    external createFramed :
      string -> pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
      = "ml_canvas_create_framed"

    external createFrameless :
      pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
      = "ml_canvas_create_frameless"

    external createOffscreen : size:(int * int) -> [> `Offscreen] t
      = "ml_canvas_create_offscreen"

    external createOffscreenFromImageData : ImageData.t -> [> `Offscreen] t
      = "ml_canvas_create_offscreen_from_image_data"

    external createOffscreenFromPNG : string -> [> `Offscreen] t
      = "ml_canvas_create_offscreen_from_png"

    (* Visibility *)

    external show : [< `Onscreen] t -> unit
      = "ml_canvas_show"

    external hide : [< `Onscreen] t -> unit
      = "ml_canvas_hide"

    external close : [> `Onscreen] t -> unit
      = "ml_canvas_close"

    (* Configuration *)

    external getId : 'a t -> int
      = "ml_canvas_get_id"

    external getSize : 'a t -> (int * int)
      = "ml_canvas_get_size"

    external setSize : 'a t -> (int * int) -> unit
      = "ml_canvas_set_size"

    external getPosition : [< `Onscreen] t -> (int * int)
      = "ml_canvas_get_position"

    external setPosition : [< `Onscreen] t -> (int * int) -> unit
      = "ml_canvas_set_position"

    (* State *)

    external save : 'a t -> unit
      = "ml_canvas_save"

    external restore : 'a t -> unit
      = "ml_canvas_restore"

    (* Transformations *)
    external setTransform :
      'a t -> (float * float * float * float * float * float) -> unit
      = "ml_canvas_set_transform"

    external transform :
      'a t -> (float * float * float * float * float * float) -> unit
      = "ml_canvas_transform"

    external translate : 'a t -> (float * float) -> unit
      = "ml_canvas_translate"

    external scale : 'a t -> (float * float) -> unit
      = "ml_canvas_scale"

    external shear : 'a t -> (float * float) -> unit
      = "ml_canvas_shear"

    external rotate : 'a t -> float -> unit
      = "ml_canvas_rotate"

    (* Style / config *)

    external getLineWidth : 'a t -> float
      = "ml_canvas_get_line_width"

    external setLineWidth : 'a t -> float -> unit
      = "ml_canvas_set_line_width"

    external getLineJoin : 'a t -> line_join
      = "ml_canvas_get_line_join"

    external setLineJoin : 'a t -> line_join -> unit
      = "ml_canvas_set_line_join"

    external getLineCap : 'a t -> line_cap
      = "ml_canvas_get_line_cap"

    external setLineCap : 'a t -> line_cap -> unit
      = "ml_canvas_set_line_cap"

    external getMiterLimit : 'a t -> float
      = "ml_canvas_get_miter_limit"

    external setMiterLimit : 'a t -> float -> unit
      = "ml_canvas_set_miter_limit"

    external getLineDashOffset : 'a t -> float
      = "ml_canvas_get_line_dash_offset"

    external setLineDashOffset : 'a t -> float -> unit
      = "ml_canvas_set_line_dash_offset"

    external getLineDash : 'a t -> float array
      = "ml_canvas_get_line_dash"

    external setLineDash : 'a t -> float array -> unit
      = "ml_canvas_set_line_dash"

    external getStrokeColor : 'a t -> Color.t
      = "ml_canvas_get_stroke_color"

    external setStrokeColor : 'a t -> Color.t -> unit
      = "ml_canvas_set_stroke_color"

    external setStrokeGradient : 'a t -> Gradient.t -> unit
      = "ml_canvas_set_stroke_gradient"

    external setStrokePattern : 'a t -> Pattern.t -> unit
      = "ml_canvas_set_stroke_pattern"

    external getStrokeStyle : 'a t -> style
      = "ml_canvas_get_stroke_style"

    external setStrokeStyle : 'a t -> style -> unit
      = "ml_canvas_set_stroke_style"

    external getFillColor : 'a t -> Color.t
      = "ml_canvas_get_fill_color"

    external setFillColor : 'a t -> Color.t -> unit
      = "ml_canvas_set_fill_color"

    external setFillGradient : 'a t -> Gradient.t -> unit
      = "ml_canvas_set_fill_gradient"

    external setFillPattern : 'a t -> Pattern.t -> unit
      = "ml_canvas_set_fill_pattern"

    external getFillStyle : 'a t -> style
      = "ml_canvas_get_fill_style"

    external setFillStyle : 'a t -> style -> unit
      = "ml_canvas_set_fill_style"

    external getGlobalAlpha : 'a t -> float
      = "ml_canvas_get_global_alpha"

    external setGlobalAlpha : 'a t -> float -> unit
      = "ml_canvas_set_global_alpha"

    external getGlobalCompositeOperation : 'a t -> composite_op
      = "ml_canvas_get_global_composite_operation"

    external setGlobalCompositeOperation : 'a t -> composite_op -> unit
      = "ml_canvas_set_global_composite_operation"

    external getShadowColor : 'a t -> Color.t
      = "ml_canvas_get_shadow_color"

    external setShadowColor : 'a t -> Color.t -> unit
      = "ml_canvas_set_shadow_color"

    external getShadowBlur : 'a t -> float
      = "ml_canvas_get_shadow_blur"

    external setShadowBlur : 'a t -> float -> unit
      = "ml_canvas_set_shadow_blur"

    external getShadowOffset : 'a t -> (float * float)
      = "ml_canvas_get_shadow_offset"

    external setShadowOffset : 'a t -> (float * float) -> unit
      = "ml_canvas_set_shadow_offset"

    external setFont :
      'a t -> string -> size:Font.size -> slant:Font.slant ->
      weight:Font.weight -> unit
      = "ml_canvas_set_font"

    external setFont_ :
      'a t -> string -> Font.size -> Font.slant -> Font.weight -> unit
      = "ml_canvas_set_font"

    (* Paths *)

    external clearPath : 'a t -> unit
      = "ml_canvas_clear_path"

    external closePath : 'a t -> unit
      = "ml_canvas_close_path"

    external moveTo : 'a t -> Point.t -> unit
      = "ml_canvas_move_to"

    external lineTo : 'a t -> Point.t -> unit
      = "ml_canvas_line_to"

    external arc :
      'a t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_arc" "ml_canvas_arc_n"

    external arcTo :
      'a t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
      = "ml_canvas_arc_to"

    external quadraticCurveTo :
      'a t -> cp:Point.t -> p:Point.t -> unit
      = "ml_canvas_quadratic_curve_to"

    external bezierCurveTo :
      'a t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
      = "ml_canvas_bezier_curve_to"

    external rect :
      'a t -> pos:Point.t -> size:(float * float) -> unit
      = "ml_canvas_rect"

    external ellipse :
      'a t -> center:Point.t -> radius:(float * float) ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
      = "ml_canvas_ellipse" "ml_canvas_ellipse_n"

    external fill : 'a t -> nonzero:bool -> unit
      = "ml_canvas_fill"

    external fillPath : 'a t -> Path.t -> nonzero:bool -> unit
      = "ml_canvas_fill_path"

    external stroke : 'a t -> unit
      = "ml_canvas_stroke"

    external strokePath : 'a t -> Path.t -> unit
      = "ml_canvas_stroke_path"

    external clip : 'a t -> nonzero:bool -> unit
      = "ml_canvas_clip"

    external clipPath : 'a t -> Path.t -> nonzero:bool -> unit
      = "ml_canvas_clip_path"

    (* Immediate drawing *)

    external fillRect :
      'a t -> pos:Point.t -> size:(float * float) -> unit
      = "ml_canvas_fill_rect"

    external strokeRect :
      'a t -> pos:Point.t -> size:(float * float) -> unit
      = "ml_canvas_stroke_rect"

    external fillText :
      'a t -> string -> Point.t -> unit
      = "ml_canvas_fill_text"

    external strokeText :
      'a t -> string -> Point.t -> unit
      = "ml_canvas_stroke_text"

    external blit :
      dst:'a t -> dpos:(int * int) ->
      src:'b t -> spos:(int * int) -> size:(int * int) -> unit
      = "ml_canvas_blit"

    (* Direct pixel access *)

    external getPixel : 'a t -> (int * int) -> Color.t
      = "ml_canvas_get_pixel"

    external putPixel : 'a t -> (int * int) -> Color.t -> unit
      = "ml_canvas_put_pixel"

    external getImageData :
      'a t -> pos:(int * int) -> size:(int * int) -> ImageData.t
      = "ml_canvas_get_image_data"

    external putImageData :
      'a t -> dpos:(int * int) -> ImageData.t ->
      spos:(int * int) -> size:(int * int) -> unit
      = "ml_canvas_put_image_data"

    external exportPNG : 'a t -> string -> unit
      = "ml_canvas_export_png"

    external importPNG : 'a t -> pos:(int * int) -> string -> unit
      = "ml_canvas_import_png"

  end

  module Event = struct

    type timestamp = Int64.t

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

    type button =
      | ButtonNone
      | ButtonLeft
      | ButtonMiddle
      | ButtonRight
      | ButtonWheelUp
      | ButtonWheelDown

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
      position: int * int; (** Cursor position when the event occured *)
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

    external int_of_key : key -> int
      = "ml_canvas_int_of_key"

    external key_of_int : int -> key
      = "ml_canvas_key_of_int"

  end

  module Backend = struct

    type _ backend_type =
      | Canvas : [`JS] backend_type
      | GDI : [`Win32] backend_type
      | Quartz : [`OSX] backend_type
      | X11 : [<`Unix | `OSX | `Win32] backend_type
      | Wayland : [`Unix] backend_type

    type options = {
      js_backends: [`JS] backend_type list;
      win32_backends: [`Win32] backend_type list;
      osx_backends: [`OSX] backend_type list;
      unix_backends: [`Unix] backend_type list;
    }

    let default_options = {
      js_backends = [Canvas];
      win32_backends = [GDI; X11];
      osx_backends = [Quartz; X11];
      unix_backends = [Wayland; X11];
    }

    external init : options -> unit
      = "ml_canvas_init"

    external run : (Event.t-> bool) -> (unit -> 'a) -> 'b
      = "ml_canvas_run"

    external stop : unit -> unit
      = "ml_canvas_stop"

    external getCanvas : int -> 'a Canvas.t option
      = "ml_canvas_get_canvas"

  end

end
