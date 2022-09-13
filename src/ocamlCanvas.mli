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

    Before using any function in the library (and assuming the `OCamlCanvas.V1`
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
    open OCamlCanvas.V1

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

module V1 : sig

  module Transform : sig
  (** Transform manipulation functions *)

    type t = {
      a : float; (** scaling/flipping, rotation *)
      b : float; (** shearing, rotation *)
      c : float; (** scaling/flipping, rotation *)
      d : float; (** shearing, rotation *)
      e : float; (** translation *)
      f : float; (** translation *)
    }
    (** A type to represent transformation matrices of the form:
 {[     a b 0
        c d 0
        e f 1 ]} *)

    val id : t
    (** Identity transformation *)

    val create : (float * float * float * float * float * float) -> t
    (** [create t] creates a transformation given
        the matrix [t = (a, b, c, d, e, f)] *)

    val mul : t -> t -> t
    (** [mul t1 t2] multiplies [t1] by [t2] *)

    val translate : t -> (float * float) -> t
    (** [translate t v] composes [t] by a translation of vector [v] *)

    val scale : t -> (float * float) -> t
    (** [scale t v] composes [t] by a scaling of vector [v] *)

    val shear : t -> (float * float) -> t
    (** [shear t v] composes [t] by a shearing of vector [v] *)

    val rotate : t -> float -> t
    (** [rotate t a] composes [t] by a rotation
        of angle [a] around the origin *)

    val inverse : t -> t
    (** [inverse t] returns the inverse of [t] *)

  end

  module Point : sig
  (** Point manipulation functions *)

    type t = (float * float)
    (** A point is a pair of floats of the form (x, y)  *)

    val translate : t -> by:(float * float) -> t
    (** [translate p ~by] translates point [p] by the vector [by] *)

    val rotate : t -> around:t -> theta:float -> t
    (** [rotate p ~around ~theta] rotates point [p] around
        a central point [around] by an angle [theta] *)

    val transform : t -> Transform.t -> t
    (** [transform p t] transforms point [p] by the given transform [t] *)

    val barycenter : float -> t -> float -> t -> t
    (** [barycenter a p1 b p2] compute the barycenter
        of ([a], [p1]) and ([b], [p2]) *)

    val distance : t -> t -> float
    (** [distance p1 p2] computes the distance between [p1] and [p2] *)

  end

  module Color : sig
  (** Color description and manipulation functions *)

    type t
    (** Abstract type for colors *)

    val of_rgb : int -> int -> int -> t
    (** [of_rgb r g b] creates a color from its [r], [g] and [b] components *)

    val to_rgb : t -> int * int * int
    (** [to_rgb c] converts a color to its [r], [g] and [b] components *)

    val of_argb : int -> int -> int -> int -> t
    (** [of_argb a r g b] creates a color from
        its [a], [r], [g], [b] components *)

    val to_argb : t -> int * int * int * int
    (** [to_argb c] converts a color to its [a], [r], [g] and [b] components *)

    val of_int : int -> t
    (** [of_int i] creates a color from its 24-bit integer representation [i] ;
        this representation does not include the alpha component *)

    val to_int : t -> int
    (** [to_int c] converts a color [c] to its 24-bit integer representation ;
        this representation does not include the alpha component *)

    val of_int32 : Int32.t -> t
    (** [of_int i] creates a color from its 32-bit integer representation [i] *)

    val to_int32 : t -> Int32.t
    (** [to_int c] converts a color [c] to its 32-bit integer representation *)

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
    (** [define_color s c] add a color [c]
        named [n] to the internal color map *)

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

  end

  module ImageData : sig
  (** Image data manipulation functions *)

    type t =
      (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t
    (** Image data are big arrays of dimension 3 (height, width, component),
        where the components are in BGRA order *)

    val create : (int * int) -> t
    (** [create size] creates an empty image data of the given [size] *)

    val createFromPNG : string -> t
    (** [createFromPNG filename] creates an image data
        with the contents of PNG file [filename] *)

    val getSize : t -> (int * int)
    (** [getSize id] returns the size of image data [id] *)

    val fill : t -> Color.t -> unit
    (** [fill id c] fills the image data [id] with the given color [c] *)

    val sub : t -> pos:(int * int) -> size:(int * int) -> t
    (** [sub c ~pos ~size] returns a copy of the pixel data
        at position [pos] of size [size] in image data [id] *)

    val blit :
      dst:t -> dpos:(int * int) ->
      src:t -> spos:(int * int) -> size:(int * int) -> unit
    (** [blit ~dst ~dpos ~src ~spos ~size] copies the area
        specified by [spos] and [size] from image data [src]
        to imdate data [dst] at position [dpos] *)

    val getPixel : t -> (int * int) -> Color.t
    (** [getPixel id pos] returns the color of the pixel
        at position [pos] in image data [id] *)

    val putPixel : t -> (int * int) -> Color.t -> unit
    (** [putPixel id pos c] sets the color of the pixel
        at position [pos] in image data [id] to color [c] *)

    val importPNG : t -> pos:(int * int) -> string -> unit
    (** [importPNG id ~pos filename] loads the file [filename]
        into image data [id] at position [pos] *)

    val exportPNG : t -> string -> unit
    (** [exportPNG id filename] saves the contents of image
        data [id] to a file with name [filename] *)

  end

  module Gradient : sig
  (** Gradient manipulation functions *)

    type t
    (** An abstract type representing a gradient *)

    val addColorStop : t -> Color.t -> float -> unit
    (** [addColorStop] gradient color stop) adds a new [color]
        color spot in the gradient object at position [stop] *)

  end

  module Pattern : sig
  (** Pattern manipulation functions *)

    type t
    (** An abstract type representing a pattern *)

    type repeat =
      | NoRepeat
      | RepeatX
      | RepeatY
      | RepeatXY (**)
    (** Pattern repetition *)

  end

  module Path : sig
  (** Path manipulation functions *)

    type t
    (** An abstract type representing a path *)

    val create : unit -> t
    (** [create ()] creates an empty path object. *)

    val moveTo: t -> Point.t -> unit
    (** [moveTo p pos] moves the path [p]'s brush position to [pos]. *)

    val close: t -> unit
    (** [close p] closes the path [p]. *)

    val lineTo: t -> Point.t -> unit
    (** [lineTo p pos] adds a straight line from
        the path [p]'s brush position to [pos]. *)

    val arc :
      t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
    (** [arc p ~center ~radius ~theta1 ~theta2 ~ccw] adds an arc of the
        given [radius], centered at [center], between angle [theta1] to
        [theta2] to the path [p]. If [ccw] is true, the arc will be
        drawn counterclockwise. Note that the last point in the
        subpath (if such point exists) will be connected to the
        first point of the arc by a straight line. *)

    val arcTo :
      t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
    (** [arcTo p ~p1 ~p2 ~radius] adds an arc of the given [radius]
        using the control points [p1] and [p2] to the path [p].
        If the path [p] is empty, this behaves as if
        [moveTo p ~p:p1] was called. *)

    val quadraticCurveTo : t -> cp:Point.t -> p:Point.t -> unit
    (** [quadraticCurveTo path ~cp ~p] adds a quadratic curve from
        [path]'s brush position to [~p] with control point [~cp]. *)

    val bezierCurveTo :
      t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
    (** [bezierCurveTo path ~cp1 ~cp2 ~p] adds a bezier curve from
        [path]'s brush position to [~p] with control points [~cp1]
        and [~cp2]. *)

    val rect : t -> pos:Point.t -> size:(float * float) -> unit
    (** [rect p ~pos ~size] adds the rectangle specified by [pos]
        and [size]) to the path [p] *)

    val ellipse :
      t -> center:Point.t -> radius:(float * float) ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
    (** [ellipse p ~center ~radius ~rotation ~theta1 ~theta2] adds
        an ellipse with the given parameters to the path [p] *)

    val add : t -> t -> unit
    (** [add dst src] adds the path [src] into the path [dst] *)

    val addTransformed : t -> t -> Transform.t -> unit
    (** [addTransformed dst src t] adds the path [src] after
        applying [t] to each of its points into the path [dst] *)

  end

  module Canvas : sig
  (** Canvas manipulation functions *)

    type 'kind t
    (** An abstract type representing a canvas *)

    type line_join =
      | Round
      | Miter
      | Bevel (**)
    (** An enum type for representing line join types *)

    type line_cap =
      | Butt
      | Square
      | RoundCap (**)
    (** An enum type for representing line cap types *)

    type style =
      | Color of Color.t
      | Gradient of Gradient.t
      | Pattern of Pattern.t (**)
    (** A type to represent stroke and fill styles *)

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
      | Luminosity (**)
    (** A type to represent blending and compositing operations *)


    (** {1 Gradient creation functions} *)

    val createLinearGradient :
      'kind t -> pos1:Point.t -> pos2:Point.t -> Gradient.t
    (** [createLinearGradient c ~pos1 ~pos2] creates a new
        linear gradient parallel to the line ([pos1][pos2])
        in window coordinates for canvas [c] *)

    val createRadialGradient:
      'kind t -> center1:Point.t -> rad1:float ->
      center2:Point.t -> rad2:float -> Gradient.t
    (** [createRadialGradient c ~center1 ~rad1 ~center2 ~rad2] creates a new
        radial gradient between the disks with centers [center1] and [center2]
        and radi [rad1] and [rad2] in window coordinates for canvas [c] *)

    val createConicGradient:
      'kind t -> center:Point.t -> angle:float -> Gradient.t
    (** [createConicGradient c ~center ~angle] creates a new conic gradient
        with center [center] and initial angle [angle] for canvas [c] *)


    (** {1 Pattern creation function} *)

    val createPattern :
      'kind t -> ImageData.t -> Pattern.repeat -> Pattern.t
    (** [createPattern img rep] creates a pattern
        of [rep] using [img] as source *)


    (** {1 Comparison functions} *)

    (** In order to ease identification of canvas or building collections
        of canvas, the usual comparison functions are provided.
        These functions simply operate on the canvas' unique ids. *)

    val hash : 'kind t -> int
    (** [hash c] returns a unique integer value for canvas [c], which
        happens to be the same as the unique canvas id [getId c] *)

    val compare : 'kind1 t -> 'kind2 t -> int
    (** [compare c1 c2] is equivalent to [compare (getId c1) (getId c2)] *)

    val equal : 'kind1 t -> 'kind2 t -> bool
    (** [equal c1 c2] is equivalent to [equal (getId c1) (getId c2)] *)

    val (=) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 = c2] is equivalent to [(getId c1) = (getId c2)] *)

    val (<>) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 <> c2] is equivalent to [(getId c1) <> (getId c2)] *)

    val (<) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 < c2] is equivalent to [(getId c1) < (getId c2)] *)

    val (>) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 > c2] is equivalent to [(getId c1) > (getId c2)] *)

    val (<=) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 <= c2] is equivalent to [(getId c1) <= (getId c2)] *)

    val (>=) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 >= c2] is equivalent to [(getId c1) >= (getId c2)] *)

    val (==) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 == c2] test for physical equality of canvas [c1] and [c2] *)

    val (!=) : 'kind1 t -> 'kind2 t -> bool
    (** [c1 != c2] test for physical inequality of canvas [c1] and [c2] *)


    (** {1 Creation} *)

    val createFramed :
      string -> pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
    (** [createFramed title ~pos ~size] creates a canvas in a window
        with title [title] at position [pos] and of size [size] *)

    val createFrameless : pos:(int * int) -> size:(int * int) -> [> `Onscreen] t
    (** [createFrameless ~pos ~size] creates a canvas in an
        undecorated window at position [pos] and of size [size] *)

    val createOffscreen : size:(int * int) -> [> `Offscreen] t
    (** [createOffscreen ~size] creates an offscreen canvas of size [size] *)

    val createOffscreenFromImageData : ImageData.t -> [> `Offscreen] t
    (** [createOffscreenFromImageData id] creates an offscreen
        canvas with the contents of image data [id] *)

    val createOffscreenFromPNG : string -> [> `Offscreen] t
    (** [createOffscreen filename] creates an offscreen
        canvas with the contents of PNG file [filename] *)


    (** {1 Visibility} *)

    val show : [< `Onscreen] t -> unit
    (** [show c] makes the canvas [c] visible on screen.
        Does not apply to offscreen canvas. *)

    val hide : [< `Onscreen] t -> unit
    (** [hide c] makes the canvas [c] invisible.
        Does not apply to offscreen canvas. *)

    val close : [> `Onscreen] t -> unit
    (** [close c] closes the canvas [c], i.e. it permanently removes
        it from the screen and prevents it to receive events ;
        however it can still be used as an offscreen canvas. *)


    (** {1 Configuration} *)

    val getId : 'kind t -> int
    (** [getId c] returns the unique id of canvas [c],
        or 0 if the canvas has been destroyed *)

    val getSize : 'kind t -> (int * int)
    (** [getSize c] returns the size of canvas [c] *)

    val setSize : 'kind t -> (int * int) -> unit
    (** [setSize c size] sets the size of canvas [c] *)

    val getPosition : [< `Onscreen] t -> (int * int)
    (** [getPosition c] returns the position of canvas [c] *)

    val setPosition : [< `Onscreen] t -> (int * int) -> unit
    (** [setPosition c pos] sets the position of canvas [c] *)


    (** {1 State} *)

    val save : 'kind t -> unit
    (** [save c] pushes the current state of canvas [c] onto the state stack *)

    val restore : 'kind t -> unit
    (** [restore c] pops the current state of canvas [c] from the state stack *)


    (** {1 Transformations} *)

    val setTransform : 'kind t -> Transform.t -> unit
    (** [setTransform c t] sets the current transformation matrix of canvas [c].
        The matrix [t = { a, b, c, d, e, f }] is of the following form:
 {[     a b 0
        c d 0
        e f 1 ]} *)

    val transform : 'kind t -> Transform.t -> unit
    (** [transform c t] apply the given arbitrary transformation
        to the current transformation matrix of canvas [c] *)

    val translate : 'kind t -> (float * float) -> unit
    (** [translate c vec] apply the given translation transform
        to the current transformation matrix of canvas [c] *)

    val scale : 'kind t -> (float * float) -> unit
    (** [scale c vec] apply the given scale transform
        to the current transformation matrix of canvas [c] *)

    val shear : 'kind t -> (float * float) -> unit
    (** [shear c vec] apply the given shear transform
        to the current transformation matrix of canvas [c] *)

    val rotate : 'kind t -> float -> unit
    (** [rotate c theta] apply the given rotation transform
        to the current transformation matrix of canvas [c] *)


    (** {1 Style} *)

    val getLineWidth : 'kind t -> float
    (** [getLineWidth c] returns the current line width of canvas [c] *)

    val setLineWidth : 'kind t -> float -> unit
    (** [setLineWidth c w] sets the current line width of canvas [c] to [w] *)

    val getLineJoin : 'kind t -> line_join
    (** [getLineJoin c] returns the current line join type of canvas [c] *)

    val setLineJoin : 'kind t -> line_join -> unit
    (** [setLineJoin c j] sets the current line join type of canvas[c] to [j] *)

    val getLineCap : 'kind t -> line_cap
    (** [getLineJoin c] returns the current line cap type of canvas [c] *)

    val setLineCap : 'kind t -> line_cap -> unit
    (** [setLineJoin c j] sets the current line cap type of canvas[c] to [j] *)

    val getMiterLimit : 'kind t -> float
    (** [getMiterLimit c] returns the current miter limit of canvas [c] *)

    val setMiterLimit : 'kind t -> float -> unit
    (** [getMiterLimit c m] sets the current miter limit of canvas [c] to [m] *)

    val getLineDashOffset : 'kind t -> float
    (** [getLineDashOffset c] returns the current line offset of [c] *)

    val setLineDashOffset : 'kind t -> float -> unit
    (** [setLineDashOffset c t] sets the current line offset of [c] to [t] *)

    val getLineDash : 'kind t -> float array
    (** [getLineDash c t] returns the current line dash pattern of [c] *)

    val setLineDash : 'kind t -> float array -> unit
    (** [setLineDash c t] sets the current line dash pattern of [c] to [t] *)

    val getStrokeColor : 'kind t -> Color.t
    (** [getStrokeColor c] returns the current stroke color of canvas [c] *)

    val setStrokeColor : 'kind t -> Color.t -> unit
    (** [setStrokeColor c col] sets the current
        stroke color of canvas [c] to [col] *)

    val setStrokeGradient : 'kind t -> Gradient.t -> unit
    (** [setStrokeGradient c grad] sets the current stroke
        style of canvas [c] to the gradient [grad] *)

    val setStrokePattern : 'kind t -> Pattern.t -> unit
    (** [setStrokePattern c pat] sets the current stroke
        style of canvas [c] to the pattern [pat] *)

    val getStrokeStyle : 'kind t -> style
    (** [getStrokeStyle c] returns the current stroke style of canvas [c] *)

    val setStrokeStyle : 'kind t -> style -> unit
    (** [setStrokeStyle c style] sets the current stroke
        style of canvas [c] to style [style] *)

    val getFillColor : 'kind t -> Color.t
    (** [getFillColor c] returns the current fill color of canvas [c] *)

    val setFillColor : 'kind t -> Color.t -> unit
    (** [setFillColor c col] sets the current
        fill color of canvas [c] to [col] *)

    val setFillGradient : 'kind t -> Gradient.t -> unit
    (** [setFillGradient c grad] sets the current fill
        style of canvas [c] to the gradient [grad] *)

    val setFillPattern : 'kind t -> Pattern.t -> unit
    (** [setFillPattern c pat] sets the current fill
        style of canvas [c] to the pattern [pat] *)

    val getFillStyle : 'kind t -> style
    (** [getFillStyle c] return the current fill style of canvas [c] *)

    val setFillStyle : 'kind t -> style -> unit
    (** [setFillStyle c style] sets the current
        fill style of canvas [c] to style [style] *)

    val getGlobalAlpha : 'kind t -> float
    (** [getGlobalAlpha c] returns the current global alpha of canvas [c] *)

    val setGlobalAlpha : 'kind t -> float -> unit
    (** [setGlobalAlpha c a] sets the global alpha value of canvas[c] to [a] *)

    val getGlobalCompositeOperation : 'kind t -> composite_op
    (** [getGlobalCompositeOperation c] returns the global
        composite or blending operation of canvas[c] *)

    val setGlobalCompositeOperation : 'kind t -> composite_op -> unit
    (** [setGlobalCompositeOperation c o] sets the global
        composite or blending operation of canvas[c] to [o] *)

    val getShadowColor :
      'kind t -> Color.t
    (** [setShadowColor c] returns the canvas [c]'s shadow color *)

    val setShadowColor :
      'kind t -> Color.t -> unit
    (** [setShadowColor c col] sets the canvas [c]'s shadow color to [col] *)

    val getShadowBlur :
      'kind t -> float
    (** [setShadowBlur c] returns the shadow blur radius of canvas [c]  *)

    val setShadowBlur :
      'kind t -> float -> unit
    (** [setShadowBlur c b] sets the shadow blur radius of canvas [c] to [b] *)

    val getShadowOffset :
      'kind t -> (float * float)
    (** [setShadowOffset c] returns the offset of the shadows drawn in [c] *)

    val setShadowOffset :
      'kind t -> (float * float) -> unit
    (** [setShadowOffset c o] sets the offset
        of the shadows drawn in [c] to [o] *)

    val setFont :
      'kind t -> string -> size:Font.size ->
      slant:Font.slant -> weight:Font.weight -> unit
    (** [setFont c family ~size ~slant ~weight] sets the current
        font of canvas [c] to the one specified by the given
        [family], [size], [slant] and [weight] *)


    (** {1 Path} *)

    val clearPath : 'kind t -> unit
    (** [clearPath c] resets the path of canvas [c] *)

    val closePath : 'kind t -> unit
    (** [closePath c] closes the current subpath of canvas [c], i.e. adds a
        line from the last point in the current subpath to the first point,
        and marks the subpath as closed. Does nothing if the subpath is empty
        or has a single point, or if the subpath is already closed. *)

    val moveTo : 'kind t -> Point.t -> unit
    (** [moveTo c p] starts a new subpath in canvas [c] containing the
        single point [p]. If the current subpath is empty, its first
        point is set to this point, instead of creating a new subpath.
        Likewise, if the current subpath has a single point, it is
        simply replaced by the given point. *)

    val lineTo : 'kind t -> Point.t -> unit
    (** [lineTo c p] adds the point [p] to the current subpath of canvas [c].
        If the current subpath is empty, this behaves just like [moveTo c ~p].*)

    val arc :
      'kind t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
    (** [arc c ~center ~radius ~theta1 ~theta2 ~ccw] adds an arc of the given
        [radius], centered at [center], between angle [theta1] to [theta2]
        to the current subpath of canvas [c]. If [ccw] is true, the arc will
        be drawn counterclockwise. Note that the last point in the subpath
        (if such point exists) will be connected to the first point of the
        arc by a straight line. *)

    val arcTo :
      'kind t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
    (** [arcTo c ~p1 ~p2 ~radius] adds an arc of the given [radius]
        using the control points [p1] and [p2] to the current
        subpath of canvas [c]. If the current subpath is empty,
        this behaves as if [moveTo c ~p:p1] was called. *)

    val quadraticCurveTo :
      'kind t -> cp:Point.t -> p:Point.t -> unit
    (** [quadraticCurveTo c ~cp ~p] adds a quadratic Bezier curve
        using the control point [cp] and the end point [p]
        to the current subpath of canvas [c] *)

    val bezierCurveTo :
      'kind t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
    (** [bezierCurve c ~cp1 ~cp2 ~p] adds a cubic Bezier curve using
        the control points [cp1] and [cp2] and the end point [p]
        to the current subpath of canvas [c] *)

    val rect : 'kind t -> pos:Point.t -> size:(float * float) -> unit
    (** [rect c ~pos ~size] adds the rectangle specified by [pos]
        and [size]) to the current subpath of canvas [c] *)

    val ellipse :
      'kind t -> center:Point.t -> radius:(float * float) ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
    (** [ellipse c ~center ~radius ~rotation ~theta1 ~theta2] adds an ellipse
        with the given parameters to the current subpath of canvas [c] *)


    (** {1 Path stroking and filling} *)

    val fill : 'kind t -> nonzero:bool -> unit
    (** [fill c ~nonzero] fills the current subpath of canvas [c]
        using the current fill color and the specified fill rule *)

    val fillPath : 'kind t -> Path.t -> nonzero:bool -> unit
    (** [fillPath c p ~nonzero] fills the path [p] on canvas [c]
        using the current fill style and the specified fill rule *)

    val stroke : 'kind t -> unit
    (** [stroke c] draws the outline of the current subpath of
        canvas [c] using the current stroke color and line width *)

    val strokePath : 'kind t -> Path.t -> unit
    (** [strokePath c p] draws the outline of the path [p] on
        canvas [c] using the current stroke style and line width *)

    val clip : 'kind t -> nonzero:bool -> unit
    (** [clipPath c p ~nonzero] intersects the current subpath of [c]
        on canvas [c]'s clip region using the specified fill rule *)

    val clipPath : 'kind t -> Path.t -> nonzero:bool -> unit
    (** [clipPath c p ~nonzero] intersects the filled path [p] on
        canvas [c]'s clip region using the specified fill rule *)


    (** {1 Immediate drawing} *)

    val fillRect : 'kind t -> pos:Point.t -> size:(float * float) -> unit
    (** [fillRect c ~pos ~size] immediatly fills the rectangle specified by
        [pos] and [size] to the canvas [c] using the current fill color *)

    val strokeRect : 'kind t -> pos:Point.t -> size:(float * float) -> unit
    (** [strokeRect c ~pos ~size] immediatly draws the outline of
        the rectangle specified by [pos] and [size] to the canvas
        [c] using the current stroke color and line width *)

    val fillText : 'kind t -> string -> Point.t -> unit
    (** [fillText c text pos] immediatly draws the text [text] at
        position [pos] on the canvas [c] using the current fill color *)

    val strokeText : 'kind t -> string -> Point.t -> unit
    (** [strokeText c text pos] immediatly draws the outline of text [text]
        at position [pos] on the canvas [c] using the current stroke color
        and line width *)

    val blit :
      dst:'kind1 t -> dpos:(int * int) ->
      src:'kind2 t -> spos:(int * int) -> size:(int * int) -> unit
    (** [blit ~dst ~dpos ~src ~spos ~size] copies the area specified by [spos]
        and [size] from canvas [src] to canvas [dst] at position [dpos] *)


    (** {1 Direct pixel access} *)

    (** Warning: these functions (especially the per-pixel functions) can
        be slow and are not meant for updating the contents of a canvas in
        real-time. Better use them on offscreen canvas during loading phases. *)

    val getPixel : 'kind t -> (int * int) -> Color.t
    (** [getPixel c pos] returns the color of the
        pixel at position [pos] in canvas [c] *)

    val putPixel : 'kind t -> (int * int) -> Color.t -> unit
    (** [putPixel c pos col] sets the color of the pixel
        at position [pos] in canvas [c] to color [col] *)

    val getImageData :
      'kind t -> pos:(int * int) -> size:(int * int) -> ImageData.t
    (** [getImageData c ~pos ~size] returns a copy of the pixel
        data at position [pos] of size [size] in canvas [c] *)

    val putImageData :
      'kind t -> dpos:(int * int) -> ImageData.t ->
      spos:(int * int) -> size:(int * int) -> unit
    (** [setImageData c ~dpos id ~spos ~size] overwrite the pixels
        at position [dpos] in canvas [c] with the provided pixel
        data starting at position [spos] and of size [size] *)

    val exportPNG : 'kind t -> string -> unit
    (** [exportPNG c filename] saves the contents of
        canvas [c] to a file with name [filename] *)

    val importPNG : 'kind t -> pos:(int * int) -> string -> unit
    (** [importPNG c ~pos filename] loads the file
        [filename] into canvas [c] at position [pos] *)

  end

  module Event : sig
  (** Event descriptions *)

    (** All event descriptions contain the [canvas] on which the event
        occured and the [timestamp] indicating when the event occured. *)

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
      | KeyInternational1 (** extra key left of RShift on JIS and Brazilian KB*)
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
      flags: flags;  (** State of various modifier keys when the event occured*)
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
    }
    (** List of available backends per OS *)

    val default_options : options
    (** The default options to use for backend initialization *)

    val init : options -> unit
    (** [init o] initializes the backend with the specified options *)

    val run :
      ('state -> Event.t -> 'state * bool) ->
      ('state -> 'dummy1) -> 'state -> 'dummy2
    (** [run h k s] executes the backend event loop, calling the event handler
        function [h] when an event occurs, and calling the continuation
        function [k] when the event loop terminates, passing the given
        state [s] to these functions as needed. The event handler function
        should return the new state, and a boolean indicating whether the
        event was actually handled. The backend uses this information to
        determine whether further actions need to be performed (eg. presenting
        the contents of a canvas when returning true in a frame event), or
        whether a default action should be performed (as of now, no default
        action is performed, but this may change in the future). Simply put,
        just return true in events you actually handle, and false otherwise.
        Note that the call to [run] MUST be the last instruction in your
        program (to avoid different behaviors between the native and javascript
        backends). If you need to perform additional stuff when the program
        terminates, yout MUST use the [k] function: it is meant for that.
        Note that calling [run] from the event handler function [h] will
        just be ignored (though this should not be done). However, [run]
        may be called from the [k] function, if needed. *)

    val stop : unit -> unit
    (** [stop ()] requests termination of the currently running event
        loop, if any. It should be called from an event handler function.
        Actual termination of the event loop will occur at the end of
        the current iteration of the event loop, so after calling [stop]
        an event handler should proceed normally until it returns. *)

    val getCanvas : int -> 'a Canvas.t option
    (** [getCanvas i] returns the canvas that has id [i], if it exists *)

  end

end
