(**************************************************************************)
(*                                                                        *)
(*    Copyright 2022 OCamlPro                                             *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

(** The OCaml-Canvas library

    The OCaml-Canvas library provides a portable Canvas framework for OCaml.
    It features an interface similar to HTML5 Canvas, and can render to
    native X11, MS Windows or macOS windows. It also features a small
    wrapper around HTML5 Canvas, allowing to use the library in any web
    browser. In addition, it allows to handle keyboard and mouse events.

    {1 Quick start}

    Before using any function in the library (and assuming the `OCamlCanvas`
    module has been opened), the user should call {!Backend.init} so that
    the library makes any internal initialization it needs for the current
    backend. This function takes as input a value of type {!Backend.options},
    allowing to tweak specific features of each backend.
    It is however recommended to simply use the default options,
    i.e {!Backend.default_options}.

    Once the backend is initialized, one can create Canvas objects
    using the {!Canvas.createFramed}, {!Canvas.createFrameless} and
    {!Canvas.createOffscreen} functions. The first one creates a canvas
    contained in a regular window (which is simulated in the Javascript
    backend). The second one creates a window that has no decoration at
    all. The last one creates canvas that are not rendered on screen,
    which can be useful to save complex images that can then simply
    copied to a visible canvas. Onscreen canvas are hidden by default,
    and {!Canvas.show} should be called on them to make them visible.

    Drawing on the canvas can be perfomed using various drawing primitives,
    the most ubiquitous being {!Canvas.clearPath}, {!Canvas.moveTo},
    {!Canvas.lineTo}, {!Canvas.arc}, {!Canvas.bezierCurveTo}, {!Canvas.fill}
    and {!Canvas.stroke}. These function allow to build a path step by step
    and either fill it completely or draw its outline. It is also possible
    to directly render some text with the {!Canvas.fillText} and
    {!Canvas.strokeText} functions.

    The Canvas drawing style can be customized using the functions
    {!Canvas.setFillColor}, {!Canvas.setStrokeColor} and
    {!Canvas.setLineWidth}. The font used to draw text can be specified
    with the {!Canvas.setFont} function. It is also possible to apply
    various transformations to a canvas, such as translation, rotation and
    scaling, with the functions {!Canvas.transform}, {!Canvas.translate},
    {!Canvas.scale}, {!Canvas.rotate} and {!Canvas.shear}. All these
    styling elements can be saved and restored using the functions
    {!Canvas.save} and {!Canvas.restore}.

    Once the canvas are ready, we may start handling events for these canvas.
    To do so, we use the {!Backend.run} function, which runs an event loop.
    This function MUST be the last instruction of the program. It takes as
    argument two functions: the first one is an event handler, and the second
    one is executed when the event loop has finished running. The event loop
    may be stopped by calling {!Backend.stop} in the event handler.

    The event handler function should pattern-match its argument against
    the constructors of the {!Event.t} type it is interested in, and return
    a boolean value indicating whether it processed the event (for some
    events, this may have some side-effect, indicated in the event's
    description). Each event reports at least the Canvas on which it
    occured, and its timestamp. It may also report mouse coordinates
    for mouse events, or keyboard status for keyboard events.

    {1 An actual example}

    The following program creates a windowed canvas with an orange background,
    a cyan border, and the "Hello world !" text drawn rotated in the middle.
    The user may press the "Escape" key to exit the program.

{[
    open OCamlCanvas

    let () =

      Backend.(init default_options);

      let c = Canvas.createFramed "Hello world"
                ~pos:(300, 200) ~size:(300, 200) in

      Canvas.setFillColor c Color.orange;
      Canvas.fillRect c ~pos:(0.0, 0.0) ~size:(300.0, 200.0);

      Canvas.setStrokeColor c Color.cyan;
      Canvas.setLineWidth c 10.0;
      Canvas.clearPath c;
      Canvas.moveTo c (5.0, 5.0);
      Canvas.lineTo c (295.0, 5.0);
      Canvas.lineTo c (295.0, 195.0);
      Canvas.lineTo c (5.0, 195.0);
      Canvas.closePath c;
      Canvas.stroke c;

      Canvas.setFont c "Liberation Sans" ~size:36.0
        ~slant:Font.Roman ~weight:Font.bold;

      Canvas.setFillColor c (Color.of_rgb 0 64 255);
      Canvas.setLineWidth c 1.0;
      Canvas.save c;
      Canvas.translate c (150.0, 100.0);
      Canvas.rotate c (-. Float.pi /. 8.0);
      Canvas.fillText c "Hello world !" (-130.0, 20.0);
      Canvas.restore c;

      Canvas.show c;

      Backend.run (function

        | Event.KeyAction { canvas = _; timestamp = _;
                            key; char = _; flags = _; state = Down } ->
            if key = Event.KeyEscape then
              Backend.stop ();
            true

        | Event.ButtonAction { canvas = _; timestamp = _;
                               position = (x, y); button = _; state = Down } ->
            Canvas.setFillColor c Color.red;
            Canvas.clearPath c;
            Canvas.arc c ~center:(float_of_int x, float_of_int y)
              ~radius:5.0 ~theta1:0.0 ~theta2:(pi *. 2.0) ~ccw:false;
            Canvas.fill c ~nonzero:false;
            true

        | Event.Frame { canvas = _; timestamp = _ } ->
            true

        | _ ->
            false

        ) (function () ->
             Printf.printf "Goodbye !\n"
        )
]}
 *)

module Color : sig
(** Color description and manipulation functions *)

  type t
  (** Abstract type for colors *)

  val of_rgb : int -> int -> int -> t
  (** [of_rgb r g b] creates a color from its [r], [g] and [b] components *)

  val of_argb : int -> int -> int -> int -> t
  (** [of_argb a r g b] creates a color from its [a], [r], [g], [b] components *)

  val to_rgb : t -> int * int * int
  (** [to_rgb c] converts a color to its [r], [g] and [b] components *)

  val of_int : int -> t
  (** [of_int i] creates a color from its integer representation [i] *)

  val to_int : t -> int
  (** [to_int c] converts a color [c] to its integer representation *)

  val black : t
  (** Predefined `black` color *)

  val white : t
  (** Predefined `white` color *)

  val blue : t
  (** Predefined `blue` color *)

  val cyan : t
  (** Predefined `cyan` color *)

  val green : t
  (** Predefined `green` color *)

  val lime : t
  (** Predefined `lime` color *)

  val orange : t
  (** Predefined `orange` color *)

  val pink : t
  (** Predefined `pink` color *)

  val red : t
  (** Predefined `red` color *)

  val of_string : string -> t
  (** [of_string s] returns the color associated with string [s] *)

  val define_color : string -> t -> t
  (** [define_color s c] add a color [c] named [n] to the internal color map *)

end

module Font : sig
(** Font description *)

  type size = float
  (** Font size *)

  type slant =
    | Roman
    | Italic
    | Oblique (**)
  (** Font slant *)

  type weight = int
  (** Font weight *)

  val thin : weight
  (** Predefined `thin` weight *)

  val extraLight : weight
  (** Predefined `extraLight` weight *)

  val light : weight
  (** Predefined `light` weight *)

(*val semiLight : weight *)
(*val book : weight *)

  val regular : weight
  (** Predefined `regular` weight *)

  val medium : weight
  (** Predefined `medium` weight *)

  val semiBold : weight
  (** Predefined `semiBold` weight *)

  val bold : weight
  (** Predefined `bold` weight *)

  val extraBold : weight
  (** Predefined `extraBold` weight *)

  val black : weight
  (** Predefined `black` weight *)

(*val extraBlack : weight *)

end

module ImageData : sig
(** Image data manipulation functions *)

  type t =
    (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t
  (** Image data are big arrays of dimension 3 (width, height, component),
      where the components are in BGRA order *)

  val createFromPNG : string -> t
  (** [createFromPNG filename] creates an image data
      with the contents of PNG file [filename] *)

  val importPNG : t -> pos:(int * int) -> string -> unit
  (** [importPNG id ~pos filename] loads the file [filename] into
      image data [id] at position [pos] *)

  val exportPNG : t -> string -> unit
  (** [exportPNG id filename] saves the contents of image data [id]
      to a file with name [filename] *)

end

module Gradient : sig

  type t
  (** An abstract type representing a gradient *)

  val addColorStop : t -> Color.t -> float -> unit
  (** [addColorStop] gradient color stop) adds a new [color]
      color spot in the gradient object at position [stop] *)

end

module Canvas : sig
(** Canvas manipulation functions *)

  type 'a t
  (** An abstract type representing a canvas *)

  type style =
    | ColorStyle of Color.t
    | GradientStyle of Gradient.t
    (** A type to represent stroke and fill styles *)

  type line_join =
    | Round
    | Miter
    | Bevel
    (** An enum type for representing line join types *)

  type line_cap =
    | Butt
    | Square
    | RoundCap
    (** An enum type for representing line cap types *)

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
    (** A type to represent blending and compositing operations *)


  (** {1 Gradient creation functions} *)

  val createLinearGradient :
    'a t -> pos1:(float * float) -> pos2:(float * float) -> Gradient.t
  (** [createLinearGradient c ~pos1 ~pos2] creates a new linear
      gradient parallel to the line ([pos1][pos2]) in window
      coordinates for canvas [c] *)

  val createLinearGradient_:
    'a t -> (float * float) -> (float * float) -> Gradient.t
  (** [createLinearGradient_ c pos1 pos2] is a labelless
      equivalent of [createLinearGradient c ~pos1 ~pos] *)

  val createRadialGradient:
    'a t -> center1:(float * float) -> rad1:float ->
    center2:(float * float) -> rad2:float -> Gradient.t
  (** [createRadialGradient c ~center1 ~rad1 ~center2 ~rad2] creates a new
      radial gradient between the disks with centers [center1] and [center2]
      and radi [rad1] and [rad2] in window coordinates for canvas [c] *)

  val createRadialGradient_ :
    'a t -> (float * float) -> float -> (float * float) -> float -> Gradient.t
  (** [createRadialGradient_ c center1 rad1 center2 rad2] is a labelless
      equivalent of [createLinearGradient c ~center1 ~rad1 ~center2 ~rad2] *)

  val createConicGradient:
    'a t -> center:(float * float) -> angle:float -> Gradient.t
  (** [createConicGradient c ~center ~angle] creates a new conic gradient
      with center [center] and initial angle [angle] for canvas [c] *)

  val createConicGradient_ :
    'a t -> (float * float) -> float -> Gradient.t
  (** [createConicGradient c center angle] is a labelless
      equivalent of [createConicGradient canvas ~center ~angle] *)

  (** {1 Comparison functions} *)

  (** In order to ease identification of canvas or building collections
      of canvas, the usual comparison functions are provided.
      These functions simply operate on the canvas' unique ids. *)

  val hash : 'a t -> int
  (** [hash c] returns a unique integer value for canvas [c],
      which happens to be the same as the unique canvas id [getId c].
      For deleted canvas, this returns 0. *)

  val compare : 'a t -> 'b t -> int
  (** [compare c1 c2] is equivalent to [compare (getId c1) (getId c2)] *)

  val equal : 'a t -> 'b t -> bool
  (** [equal c1 c2] is equivalent to [equal (getId c1) (getId c2)] *)

  val (=) : 'a t -> 'b t -> bool
  (** [c1 = c2] is equivalent to [(getId c1) = (getId c2)] *)

  val (<>) : 'a t -> 'b t -> bool
  (** [c1 <> c2] is equivalent to [(getId c1) <> (getId c2)] *)

  val (<) : 'a t -> 'b t -> bool
  (** [c1 < c2] is equivalent to [(getId c1) < (getId c2)] *)

  val (>) : 'a t -> 'b t -> bool
  (** [c1 > c2] is equivalent to [(getId c1) > (getId c2)] *)

  val (<=) : 'a t -> 'b t -> bool
  (** [c1 <= c2] is equivalent to [(getId c1) <= (getId c2)] *)

  val (>=) : 'a t -> 'b t -> bool
  (** [c1 >= c2] is equivalent to [(getId c1) >= (getId c2)] *)

  val (==) : 'a t -> 'b t -> bool
  (** [c1 == c2] test for physical equality of canvas [c1] and [c2] *)

  val (!=) : 'a t -> 'b t -> bool
  (** [c1 != c2] test for physical inequality of canvas [c1] and [c2] *)

  (** {1 Creation} *)

  val createFramed :
    string -> pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
  (** [createFramed title ~pos ~size] creates a canvas in a window
      with title [title] at position [pos] and of size [size] *)

  val createFramed_ : string -> (int * int) -> (int * int) -> [> `Onscreen] t
  (** [createFramed_ title pos size] is a labelless equivalent of
      [createFramed title ~pos ~size]  *)

  val createFrameless : pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
  (** [createFrameless ~pos ~size] creates a canvas in an
      undecorated window at position [pos] and of size [size] *)

  val createFrameless_ : (int * int) -> (int * int) -> [> `Onscreen] t
  (** [createFrameless_ pos size] is a labelless equivalent of
      [createFrameless ~pos ~size]  *)

  val createOffscreen : size:(int * int) -> [> `Offscreen] t
  (** [createOffscreen ~size] creates an offscreen canvas of size [size] *)

  val createOffscreen_ : (int * int) -> [> `Offscreen] t
  (** [createOffscreen_ size] is a labelless equivalent of
      [createOffscreen ~size]  *)

  val createOffscreenFromImageData : ImageData.t -> [> `Offscreen] t
  (** [createOffscreenFromImageData id] creates an offscreen canvas
      with the contents of image data [id] *)

  val createOffscreenFromPNG : string -> [> `Offscreen] t
  (** [createOffscreen filename] creates an offscreen canvas
      with the contents of PNG file [filename] *)

  (** {1 Visibility} *)

  val show : [< `Onscreen] t -> unit
  (** [show c] makes the canvas [c] visible on screen.
      Does not apply to offscreen canvas. *)

  val hide : [< `Onscreen] t -> unit
  (** [hide c] makes the canvas [c] invisible.
      Does not apply to offscreen canvas. *)

  val close : [> `Onscreen] t -> unit
  (** [close c] closes the canvas [c], i.e. it permanently removes it from
      the screen and prevents it to receive events ; however it can still
      be used as an offscreen canvas. *)

  (** {1 Configuration} *)

  val getId : 'a t -> int
  (** [getId c] returns the unique id of canvas [c],
      or 0 if the canvas has been destroyed *)

  val getSize : 'a t -> (int * int)
  (** [getSize c] returns the size of canvas [c] *)

  val setSize : 'a t -> (int * int) -> unit
  (** [setSize c size] sets the size of canvas [c] *)

  val getPosition : [< `Onscreen] t -> (int * int)
  (** [getPosition c] returns the position of canvas [c] *)

  val setPosition : [< `Onscreen] t -> (int * int) -> unit
  (** [setPosition c pos] sets the position of canvas [c] *)

  (** {1 State} *)

  val save : 'a t -> unit
  (** [save c] pushes the current state of canvas [c] onto the state stack *)

  val restore : 'a t -> unit
  (** [restore c] pops the current state of canvas [c] from the state stack *)

  (** {1 Transformations} *)

  val setTransform :
    'a t -> (float * float * float * float * float * float) -> unit
  (** [setTransform c t] sets the current transformation matrix for canvas [c].
      The matrix [t = (a, b, c, d, e, f)] is of the following form:
 {[     a b 0
        c d 0
        e f 1 ]} *)

  val transform :
    'a t -> (float * float * float * float * float * float) -> unit
  (** [transform c t] apply the given arbitrary transformation
      to the current transformation matrix for canvas [c] *)

  val translate : 'a t -> (float * float) -> unit
  (** [translate c vec] apply the given translation transform
      to the current transformation matrix for canvas [c] *)

  val scale : 'a t -> (float * float) -> unit
  (** [scale c vec] apply the given scale transform
      to the current transformation matrix for canvas [c] *)

  val shear : 'a t -> (float * float) -> unit
  (** [shear c vec] apply the given shear transform
      to the current transformation matrix for canvas [c] *)

  val rotate : 'a t -> float -> unit
  (** [rotate c theta] apply the given rotation transform
      to the current transformation matrix for canvas [c] *)

  (** {1 Style} *)

  val getLineWidth : 'a t -> float
  (** [getLineWidth c] returns the current line width for canvas [c] *)

  val setLineWidth : 'a t -> float -> unit
  (** [setLineWidth c w] sets the current line width for canvas [c] to [w] *)

  val getLineJoin : 'a t -> line_join
  (** [getLineJoin c] returns the current line join type for canvas [c] *)

  val setLineJoin : 'a t -> line_join -> unit
  (** [setLineJoin c j] sets the current line join type for canvas[c] to [j] *)

  val getLineCap : 'a t -> line_cap
  (** [getLineJoin c] returns the current line cap type for canvas [c] *)

  val setLineCap : 'a t -> line_cap -> unit
  (** [setLineJoin c j] sets the current line cap type for canvas[c] to [j] *)

  val getStrokeColor : 'a t -> Color.t
  (** [getStrokeColor c] returns the current stroke color for canvas [c] *)

  val setStrokeColor : 'a t -> Color.t -> unit
  (** [setStrokeColor c col] sets the current stroke color
      for canvas [c] to [col] *)

  val getStrokeStyle : 'a t -> style
  (** [getStrokeStyle c] returns the current stroke style for canvas [c] *)

  val setStrokeStyle : 'a t -> style -> unit
  (** [setStrokeStyle c style] sets the current stroke style for
      canvas [c] to style [style]*)

  val setStrokeGradient : 'a t -> Gradient.t -> unit
  (** [setStrokeGradient c grad] sets the current stroke style for
      canvas [c] to the gradient [grad] *)

  val getFillColor : 'a t -> Color.t
  (** [getFillColor c] returns the current fill color for canvas [c] *)

  val setFillColor : 'a t -> Color.t -> unit
  (** [setFillColor c col] sets the current fill color
      for canvas [c] to [col] *)

  val getFillStyle : 'a t -> style
  (** [getFillStyle c] return the current fill style for canvas [c] *)

  val setFillStyle : 'a t -> style -> unit
  (** [setFillStyle c style] sets the current fill style for
      canvas [c] to style [style]*)

  val setFillGradient : 'a t -> Gradient.t -> unit
  (** [setFillGradient c grad] sets the current fill style for
      canvas [c] to the gradient [grad] *)

  val getGlobalAlpha : 'a t -> float
  (** [getGlobalAlpha c] returns the current global alpha for canvas [c] *)

  val setGlobalAlpha : 'a t -> float -> unit
  (** [setGlobalAlpha c a] sets the global alpha value for
      canvas[c] to [a] *)

  val getGlobalCompositeOperation : 'a t -> composite_op
  (** [getGlobalCompositeOperation c] returns the global composite or blending
      operation for canvas[c] *)

  val setGlobalCompositeOperation : 'a t -> composite_op -> unit
  (** [setGlobalCompositeOperation c o] sets the global composite or blending
      operation for canvas[c] to [o] *)

  val setFont :
    'a t -> string -> size:Font.size ->
    slant:Font.slant -> weight:Font.weight -> unit
  (** [setFont c family ~size ~slant ~weight] sets the current font for
      canvas [c] to the one specified by the given [family], [size],
      [slant] and [weight] *)

  val setFont_ :
    'a t -> string -> Font.size -> Font.slant -> Font.weight -> unit
  (** [setFont_ c family size slant weight] is a labelless equivalent of
      [setFont c family ~size ~slant ~weight]  *)

  (** {1 Path} *)

  val clearPath : 'a t -> unit
  (** [clearPath c] resets the path for canvas [c] *)

  val closePath : 'a t -> unit
  (** [closePath c] closes the current subpath for canvas [c], i.e. adds a
      line from the last point in the current subpath to the first point,
      and marks the subpath as closed. Does nothing if the subpath is empty
      or has a single point, or if the subpath is already closed. *)

  val moveTo : 'a t -> (float * float) -> unit
  (** [moveTo c p] starts a new subpath in canvas [c] containing the
      single point [p]. If the current subpath is empty, its first
      point is set to this point, instead of creating a new subpath.
      Likewise, if the current subpath has a single point, it is
      simply replaced by the given point. *)

  val lineTo : 'a t -> (float * float) -> unit
  (** [lineTo c p] adds the point [p] to the current subpath of canvas [c].
      If the current subpath is empty, this behaves just like [moveTo c ~p]. *)

  val arc :
    'a t -> center:(float * float) -> radius:float ->
    theta1:float -> theta2:float -> ccw:bool -> unit
  (** [arc c ~center ~radius ~theta1 ~theta2 ~ccw] adds an arc of the given
      [radius], centered at [center], between angle [theta1] to [theta2]
      to the current subpath of canvas [c]. If [ccw] is true, the arc will
      be drawn counterclockwise. Note that the last point in the subpath
      (if such point exists) will be connected to the first point of the
      arc by a straight line. *)

  val arc_ :
    'a t -> (float * float) -> float -> float -> float -> bool -> unit
  (** [arc_ c center radius theta1 theta2 ccw] is a labelless equivalent of
      [arc c ~center ~radius ~theta1 ~theta2 ~ccw]  *)

  val arcTo :
    'a t -> p1:(float * float) -> p2:(float * float) -> radius:float -> unit
  (** [arcTo c ~p1 ~p2 ~radius] adds an arc of the given [radius]
      using the control points [p1] and [p2] to the current
      subpath of canvas [c]. If the current subpath is empty,
      this behaves as if [moveTo c ~p:p1] was called. *)

  val arcTo_ : 'a t -> (float * float) -> (float * float) -> float -> unit
  (** [arcTo_ c p1 p2 radius] is a labelless equivalent of
      [arcTo c ~p1 ~p2 ~radius]  *)

  val quadraticCurveTo :
    'a t -> cp:(float * float) -> p:(float * float) -> unit
  (** [quadraticCurveTo c ~cp ~p] adds a quadratic Bezier curve
      using the control point [cp] and the end point [p]
      to the current subpath of canvas [c] *)

  val quadraticCurveTo_ :
    'a t -> (float * float) -> (float * float) -> unit
  (** [quadraticCurveTo_ c cp p] is a labelless equivalent of
      [quadraticCurveTo c ~cp ~p]  *)

  val bezierCurveTo :
    'a t -> cp1:(float * float) -> cp2:(float * float) ->
    p:(float * float) -> unit
  (** [bezierCurve c ~cp1 ~cp2 ~p] adds a cubic Bezier curve using
      the control points [cp1] and [cp2] and the end point [p]
      to the current subpath of canvas [c] *)

  val bezierCurveTo_ :
    'a t -> (float * float) -> (float * float) -> (float * float) -> unit
  (** [bezierCurve_ c cp1 cp2 p] is a labelless equivalent of
      [bezierCurve c ~cp1 ~cp2 ~p]  *)

  val rect : 'a t -> pos:(float * float) -> size:(float * float) -> unit
  (** [rect c ~pos ~size] adds the rectangle specified by [pos]
      and [size]) to the current subpath of canvas [c] *)

  val rect_ : 'a t -> (float * float) -> (float * float) -> unit
  (** [rect_ c pos size] is a labelless equivalent of
      [rect c ~pos ~size]  *)

  val ellipse :
    'a t -> center:(float * float) -> radius:(float * float) ->
    rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
  (** [ellipse c ~center ~radius ~rotation ~theta1 ~theta2] adds an ellipse
      with the given parameters to the current subpath of canvas [c] *)

  val ellipse_ :
    'a t -> (float * float) -> (float * float) ->
    float -> float -> float -> bool -> unit
  (** [ellipse_ c center radius rotation theta1 theta2]
      is a labelless equivalent of
      [ellipse c ~center ~radius ~rotation ~theta1 ~theta2]  *)

  (** {1 Path stroking and filling} *)

  val fill : 'a t -> nonzero:bool -> unit
  (** [fill c ~nonzero] fills the current subpath of canvas [c]
      using the current fill color and the specified fill rule *)

  val fill_ : 'a t -> bool -> unit
  (** [fill_ c nonzero] is a labelless equivalent of
      [fill c ~nonzero]  *)

  val stroke : 'a t -> unit
  (** [stroke c] draws the outline of the current subpath of
      canvas [c] using the current stroke color and line width *)

  (** {1 Immediate drawing} *)

  val fillRect : 'a t -> pos:(float * float) -> size:(float * float) -> unit
  (** [fillRect c ~pos ~size] immediatly fills the rectangle specified by
      [pos] and [size] to the canvas [c] using the current fill color *)

  val fillRect_ : 'a t -> (float * float) -> (float * float) -> unit
  (** [fillRect_ c pos size] is a labelless equivalent of
      [fillRect c ~pos ~size]  *)

  val strokeRect : 'a t -> pos:(float * float) -> size:(float * float) -> unit
  (** [strokeRect c ~pos ~size] immediatly draws the outline of
      the rectangle specified by [pos] and [size] to the canvas
      [c] using the current stroke color and line width *)

  val strokeRect_ : 'a t -> (float * float) -> (float * float) -> unit
  (** [strokeRect_ c pos size] is a labelless equivalent of
      [strokeRect c ~pos ~size]  *)

  val fillText : 'a t -> string -> (float * float) -> unit
  (** [fillText c text pos] immediatly draws the text [text] at
      position [pos] on the canvas [c] using the current fill color *)

  val strokeText : 'a t -> string -> (float * float) -> unit
  (** [strokeText c text pos] immediatly draws the outline of text [text]
      at position [pos] on the canvas [c] using the current stroke color
      and line width *)

  val blit :
    dst:'a t -> dpos:(int * int) ->
    src:'b t -> spos:(int * int) -> size:(int * int) -> unit
  (** [blit ~dst ~dpos ~src ~spos ~size] copies the area specified by [spos]
      and [size] from canvas [src] to canvas [dst] at position [dpos] *)

  val blit_ :
    'a t -> (int * int) -> 'b t -> (int * int) -> (int * int) -> unit
  (** [blit_ dst dpos src spos size] is a labelless equivalent of
      [blit ~dst ~dpos ~src ~spos ~size]  *)

  (** {1 Direct pixel access} *)

  (** Warning: these functions (especially the per-pixel functions) can
      be slow and are not meant for updating the contents of a canvas in
      real-time. Better use them on offscreen canvas during loading phases. *)

  val getPixel : 'a t -> (int * int) -> Color.t
  (** [getPixel c pos] returns the color of the pixel
      at position [pos] in canvas [c] *)

  val setPixel : 'a t -> (int * int) -> Color.t -> unit
  (** [setPixel c pos col] sets the color of the pixel
      at position [pos] in canvas [c] to color [col] *)

  val getImageData : 'a t -> pos:(int * int) -> size:(int * int) -> ImageData.t
  (** [getImageData c ~pos ~size] returns a copy of the pixel
      data at position [pos] of size [size] in canvas [c] *)

  val getImageData_ : 'a t -> (int * int) -> (int * int) -> ImageData.t
  (** [getImageData_ c pos size] is a labelless equivalent of
      [getImageData c ~pos ~size]  *)

  val setImageData :
    'a t -> dpos:(int * int) -> ImageData.t ->
    spos:(int * int) -> size:(int * int) -> unit
  (** [setImageData c ~dpos id ~spos ~size] overwrite the pixels
      at position [dpos] in canvas [c] with the provided pixel data
      starting at position [spos] and of size [size] *)

  val setImageData_ :
    'a t -> (int * int) -> ImageData.t -> (int * int) -> (int * int) -> unit
  (** [setImageData_ c dpos id spos size] is a labelless equivalent of
      [setImageData c ~dpos id ~spos ~size]  *)

  val exportPNG : 'a t -> string -> unit
  (** [exportPNG c filename] saves the contents of canvas [c]
      to a file with name [filename] *)

  val importPNG : 'a t -> pos:(int * int) -> string -> unit
  (** [importPNG c ~pos filename] loads the file [filename] into
      canvas [c] at position [pos] *)

end




module Event : sig
(** Event descriptions *)

(** All event descriptions contain the [canvas] on which the event
    occured and the [timestamp] indicating when the event occured.  *)

  type timestamp = Int64.t
  (** Timestamps represent the time in microseconds,
      from an arbitrary starting point *)

  type frame_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
  }
  (** Describes a frame event *)

  type focus_direction =
    | Out
    | In (**)
  (** Focus direction *)

  type canvas_focused_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    focus: focus_direction; (** Whether the focus was taken or lost *)
  }
  (** Describes a canvas focus event *)

  type canvas_resized_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    size: int * int; (** New canvas size *)
  }
  (** Describes a canvas resize event *)

  type canvas_moved_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    position: int * int; (** New canvas position *)
  }
  (** Describes a canvas motion event *)

  type canvas_closed_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
  }
  (** Describes a canvas closure event *)

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
    | KeyPrintScreen (** absent from Mac Keyboards *)
    | KeyScrollLock (** absent from Mac Keyboards *)
    | KeyPause (** absent from Mac Keyboards *)

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
    | KeyBackslashPipe (** replaced by KeyNonUSNumberTilde on ISO KB *)

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
    | KeyNonUSNumberTilde (** extra key left of Return on ISO KB, although
                              generally mapped to KeyBackslashPipe instead *)
    | KeyReturn

    (* Alphanumeric, fourth row *)
    | KeyLShift
    | KeyNonUSBackslashPipe (** extra key right of LShift on ISO KB *)
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
    | KeyLMeta (** left Windows / Command key *)
    | KeyLAlt
    | KeySpacebar
    | KeyRAlt
    | KeyRMeta (** right Windows / Command key *)
    | KeyMenu
    | KeyRControl

    (* Control pad *)
    | KeyInsert (** replaced by a Fn key on Mac (with a different code) *)
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
    | KeyPadNumlockClear (** on Mac, Clear replaces NumLock *)
    | KeyPadEquals (** on Mac keyboards only *)
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
    | KeyPadComma (** specific to Brazilian keyboards *)
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
    | KeyInternational1 (** extra key left of RShift on JIS and Brazilian KB *)
    | KeyInternational2 (** Katakana/Hiragana key right of Space on JIS KB *)
    | KeyInternational3 (** extra key left of Backspace on JIS KB *)
    | KeyInternational4 (** Henkan key right of Space on JIS KB *)
    | KeyInternational5 (** Muhenkan key left of Space on JIS KB *)
    | KeyInternational6 (** Kanma (comma) key right of KP0 on JIS KB *)
    | KeyInternational7 (** Double-Byte/Single-Byte toggle key *)
    | KeyInternational8 (** Undefined *)
    | KeyInternational9 (** Undefined *)
    | KeyLang1 (** Hangul/English toggle key (Korean) *)
    | KeyLang2 (** Hanja conversion key (Korean) *)
    | KeyLang3 (** Katakana key (Japanese) *)
    | KeyLang4 (** Hiragana key (Japanese) *)
    | KeyLand5 (** Zenkaku/Hankaku key (Japanese) *)

    (* Extensions *)
    | KeyHelp
    | KeyMute
    | KeyVolumeUp
    | KeyVolumeDown (**)
  (** A physical keyboard key, assuming an ideal "extended" QWERTY keyboard
      that synthetizes various layouts, including ANSI, ISO and JIS.
      Note that the symbol on the key may be different from the symbolic
      key name: this allows to refer to keys by their physical location,
      which can be useful for instance in games. *)

  type flags = {
    flag_shift : bool;
    flag_alt : bool;
    flag_control : bool;
    flag_meta : bool;
    flag_capslock : bool;
    flag_numlock : bool;
    flag_dead : bool;
  }
  (** The state of various keyboard flags *)

  type state =
    | Up
    | Down (**)
  (** A keyboard key or mouse button state *)

  type key_action_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    key: key;      (** Physical key that was pressed/released *)
    char: Uchar.t; (** Equivalent Unicode character in the current layout *)
    flags: flags;  (** State of various modifier keys when the event occured *)
    state: state;  (** Whether the key was pressed or released *)
  }
  (** Describes a keyboard event *)

  type button =
    | ButtonNone
    | ButtonLeft
    | ButtonMiddle
    | ButtonRight
    | ButtonWheelUp
    | ButtonWheelDown (**)
    (** A mouse button *)

  type button_action_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    position: int * int; (** Cursor position when the event occured *)
    button: button;      (** Button that was pressed/released *)
    state: state;        (** Whether the button was pressed or released *)
  }
  (** Describes a mouse button event *)

  type mouse_move_event = {
    canvas: [`Onscreen] Canvas.t;
    timestamp: timestamp;
    position: int * int; (** Cursor position when the event occured *)
  }
  (** Describes a mouse motion event *)

  type t =
    | Frame of frame_event
    (** Occurs 60 times per second. If you actually draw something to the
        canvas when handling this event, you should return true to let the
        library know that the contents of the canvas changed, so that the
        display can be refreshed. *)
    | CanvasFocused of canvas_focused_event
    (** Occurs when the canvas becomes active or inactive,
        as a result of being clicked or tabbed-into *)
    | CanvasResized of canvas_resized_event
    (** Occurs when the canvas is resized by a user action *)
    | CanvasMoved of canvas_moved_event
    (** Occurs when the canvas is moved by a user action *)
    | CanvasClosed of canvas_closed_event
    (** Occurs when the user clicks the close button *)
    | KeyAction of key_action_event
    (** Occurs when the user presses a key on the keyboard. The event
        description contains both the physical key (of type {!Event.key}
        and the Unicode character corresponding to that key (if any),
        according to the current keyboard layout. For instance, pressing
        the "A" key on an AZERTY keyboard will yield a physical key
        [Event.KeyQ] and the Unicode code point for character "A". *)
    | ButtonAction of button_action_event
    (** Occurs when the user presses a mouse button *)
    | MouseMove of mouse_move_event
    (** Occurs when the user moves the mouse cursor *)

    val int_of_key : key -> int
  (** [int_of_key k] returns a platform-independent integer representation
      of key [k]. This integer corresponds to the key code as defined
      by the USB standard for keybords.
      @see <https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf>
      https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf *)

    val key_of_int : int -> key
  (** [key_of_int i] returns the key corresponding to the
      platform-independent integer [i]. *)

end

module Backend : sig
(** Initialization and event loop control *)

  type _ backend_type =
    | Canvas : [`JS] backend_type
    | GDI : [`Win32] backend_type
    | Quartz : [`OSX] backend_type
    | X11 : [<`Unix | `OSX | `Win32] backend_type
    | Wayland : [`Unix] backend_type (**)
  (** The different kind of supported backends *)

  type options = {
    js_backends: [`JS] backend_type list;
    win32_backends: [`Win32] backend_type list;
    osx_backends: [`OSX] backend_type list;
    unix_backends: [`Unix] backend_type list;
  } (** List of available backends per OS *)

  val default_options : options
  (** The default options to use for backend initialization *)

  val init : options -> unit
  (** [init o] initializes the backend with the specified options *)

  val run : (Event.t-> bool) -> (unit -> 'a) -> 'b
  (** [run h k] executes the backend event loop, calling [h] when an event
      occur, and calling [k] when the event loop terminates. The call to
      [run] MUST be the last instruction of your program (to avoid different
      behaviors between the native and javascript backends). If you need
      to perform additional stuff when the program terminates, DO use
      the [k] function: it is meant for that. Note that calling [run]
      within the event handler function [h] will just be ignored
      (but this should not be done). However, [run] may be called
      in the [k] function, if needed. *)

  val stop : unit -> unit
  (** [stop ()] stops the currently running event loop, if any.
      This should be called within an event handler function. *)

  val getCanvas : int -> 'a Canvas.t option
  (** [getCanvas i] returns the canvas that has id [i], if it exists *)

end
