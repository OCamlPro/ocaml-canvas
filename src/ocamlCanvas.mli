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

    Before using any function in the library (and assuming the {!OcamlCanvas.V1}
    module has been opened), the user should call {!Backend.init} so that
    the library makes any internal initialization it needs for the current
    backend.

    Once the backend is initialized, one can create Canvas objects using
    the {!Canvas.createOnscreen} and {!Canvas.createOffscreen} functions.
    The first one creates canvases contained in regular windows (which are
    simulated in the Javascript backend), while the second one creates
    canvases that are not rendered on screen, which can be useful to save
    complex images that can then simply be copied to a visible canvas.
    Onscreen canvases are hidden by default, and {!Canvas.show}
    should be called on them to make them visible.

    Drawing on a canvas can be perfomed using various drawing primitives,
    the most ubiquitous being {!Canvas.clearPath}, {!Canvas.moveTo},
    {!Canvas.lineTo}, {!Canvas.arc}, {!Canvas.bezierCurveTo}, {!Canvas.fill}
    and {!Canvas.stroke}. These functions allow to build a path step by step
    and either fill it completely or draw its outline. It is also possible
    to directly render some text with the {!Canvas.fillText} and
    {!Canvas.strokeText} functions.

    The canvas drawing style can be customized using functions
    such as {!Canvas.setFillColor}, {!Canvas.setStrokeColor} or
    {!Canvas.setLineWidth}. The font used to draw text can be specified
    with the {!Canvas.setFont} function. It is also possible to apply
    various transformations to a canvas, such as translation, rotation and
    scaling, with the functions {!Canvas.transform}, {!Canvas.translate},
    {!Canvas.scale}, {!Canvas.rotate} and {!Canvas.shear}. All these
    styling elements can be saved and restored to/from a state stack
    using the functions {!Canvas.save} and {!Canvas.restore}.

    Once the canvases are ready, we may start handling events for these
    canvases. To do so, we use the {!Backend.run} function, which runs an
    event loop. This function MUST be the last instruction of the program.
    It takes a single argument, which is a function to be executed when the
    event loop has finished running. The event loop may be stopped by calling
    {!Backend.stop} from any update function.

    Each event reports at least the canvas on which it occured, and its
    timestamp. It may also report mouse coordinates for mouse events,
    or keyboard status for keyboard events.

    {1 An actual example}

    The following program creates a windowed canvas with an orange background,
    a cyan border, and the "Hello world !" text drawn rotated in the middle.
    The user may press the "Escape" key or close the window to exit the
    program. It will show the number of frames displayed when quitting.

{[
    open OcamlCanvas.V1

    let () =

      Backend.init ();

      let c = Canvas.createOnscreen ~title:"Hello world"
                ~pos:(300, 200) ~size:(300, 200) () in

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
      Canvas.rotate c (-. Const.pi_8);
      Canvas.fillText c "Hello world !" (-130.0, 20.0);
      Canvas.restore c;

      Canvas.show c;

      let e1 =
        React.E.map (fun { Event.canvas = _; timestamp = _; data = () } ->
            Backend.stop ()
          ) Event.close
      in

      let e2 =
        React.E.map (fun { Event.canvas = _; timestamp = _;
                           data = { Event.key; char = _; flags = _ }; _ } ->
            if key = KeyEscape then
              Backend.stop ()
          ) Event.key_down
      in

      let e3 =
        React.E.map (fun { Event.canvas = _; timestamp = _;
                           data = { Event.position = (x, y); button } } ->
            Canvas.setFillColor c Color.red;
            Canvas.clearPath c;
            Canvas.arc c ~center:(float_of_int x, float_of_int y)
              ~radius:5.0 ~theta1:0.0 ~theta2:(2.0 * Const.pi) ~ccw:false;
            Canvas.fill c ~nonzero:false
          ) Event.button_down
      in

      let frames = ref 0 in

      let e4 =
        React.E.map (fun { Event.canvas = _; timestamp = _ } ->
            frames := Int64.add !frames Int64.one
          ) Event.frame
      in

      Backend.run (fun () ->
          ignore e1; ignore e2; ignore e3; ignore e4;
          Printf.printf "Displayed %Ld frames. Goodbye !\n" !frames)
]} *)

