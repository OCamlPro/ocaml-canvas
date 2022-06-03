(**************************************************************************)
(*                                                                        *)
(*    Copyright 2022 OCamlPro                                             *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

exception CanvasDestroyed

let () = Callback.register_exception "canvas_destroyed" CanvasDestroyed

module Color = struct

  type t = int

  let clip_8 i =
    if i < 0 then 0
    else if i > 0xFF then 0xFF
    else i

  let clip_24 i =
    if i < 0 then 0
    else if i > 0xFFFFFF then 0xFFFFFF
    else i

  let of_rgb r g b = (clip_8 r) * 0x10000 + (clip_8 g) * 0x100 + (clip_8 b)
  let to_rgb c = (c / 0x10000 mod 0x100), (c / 0x100 mod 0x100), (c mod 0x100)
  let of_int i = clip_24 i
  let to_int c = c

  module StringMap = Map.Make(String)
  let colors = ref StringMap.empty
  let define_color name c =
    colors := StringMap.add (String.lowercase_ascii name) c !colors;
    c

  let black = define_color "black" 0x000000
  let white = define_color "white" 0xFFFFFF
  let blue = define_color "blue" 0x0000FF
  let cyan = define_color "cyan" 0x00FFFF
  let green = define_color "green" 0x008000
  let lime = define_color "lime" 0x00FF00
  let orange = define_color "orange" 0xFFA500
  let pink = define_color "pink" 0xFFC0CB
  let red = define_color "red" 0xFF0000

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

module Canvas = struct

  type 'a t

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

  (* Creation / destruction *)

  external createFramed :
    string -> pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
    = "ml_canvas_create_framed"

  external createFramed_ :
    string -> (int * int) -> (int * int) -> [> `Onscreen] t
    = "ml_canvas_create_framed"

  external createFrameless :
    pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
    = "ml_canvas_create_frameless"

  external createFrameless_ : (int * int) -> (int * int) -> [> `Onscreen] t
    = "ml_canvas_create_frameless"

  external createOffscreen : size:(int * int) -> [> `Offscreen] t
    = "ml_canvas_create_offscreen"

  external createOffscreen_ : (int * int) -> [> `Offscreen] t
    = "ml_canvas_create_offscreen"

  external destroy : 'a t -> unit
    = "ml_canvas_destroy"

  (* Visibility *)

  external show : [< `Onscreen] t -> unit
    = "ml_canvas_show"

  external hide : [< `Onscreen] t -> unit
    = "ml_canvas_hide"

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

  external getStrokeColor : 'a t -> Color.t
    = "ml_canvas_get_stroke_color"

  external setStrokeColor : 'a t -> Color.t -> unit
    = "ml_canvas_set_stroke_color"

  external getFillColor : 'a t -> Color.t
    = "ml_canvas_get_fill_color"

  external setFillColor : 'a t -> Color.t -> unit
    = "ml_canvas_set_fill_color"

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

  external moveTo : 'a t -> (float * float) -> unit
    = "ml_canvas_move_to"

  external lineTo : 'a t -> (float * float) -> unit
    = "ml_canvas_line_to"

  external arc :
    'a t -> center:(float * float) -> radius:float ->
    theta1:float -> theta2:float -> ccw:bool -> unit
    = "ml_canvas_arc" "ml_canvas_arc_n"

  external arc_ :
    'a t -> (float * float) -> float -> float -> float -> bool -> unit
    = "ml_canvas_arc" "ml_canvas_arc_n"

  external arcTo :
    'a t -> p1:(float * float) -> p2:(float * float) -> radius:float -> unit
    = "ml_canvas_arc_to"

  external arcTo_ :
    'a t -> (float * float) -> (float * float) -> float -> unit
    = "ml_canvas_arc_to"

  external quadraticCurveTo :
    'a t -> cp:(float * float) -> p:(float * float) -> unit
    = "ml_canvas_quadratic_curve_to"

  external quadraticCurveTo_ :
    'a t -> (float * float) -> (float * float) -> unit
    = "ml_canvas_quadratic_curve_to"

  external bezierCurveTo :
    'a t -> cp1:(float * float) -> cp2:(float * float) ->
    p:(float * float) -> unit
    = "ml_canvas_bezier_curve_to"

  external bezierCurveTo_ :
    'a t -> (float * float) -> (float * float) -> (float * float) -> unit
    = "ml_canvas_bezier_curve_to"

  external rect :
    'a t -> pos:(float * float) -> size:(float * float) -> unit
    = "ml_canvas_rect"

  external rect_ :
    'a t -> (float * float) -> (float * float) -> unit
    = "ml_canvas_rect"

  external ellipse :
    'a t -> center:(float * float) -> radius:(float * float) ->
    rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
    = "ml_canvas_ellipse" "ml_canvas_ellipse_n"

  external ellipse_ :
    'a t -> (float * float) -> (float * float) ->
    float -> float -> float -> bool -> unit
    = "ml_canvas_ellipse" "ml_canvas_ellipse_n"

  external fill : 'a t -> nonzero:bool -> unit
    = "ml_canvas_fill"

  external fill_ : 'a t -> bool -> unit
    = "ml_canvas_fill"

  external stroke : 'a t -> unit
    = "ml_canvas_stroke"

  (* Immediate drawing *)

  external fillRect :
    'a t -> pos:(float * float) -> size:(float * float) -> unit
    = "ml_canvas_fill_rect"

  external fillRect_ :
    'a t -> (float * float) -> (float * float) -> unit
    = "ml_canvas_fill_rect"

  external strokeRect :
    'a t -> pos:(float * float) -> size:(float * float) -> unit
    = "ml_canvas_stroke_rect"

  external strokeRect_ :
    'a t -> (float * float) -> (float * float) -> unit
    = "ml_canvas_stroke_rect"

  external fillText :
    'a t -> string -> (float * float) -> unit
    = "ml_canvas_fill_text"

  external strokeText :
    'a t -> string -> (float * float) -> unit
    = "ml_canvas_stroke_text"

  external blit :
    dst:'a t -> dpos:(int * int) ->
    src:'b t -> spos:(int * int) -> size:(int * int) -> unit
    = "ml_canvas_blit"

  external blit_ :
    'a t -> (int * int) -> 'b t -> (int * int) -> (int * int) -> unit
    = "ml_canvas_blit"

  (* Direct pixel access *)

  external getPixel : 'a t -> (int * int) -> Color.t
    = "ml_canvas_get_pixel"

  external setPixel : 'a t -> (int * int) -> Color.t -> unit
    = "ml_canvas_set_pixel"

  type image_data =
    (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t

  external getImageData :
    'a t -> pos:(int * int) -> size:(int * int) -> image_data
    = "ml_canvas_get_image_data"

  external getImageData_ :
    'a t -> (int * int) -> (int * int) -> image_data
    = "ml_canvas_get_image_data"

  external setImageData :
    'a t -> dpos:(int * int) -> image_data ->
    spos:(int * int) -> size:(int * int) -> unit
    = "ml_canvas_set_image_data"

  external setImageData_ :
    'a t -> (int * int) -> image_data -> (int * int) -> (int * int) -> unit
    = "ml_canvas_set_image_data"

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