module V1 : sig
(** The OCaml-Canvas module is versioned. This is version 1.
    It is guaranteed that this interface will always remain compatible
    with existing programs, provided that the modules defined here
    ARE NEVER included in other modules nor opened globally.
    Local opens should be performed very carefully, as new identifiers
    may be introduced in modules and thus shadow any identifier defined
    before the open directive. An effort will be made to avoid introducing
    new identifiers that are of length 3 of less, or starting with a single
    character followed by an underscore. Hence such identifiers should be
    safe to use without risking to be shadowed. *)

  module Exception : sig
  (** OCaml-Canvas' exceptions *)

    exception Not_initialized
    (** Raised when calling a function before {!Backend.init} is called *)

    exception File_not_found of string
    (** Raised by PNG import/export functions to indicate the given file could
        not be found. The string argument recalls the file in question. *)

    exception Read_png_failed of string
    (** Raised by PNG import functions to indicate failure. The string
        argument might provide futher information explaining the failure. *)

    exception Write_png_failed of string
    (** Raised by PNG export functions to indicate failure. The string
        argument might provide futher information explaining the failure. *)

  end

  module Const : sig
  (** Some useful mathematical constants *)

    val pi : float
    (** [pi] is an approximation of pi with double precision *)

    val pi_2 : float
    (** [pi_2] is pi / 2 *)

    val pi_4 : float
    (** [pi_4] is pi / 4 *)

    val pi_8 : float
    (** [pi_8] is pi / 8 *)

    val inv_pi : float
    (** [inv_pi] is 1 / pi *)

    val inv_pi_2 : float
    (** [inv_pi_2] is 2 / pi *)

    val inv_pi_4 : float
    (** [inv_pi_4] is 4 / pi *)

    val inv_pi_8 : float
    (** [inv_pi_8] is 8 / pi *)

    val e : float
    (** [e] is an approximation of e with double precision *)

    val inv_e : float
    (** [inv_e] is 1 / e *)

    val log2_e : float
    (** [log2_e] is log{_2}(e) *)

    val log10_e : float
    (** [log10_e] is log{_10}(e) *)

    val ln_2 : float
    (** [ln_2] is ln(2) *)

    val ln_10 : float
    (** [ln_10] is ln(10) *)

  end

  module Vector : sig
  (** Vector manipulation functions *)

    type t = (float * float)
    (** A vector is a pair of floats of the form (x, y)  *)

    val zero : t
    (** [zero] is the zero/null vector *)

    val unit : t
    (** [unit] is the unit vector *)

    val add : t -> t -> t
    (** [add v1 v2] returns the sum of vectors [v1] and v2 *)

    val sub : t -> t -> t
    (** [sub v1 v2] returns the difference of vectors [v1] and [v2] *)

    val mul : t -> float -> t
    (** [mul v k] returns the product of vector [v] by the scalar [k] *)

    val dot  : t -> t -> float
    (** [dot v1 v2] returns the dot product of vectors [v1] and [v2] *)

    val norm : t -> float
    (** [norm v] computes the norm of vector [v] *)

  end

  module Transform : sig
  (** Transform manipulation functions *)

    type t = {
      a : float; (** x scaling/flipping, rotation *)
      b : float; (** x shearing, rotation *)
      c : float; (** y shearing, rotation *)
      d : float; (** y scaling/flipping, rotation *)
      e : float; (** x translation *)
      f : float; (** y translation *)
    }
    (** A type to represent transformation matrices of the form:
 {[
        a b 0
        c d 0
        e f 1 ]} *)

    val id : t
    (** Identity transformation *)

    val create : (float * float * float * float * float * float) -> t
    (** [create t] creates a transformation given
        the matrix [t = (a, b, c, d, e, f)] *)

    val mul : t -> t -> t
    (** [mul t1 t2] multiplies [t1] by [t2] *)

    val translate : t -> Vector.t -> t
    (** [translate t v] composes [t] by a translation of vector [v] *)

    val scale : t -> Vector.t -> t
    (** [scale t v] composes [t] by a scaling of vector [v] *)

    val shear : t -> Vector.t -> t
    (** [shear t v] composes [t] by a shearing of vector [v] *)

    val rotate : t -> float -> t
    (** [rotate t a] composes [t] by a rotation
        of angle [a] around the origin *)

    val inverse : t -> t
    (** [inverse t] returns the inverse of [t]

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if [t] is a singular transformation matrix }} *)

  end

  module Point : sig
  (** Point manipulation functions *)

    type t = (float * float)
    (** A point is a pair of floats of the form (x, y)  *)

    val of_ints : (int * int) -> t
    (** [of_ints (x, y)] creates a point from integer coordinates *)

    val sub : t -> t -> Vector.t
    (** [sub p1 p2] returns the difference between points [p1] and [p2] *)

    val translate : t -> by:Vector.t -> t
    (** [translate p ~by] translates point [p] by vector [by] *)

    val rotate : t -> around:t -> theta:float -> t
    (** [rotate p ~around ~theta] rotates point [p] around
        a central point [around] by an angle [theta] *)

    val transform : t -> Transform.t -> t
    (** [transform p t] transforms point [p] by the given transform [t] *)

    val barycenter : float -> t -> float -> t -> t
    (** [barycenter a p1 b p2] compute the barycenter
        of ([a], [p1]) and ([b], [p2])

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if [a] + [b] = 0.0}} *)

    val distance : t -> t -> float
    (** [distance p1 p2] computes the distance between [p1] and [p2] *)

  end

  module Color : sig
  (** Color description and manipulation functions *)

    type t
    (** Abstract type for colors *)

    val of_rgb : int -> int -> int -> t
    (** [of_rgb r g b] creates a color from its [r], [g] and [b] components.
        Those components are automatically clipped to the 0-255 range. *)

    val to_rgb : t -> int * int * int
    (** [to_rgb c] converts a color to its [r], [g] and [b] components,
        ignoring the alpha component *)

    val of_argb : int -> int -> int -> int -> t
    (** [of_argb a r g b] creates a color from it [a], [r], [g] and [b]
        components. Those components are automatically clipped to the
        0-255 range. *)

    val to_argb : t -> int * int * int * int
    (** [to_argb c] converts a color to its [a], [r], [g] and [b] components *)

    val of_int : int -> t
    (** [of_int i] creates a color from its 24-bit integer representation [i] ;
        this representation does not include the alpha component.
        This integer is clipped to 24-bit beforehand. *)

    val to_int : t -> int
    (** [to_int c] converts a color [c] to its 24-bit integer representation ;
        this representation does not include the alpha component *)

    val of_int32 : Int32.t -> t
    (** [of_int i] creates a color from its 32-bit integer representation [i] *)

    val to_int32 : t -> Int32.t
    (** [to_int c] converts a color [c] to its 32-bit integer representation *)

    val transpBlack : t
    (** Predefined transparent black color *)

    val transpWhite : t
    (** Predefined transparent white color *)

    val black : t
    (** Predefined black color *)

    val white : t
    (** Predefined white color *)

    val blue : t
    (** Predefined blue color *)

    val cyan : t
    (** Predefined cyan color *)

    val green : t
    (** Predefined green color *)

    val lime : t
    (** Predefined lime color *)

    val orange : t
    (** Predefined orange color *)

    val pink : t
    (** Predefined pink color *)

    val red : t
    (** Predefined red color *)

    val of_string : string -> t
    (** [of_string s] returns the color associated with string [s].
        [s] can be either a color name (as defined by [define_color]),
        or an hexadecimal representation of the form #AARRGGBB.
        If the color is unknown or unparsable, this returns
        [transparent_black]. *)

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
      | Oblique (** *)
    (** Font slant *)

    type weight = int
    (** Font weight *)

    val thin : weight
    (** Predefined thin weight *)

    val extraLight : weight
    (** Predefined extraLight weight *)

    val light : weight
    (** Predefined light weight *)

    val regular : weight
    (** Predefined regular weight *)

    val medium : weight
    (** Predefined medium weight *)

    val semiBold : weight
    (** Predefined semiBold weight *)

    val bold : weight
    (** Predefined bold weight *)

    val extraBold : weight
    (** Predefined extraBold weight *)

    val black : weight
    (** Predefined black weight *)

  end

  module ImageData : sig
  (** Image data manipulation functions *)

    type t
    (** An abstract type representing an image data *)

    val create : (int * int) -> t
    (** [create size] creates an empty image data of the given [size].

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val createFromPNG : string -> t React.event
    (** [createFromPNG filename] creates an image data with
        the contents of PNG file [filename]. The returned
        event will be triggered once the image is loaded.

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Exception.Read_png_failed} if the PNG file could not be read}} *)

    val getSize : t -> (int * int)
    (** [getSize id] returns the size of image data [id] *)

    val fill : t -> Color.t -> unit
    (** [fill id c] fills the image data [id] with the given color [c] *)

    val sub : t -> pos:(int * int) -> size:(int * int) -> t
    (** [sub c ~pos ~size] returns a copy of the pixel data
        at position [pos] of size [size] in image data [id].
        Any pixel outside the image bounds is considered
        to be transparent black.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val blit :
      dst:t -> dpos:(int * int) ->
      src:t -> spos:(int * int) -> size:(int * int) -> unit
    (** [blit ~dst ~dpos ~src ~spos ~size] copies the area specified
        by [spos] and [size] from image data [src] to image data [dst]
        at position [dpos]. If the given position and size yield an
        inconsistent area, this has no effect.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val getPixel : t -> (int * int) -> Color.t
    (** [getPixel id pos] returns the color of the pixel at position
        [pos] in image data [id]. If [pos] is outside the image
        bounds, returns the transparent black color. *)

    val putPixel : t -> (int * int) -> Color.t -> unit
    (** [putPixel id pos c] sets the color of the pixel at position
        [pos] in image data [id] to color [c]. If [pos] is
        outside the image bounds, this has no effect. *)

    val importPNG : t -> pos:(int * int) -> string -> t React.event
    (** [importPNG id ~pos filename] loads the file [filename]
        into image data [id] at position [pos]. Any pixel that
        falls outside the image bounds is ignored. The returned
        event will be triggered once the image is loaded.

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Exception.Read_png_failed} if the PNG file could not be read}} *)

    val exportPNG : t -> string -> unit
    (** [exportPNG id filename] saves the contents of image
        data [id] to a file with name [filename]

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Exception.Write_png_failed} if the PNG file could not be written}} *)

    type t_repr =
      (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array3.t
    (** Image data's internal representation is a big array of dimension 3
        (height, width, component), with the components in BGRA order *)

    val of_bigarray : t_repr -> t
    (** [of_bigarray ba] reinterprets a big array [ba] as an image data.
        The big array must be of dimension 3 (height, width, component),
        with the components in BGRA order. The underlying memory
        will be shared between the image data and the big array.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if the first or second dimension of [ba] is outside the range 1-32767, or if the third dimension of [ba] is not 4}} *)

    val to_bigarray : t -> t_repr
    (** [to_bigarray id] reinterprets an image data [id] as a big array.
        The resulting big array will be of dimension 3 (height, width,
        component), with the components in BGRA order. The underlying
        memory will be shared between the image data and the big array.  *)

  end

  module Gradient : sig
  (** Gradient manipulation functions *)

    type t
    (** An abstract type representing a gradient *)

    val createLinear : pos1:Point.t -> pos2:Point.t -> t
    (** [createLinear ~pos1 ~pos2] creates a new linear gradient
        parallel to the line ([pos1], [pos2]) in window coordinates *)

    val createRadial:
      center1:Point.t -> rad1:float -> center2:Point.t -> rad2:float -> t
    (** [createRadial ~center1 ~rad1 ~center2 ~rad2] creates a new
        radial gradient between the disks with centers [center1] and
        [center2] and radi [rad1] and [rad2] in window coordinates *)

    val createConic: center:Point.t -> angle:float -> t
    (** [createConic ~center ~angle] creates a new conic gradient
        with center [center] and initial angle [angle] *)

    val addColorStop : t -> Color.t -> float -> unit
    (** [addColorStop gradient color stop] adds a new [color]
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
      | RepeatXY (** *)
    (** Pattern repetition *)

    val create : ImageData.t -> repeat -> t
    (** [create img rep] creates a pattern of [rep] using [img] as source *)

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

    val rect : t -> pos:Point.t -> size:Vector.t -> unit
    (** [rect p ~pos ~size] adds the rectangle specified by [pos]
        and [size]) to the path [p] *)

    val ellipse :
      t -> center:Point.t -> radius:Vector.t ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
    (** [ellipse p ~center ~radius ~rotation ~theta1 ~theta2] adds
        an ellipse with the given parameters to the path [p] *)

    val add : t -> t -> unit
    (** [add dst src] adds the path [src] into the path [dst] *)

    val addTransformed : t -> t -> Transform.t -> unit
    (** [addTransformed dst src t] adds the path [src] after
        applying [t] to each of its points into the path [dst] *)

  end

  module Join : sig

    type t =
      | Round
      | Miter
      | Bevel (** *)
    (** The different kinds of line joins *)

  end

  module Cap : sig

    type t =
      | Butt
      | Square
      | Round (** *)
    (** The different kinds of line caps *)

  end

  module Style : sig

    type t =
      | Color of Color.t
      | Gradient of Gradient.t
      | Pattern of Pattern.t (** *)
    (** Stroke and fill styles *)

  end

  module CompositeOp : sig

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
      | Luminosity (** *)
    (** Blending and compositing operations *)

  end

  module Canvas : sig
  (** Canvas manipulation functions *)

    type t
    (** An abstract type representing a canvas *)


    (** {1 Comparison and hash functions} *)

    (** In order to ease identification of canvas or building collections
        of canvas, the usual comparison and hash functions are provided.
        These functions simply operate on the canvas' unique ids.
        Also, standard structural and physical comparison
        operators can be used to compare canvases. *)

    val compare : t -> t -> int
    (** [compare c1 c2] is equivalent to [compare (getId c1) (getId c2)] *)

    val hash : t -> int
    (** [hash c] returns a unique integer value for canvas [c],
        which is computed as [Hashtbl.hash (Canvas.getId c)] *)


    (** {1 Creation} *)

    val createOnscreen :
      ?autocommit:bool -> ?decorated:bool -> ?resizeable:bool ->
      ?minimize:bool -> ?maximize:bool -> ?close:bool -> ?title:string ->
      ?pos:(int * int) -> size:(int * int) -> unit -> t
    (** [createOnscreen ?autocommit ?decorated ?resizeable ?minimize
        ?maximize ?close ?title ?pos ~size ()] creates a windowed
        canvas of size [size]. The window title and position can be
        specified using the optional arguments [title] and [pos].
        The window decorations, which are active by default, can
        be disabled using the optional arguments [decorated],
        [resizeable], [minimize], [maximize], and [close].
        The [decorated] argument has a higher priority: if set to false,
        all other decoration arguments will be ignored (considered to be
        false), and all decorations will be removed from the window.
        The [autocommit] option, which is active by default, indicates whether
        the canvas should be automatically presented after each frame event.
        See {!Canvas.commit} for more info on [autocommit].

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val createOffscreen : size:(int * int) -> unit -> t
    (** [createOffscreen ~size] creates an offscreen canvas of size [size]

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val createOffscreenFromImageData : ImageData.t -> t
    (** [createOffscreenFromImageData id] creates an offscreen
        canvas with the contents of image data [id]

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}} *)

    val createOffscreenFromPNG : string -> t React.event
    (** [createOffscreen filename] creates an offscreen canvas
        with the contents of PNG file [filename]. The returned
        event will be triggered once the image is loaded.

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Exception.Read_png_failed} if the PNG file could not be read}} *)


    (** {1 Visibility} *)

    val show : t -> unit
    (** [show c] makes the canvas [c] visible on screen.
        Does nothing on offscreen canvases. *)

    val hide : t -> unit
    (** [hide c] makes the canvas [c] invisible.
        Does nothing offscreen canvases. *)

    val close : t -> unit
    (** [close c] closes the canvas [c], i.e. it permanently removes
        it from the screen and prevents it to receive events ;
        however it can still be used as an offscreen canvas.
        Does nothing on offscreen canvases. *)


    (** {1 Rendering} *)

    val commit : t -> unit
    (** [commit c] informs the backend that the canvas has been modified and
        should be presented on screen. This is not necessary if the canvas has
        been created with autocommit set to true, as in this case the canvas
        will be automatically presented after each frame event. Note that
        it is also useless when using the Javascript backend ; however, to
        maintain consistent behavior between the various backends, do remember
        to use [commit] on any canvas created with autocommit set to false.
        Does nothing on offscreen canvases. *)


    (** {1 Configuration} *)

    val getId : t -> int
    (** [getId c] returns the unique id of canvas [c] *)

    val getSize : t -> (int * int)
    (** [getSize c] returns the size of canvas [c] *)

    val setSize : t -> (int * int) -> unit
    (** [setSize c size] sets the size of canvas [c]

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val getPosition : t -> (int * int)
    (** [getPosition c] returns the position of canvas [c].
        Returns (0, 0) when used on offscreen cavnas. *)

    val setPosition : t -> (int * int) -> unit
    (** [setPosition c pos] sets the position of canvas [c].
        Does nothing on offscreen canvases. *)


    (** {1 State} *)

    val save : t -> unit
    (** [save c] pushes the current state of canvas [c] onto the state stack *)

    val restore : t -> unit
    (** [restore c] pops the current state of canvas [c] from the state stack *)


    (** {1 Transformations} *)

    val setTransform : t -> Transform.t -> unit
    (** [setTransform c t] sets the current transformation matrix of canvas [c].
        The matrix [t = { a, b, c, d, e, f }] is of the following form:
{[
        a b 0
        c d 0
        e f 1 ]} *)

    val transform : t -> Transform.t -> unit
    (** [transform c t] apply the given arbitrary transformation
        to the current transformation matrix of canvas [c] *)

    val translate : t -> Vector.t -> unit
    (** [translate c vec] apply the given translation transform
        to the current transformation matrix of canvas [c] *)

    val scale : t -> Vector.t -> unit
    (** [scale c vec] apply the given scale transform
        to the current transformation matrix of canvas [c] *)

    val shear : t -> Vector.t -> unit
    (** [shear c vec] apply the given shear transform
        to the current transformation matrix of canvas [c] *)

    val rotate : t -> float -> unit
    (** [rotate c theta] apply the given rotation transform
        to the current transformation matrix of canvas [c] *)


    (** {1 Style} *)

    val getLineWidth : t -> float
    (** [getLineWidth c] returns the current line width of canvas [c] *)

    val setLineWidth : t -> float -> unit
    (** [setLineWidth c w] sets the current line width of canvas [c] to [w] *)

    val getLineJoin : t -> Join.t
    (** [getLineJoin c] returns the current line join type of canvas [c] *)

    val setLineJoin : t -> Join.t -> unit
    (** [setLineJoin c j] sets the current line join type of canvas[c] to [j] *)

    val getLineCap : t -> Cap.t
    (** [getLineJoin c] returns the current line cap type of canvas [c] *)

    val setLineCap : t -> Cap.t -> unit
    (** [setLineJoin c j] sets the current line cap type of canvas[c] to [j] *)

    val getMiterLimit : t -> float
    (** [getMiterLimit c] returns the current miter limit of canvas [c] *)

    val setMiterLimit : t -> float -> unit
    (** [getMiterLimit c m] sets the current miter limit of canvas [c] to [m] *)

    val getLineDashOffset : t -> float
    (** [getLineDashOffset c] returns the current line offset of [c] *)

    val setLineDashOffset : t -> float -> unit
    (** [setLineDashOffset c t] sets the current line offset of [c] to [t] *)

    val getLineDash : t -> float array
    (** [getLineDash c t] returns the current line dash pattern of [c] *)

    val setLineDash : t -> float array -> unit
    (** [setLineDash c t] sets the current line dash pattern of [c] to [t] *)

    val getStrokeColor : t -> Color.t
    (** [getStrokeColor c] returns the current stroke color of canvas [c] *)

    val setStrokeColor : t -> Color.t -> unit
    (** [setStrokeColor c col] sets the current
        stroke color of canvas [c] to [col] *)

    val setStrokeGradient : t -> Gradient.t -> unit
    (** [setStrokeGradient c grad] sets the current stroke
        style of canvas [c] to the gradient [grad] *)

    val setStrokePattern : t -> Pattern.t -> unit
    (** [setStrokePattern c pat] sets the current stroke
        style of canvas [c] to the pattern [pat] *)

    val getStrokeStyle : t -> Style.t
    (** [getStrokeStyle c] returns the current stroke style of canvas [c] *)

    val setStrokeStyle : t -> Style.t -> unit
    (** [setStrokeStyle c style] sets the current stroke
        style of canvas [c] to style [style] *)

    val getFillColor : t -> Color.t
    (** [getFillColor c] returns the current fill color of canvas [c] *)

    val setFillColor : t -> Color.t -> unit
    (** [setFillColor c col] sets the current
        fill color of canvas [c] to [col] *)

    val setFillGradient : t -> Gradient.t -> unit
    (** [setFillGradient c grad] sets the current fill
        style of canvas [c] to the gradient [grad] *)

    val setFillPattern : t -> Pattern.t -> unit
    (** [setFillPattern c pat] sets the current fill
        style of canvas [c] to the pattern [pat] *)

    val getFillStyle : t -> Style.t
    (** [getFillStyle c] return the current fill style of canvas [c] *)

    val setFillStyle : t -> Style.t -> unit
    (** [setFillStyle c style] sets the current
        fill style of canvas [c] to style [style] *)

    val getGlobalAlpha : t -> float
    (** [getGlobalAlpha c] returns the current global alpha of canvas [c] *)

    val setGlobalAlpha : t -> float -> unit
    (** [setGlobalAlpha c a] sets the global alpha value of canvas[c] to [a] *)

    val getGlobalCompositeOperation : t -> CompositeOp.t
    (** [getGlobalCompositeOperation c] returns the global
        composite or blending operation of canvas[c] *)

    val setGlobalCompositeOperation : t -> CompositeOp.t -> unit
    (** [setGlobalCompositeOperation c o] sets the global
        composite or blending operation of canvas[c] to [o] *)

    val getShadowColor : t -> Color.t
    (** [setShadowColor c] returns the canvas [c]'s shadow color *)

    val setShadowColor : t -> Color.t -> unit
    (** [setShadowColor c col] sets the canvas [c]'s shadow color to [col] *)

    val getShadowBlur : t -> float
    (** [setShadowBlur c] returns the shadow blur radius of canvas [c]  *)

    val setShadowBlur : t -> float -> unit
    (** [setShadowBlur c b] sets the shadow blur radius of canvas [c] to [b] *)

    val getShadowOffset : t -> Vector.t
    (** [setShadowOffset c] returns the offset of the shadows drawn in [c] *)

    val setShadowOffset : t -> Vector.t -> unit
    (** [setShadowOffset c o] sets the offset
        of the shadows drawn in [c] to [o] *)

    val setFont :
      t -> string -> size:Font.size ->
      slant:Font.slant -> weight:Font.weight -> unit
    (** [setFont c family ~size ~slant ~weight] sets the current
        font of canvas [c] to the one specified by the given
        [family], [size], [slant] and [weight] *)


    (** {1 Path} *)

    val clearPath : t -> unit
    (** [clearPath c] resets the path of canvas [c] *)

    val closePath : t -> unit
    (** [closePath c] closes the current subpath of canvas [c], i.e. adds a
        line from the last point in the current subpath to the first point,
        and marks the subpath as closed. Does nothing if the subpath is empty
        or has a single point, or if the subpath is already closed. *)

    val moveTo : t -> Point.t -> unit
    (** [moveTo c p] starts a new subpath in canvas [c] containing the
        single point [p]. If the current subpath is empty, its first
        point is set to this point, instead of creating a new subpath.
        Likewise, if the current subpath has a single point, it is
        simply replaced by the given point. *)

    val lineTo : t -> Point.t -> unit
    (** [lineTo c p] adds the point [p] to the current subpath of canvas [c].
        If the current subpath is empty, this behaves just like [moveTo c ~p].*)

    val arc :
      t -> center:Point.t -> radius:float ->
      theta1:float -> theta2:float -> ccw:bool -> unit
    (** [arc c ~center ~radius ~theta1 ~theta2 ~ccw] adds an arc of the given
        [radius], centered at [center], between angle [theta1] to [theta2]
        to the current subpath of canvas [c]. If [ccw] is true, the arc will
        be drawn counterclockwise. Note that the last point in the subpath
        (if such point exists) will be connected to the first point of the
        arc by a straight line. *)

    val arcTo : t -> p1:Point.t -> p2:Point.t -> radius:float -> unit
    (** [arcTo c ~p1 ~p2 ~radius] adds an arc of the given [radius]
        using the control points [p1] and [p2] to the current
        subpath of canvas [c]. If the current subpath is empty,
        this behaves as if [moveTo c ~p:p1] was called. *)

    val quadraticCurveTo : t -> cp:Point.t -> p:Point.t -> unit
    (** [quadraticCurveTo c ~cp ~p] adds a quadratic Bezier curve
        using the control point [cp] and the end point [p]
        to the current subpath of canvas [c] *)

    val bezierCurveTo :
      t -> cp1:Point.t -> cp2:Point.t -> p:Point.t -> unit
    (** [bezierCurve c ~cp1 ~cp2 ~p] adds a cubic Bezier curve using
        the control points [cp1] and [cp2] and the end point [p]
        to the current subpath of canvas [c] *)

    val rect : t -> pos:Point.t -> size:Vector.t -> unit
    (** [rect c ~pos ~size] adds the rectangle specified by [pos]
        and [size]) to the current subpath of canvas [c] *)

    val ellipse :
      t -> center:Point.t -> radius:Vector.t ->
      rotation:float -> theta1:float -> theta2:float -> ccw:bool -> unit
    (** [ellipse c ~center ~radius ~rotation ~theta1 ~theta2] adds an ellipse
        with the given parameters to the current subpath of canvas [c] *)


    (** {1 Path stroking and filling} *)

    val fill : t -> nonzero:bool -> unit
    (** [fill c ~nonzero] fills the current subpath of canvas [c]
        using the current fill color and the specified fill rule *)

    val fillPath : t -> Path.t -> nonzero:bool -> unit
    (** [fillPath c p ~nonzero] fills the path [p] on canvas [c]
        using the current fill style and the specified fill rule *)

    val stroke : t -> unit
    (** [stroke c] draws the outline of the current subpath of
        canvas [c] using the current stroke color and line width *)

    val strokePath : t -> Path.t -> unit
    (** [strokePath c p] draws the outline of the path [p] on
        canvas [c] using the current stroke style and line width *)

    val clip : t -> nonzero:bool -> unit
    (** [clipPath c p ~nonzero] intersects the current subpath of [c]
        on canvas [c]'s clip region using the specified fill rule *)

    val clipPath : t -> Path.t -> nonzero:bool -> unit
    (** [clipPath c p ~nonzero] intersects the filled path [p] on
        canvas [c]'s clip region using the specified fill rule *)


    (** {1 Immediate drawing} *)

    val fillRect : t -> pos:Point.t -> size:Vector.t -> unit
    (** [fillRect c ~pos ~size] immediatly fills the rectangle specified by
        [pos] and [size] to the canvas [c] using the current fill color *)

    val strokeRect : t -> pos:Point.t -> size:Vector.t -> unit
    (** [strokeRect c ~pos ~size] immediatly draws the outline of
        the rectangle specified by [pos] and [size] to the canvas
        [c] using the current stroke color and line width *)

    val fillText : t -> string -> Point.t -> unit
    (** [fillText c text pos] immediatly draws the text [text] at
        position [pos] on the canvas [c] using the current fill color *)

    val strokeText : t -> string -> Point.t -> unit
    (** [strokeText c text pos] immediatly draws the outline of text [text]
        at position [pos] on the canvas [c] using the current stroke color
        and line width *)

    val blit :
      dst:t -> dpos:(int * int) ->
      src:t -> spos:(int * int) -> size:(int * int) -> unit
    (** [blit ~dst ~dpos ~src ~spos ~size] copies the area specified
        by [spos] and [size] from canvas [src] to canvas [dst] at
        position [dpos]. If the given position and size yield an
        inconsistent area, this has no effect.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)


    (** {1 Direct pixel access} *)

    (** Warning: these functions (especially the per-pixel functions) can
        be slow and are not meant for updating the contents of a canvas in
        real-time. Better use them on offscreen canvas during loading phases. *)

    val getPixel : t -> (int * int) -> Color.t
    (** [getPixel c pos] returns the color of the pixel at position
        [pos] in canvas [c]. If [pos] is outside the canvas
        bounds, returns the transparent black color. *)

    val putPixel : t -> (int * int) -> Color.t -> unit
    (** [putPixel c pos col] sets the color of the pixel at position
        [pos] in canvas [c] to color [col]. If [pos] is
        outside the canvas bounds, this has no effect. *)

    val getImageData :
      t -> pos:(int * int) -> size:(int * int) -> ImageData.t
    (** [getImageData c ~pos ~size] returns a copy of the pixel
        data at position [pos] of size [size] in canvas [c].
        Any pixel outside the canvas bounds is considered
        to be transparent black.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val putImageData :
      t -> dpos:(int * int) -> ImageData.t ->
      spos:(int * int) -> size:(int * int) -> unit
    (** [setImageData c ~dpos id ~spos ~size] overwrite the pixels
        at position [dpos] in canvas [c] with the provided pixel
        data starting at position [spos] and of size [size].
        If the given position and size yield an
        inconsistent area, this has no effect.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if either component of [size] is outside the range 1-32767}} *)

    val importPNG :
      t -> pos:(int * int) -> string -> t React.event
    (** [importPNG c ~pos filename] loads the file [filename]
        into canvas [c] at position [pos]. The returned
        event will be triggered once the image is loaded.

        {b Exceptions:}
        {ul
        {- {!Exception.Read_png_failed} if the PNG file could not be read}} *)

    val exportPNG : t -> string -> unit
    (** [exportPNG c filename] saves the contents of
        canvas [c] to a file with name [filename]

        {b Exceptions:}
        {ul
        {- {!Exception.Write_png_failed} if the PNG file could not be written}} *)

  end

  module Event : sig
  (** Event descriptions *)

    (** All event descriptions contain the [canvas] on which the event
        occured and the [timestamp] indicating when the event occured. *)

    type timestamp = Int64.t
    (** Timestamps represent the time in microseconds,
        from an arbitrary starting point *)

    type 'a canvas_event = {
      canvas: Canvas.t;
      timestamp: timestamp;
      data: 'a;
    }
    (** Generic description of events occurring on canvases *)

    type position = int * int
    (** A position is a pair of integers *)

    type size = int * int
    (** A size is a pair of integers *)

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
      | KeyVolumeDown
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

    type key_data = {
      key: key;      (** Physical key that was pressed/released *)
      char: Uchar.t; (** Equivalent Unicode character in the current layout *)
      flags: flags;  (** State of various modifier keys when the event occured*)
    }
    (** Describes a keyboard event *)

    type button =
      | ButtonNone
      | ButtonLeft
      | ButtonMiddle
      | ButtonRight
      | ButtonWheelUp
      | ButtonWheelDown
    (** A mouse button *)

    type button_data = {
      position: position; (** Cursor position when the event occured *)
      button: button;     (** Button that was pressed/released *)
    }
    (** Describes a mouse button event *)

    val frame : unit canvas_event React.event
    (** Occurs when the backend determines it is time to draw a new frame.
        A typical frequency is 60 times per second, though this varies
        depending on the platform. *)

    val focus_in : unit canvas_event React.event
    (** Occurs when the canvas becomes active as a
        result of being clicked or tabbed-into *)

    val focus_out : unit canvas_event React.event
    (** Occurs when the canvas becomes inactive as a result
        of another canvas being clicked or tabbed-into *)

    val resize : size canvas_event React.event
    (** Occurs when the canvas is resized by a user action *)

    val move : position canvas_event React.event
    (** Occurs when the canvas is moved by a user action *)

    val close : unit canvas_event React.event
    (** Occurs when the user clicks the close button *)

    val key_down : key_data canvas_event React.event
    (** Occurs when the user presses a key on the keyboard. The event
        description contains both the physical key (of type {!Event.key}
        and the Unicode character corresponding to that key (if any),
        according to the current keyboard layout. For instance, pressing
        the "A" key on an AZERTY keyboard will yield a physical key
        [Event.KeyQ] and the Unicode code point for character "A". *)

    val key_up : key_data canvas_event React.event
    (** Occurs when the user releases a key on the keyboard *)

    val button_down : button_data canvas_event React.event
    (** Occurs when the user presses a mouse button *)

    val button_up : button_data canvas_event React.event
    (** Occurs when the user releases a mouse button *)

    val mouse_move : size canvas_event React.event
    (** Occurs when the user moves the mouse cursor *)

    val event_timestamp : Int64.t React.signal
    (** The timestamp of the last canvas event that occurred *)

    val int_of_key : key -> int
    (** [int_of_key k] returns a platform-independent integer representation
        of key [k]. This integer corresponds to the key code as defined
        by the USB standard for keybords.
        @see <https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf>
    *)

    val key_of_int : int -> key
    (** [key_of_int i] returns the key corresponding to the
        platform-independent integer [i] in the range 0-255.

        {b Exceptions:}
        {ul
        {- {!Invalid_argument} if [i] < 0 or [i] > 255}} *)


  end

  module Backend : sig
  (** Initialization and event loop control *)

    val init : unit -> unit
    (** [init ()] initializes the backend

        {b Exceptions:}
        {ul
        {- {!Failure} if initialization fails}} *)

    val run : (unit -> 'dummy1) -> 'dummy2
    (** [run k] executes the backend event loop, calling update functions as
        needed, and calling the continuation function [k] when the event loop
        terminates. Note that the call to [run] MUST be the last instruction
        in your program (to avoid different behaviors between the native and
        javascript backends). If you need to perform additional stuff when
        the program terminates, you MUST use the [k] function: it is meant
        for that. Note that calling [run] from an update function will just
        be ignored (though this should not be done). However, [run] may
        be called from the [k] function, if needed.

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}} *)

    val stop : unit -> unit
    (** [stop ()] requests termination of the currently running event
        loop, if any. It should be called from an update function.
        Actual termination of the event loop will occur at the end of
        the current iteration of the event loop, so after calling [stop]
        an update function should proceed normally until it returns.

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}} *)

    val getCanvas : int -> Canvas.t
    (** [getCanvas i] returns the canvas that has id [i], if it exists

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}
        {- {!Not_found} if no canvas has id [i]}} *)

    val getCurrentTimestamp : unit -> Event.timestamp
    (** [getCurrentTimestamp ()] returns the current timestamp
        in microseconds, from an arbitrary starting point

        {b Exceptions:}
        {ul
        {- {!Exception.Not_initialized} if {!Backend.init} was not called}} *)

  end

end
