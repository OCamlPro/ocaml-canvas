Quick start
===========

Initialization
--------------

Before using any function in the library (and assuming the :code:`OCamlCanvas.V1`
module has been opened), the user should call :code:`Backend.init` so that
the library makes any internal initialization it needs for the current
backend. This function takes as input a value of type :code:`Backend.options`,
allowing to tweak specific features of each backend.
It is however recommended to simply use the default options,
i.e :code:`Backend.default_options`.

Creating canvases
-----------------

Once the backend is initialized, one can create Canvas objects
using the :code:`Canvas.createFramed`, :code:`Canvas.createFrameless` and
:code:`Canvas.createOffscreen` functions. The first one creates a canvas
contained in a regular window (which is simulated in the Javascript
backend). The second one creates a window that has no decoration at
all. The last one creates canvases that are not rendered on screen,
which can be useful to save complex images that can then simply
copied to a visible canvas. Onscreen canvases are hidden by default,
and :code:`Canvas.show` should be called on them to make them visible.

Drawing on canvases
-------------------

Drawing on a canvas can be perfomed using various drawing primitives,
the most ubiquitous being :code:`Canvas.clearPath`, :code:`Canvas.moveTo`,
:code:`Canvas.lineTo`, :code:`Canvas.arc`, :code:`Canvas.bezierCurveTo`, :code:`Canvas.fill`
and :code:`Canvas.stroke`. These functions allow to build a path step by step
and either fill it completely or draw its outline. It is also possible
to directly render some text with the :code:`Canvas.fillText` and
:code:`Canvas.strokeText` functions.

The canvas drawing style can be customized using functions
such as :code:`Canvas.setFillColor`, :code:`Canvas.setStrokeColor` or
:code:`Canvas.setLineWidth`. The font used to draw text can be specified
with the :code:`Canvas.setFont` function. It is also possible to apply
various transformations to a canvas, such as translation, rotation and
scaling, with the functions :code:`Canvas.transform`, :code:`Canvas.translate`,
:code:`Canvas.scale`, :code:`Canvas.rotate` and :code:`Canvas.shear`. All these
styling elements can be saved and restored to/from a state stack
using the functions :code:`Canvas.save` and :code:`Canvas.restore`.

Handling canvas events
----------------------

Once the canvases are ready, we may start handling events for these canvases.
To do so, we use the :code:`Backend.run` function, which runs an event loop.
**This function MUST be the last instruction of the program**. It takes three
arguments: the first one is an event handler function, the second one is
is executed when the event loop has finished running, and the third one
is an initial state of an arbitrary type. The event loop may be stopped
by calling :code:`Backend.stop` from the event handler.

The event handler function takes two arguments: a state, and an event
description. It should pattern-match the event against the various
constructors of the :code:`Event.t` type it is interested in, and return a
pair consisting of the new state and a boolean value indicating whether
the event was processed (for some events, this may have some side-effect,
indicated in the event's description). Each event reports at least the
canvas on which it occured, and its timestamp. It may also report mouse
coordinates for mouse events, or keyboard status for keyboard events.

An actual example
-----------------

The following program creates a windowed canvas with an orange background,
a cyan border, and the "Hello world !" text drawn rotated in the middle.
The user may press the "Escape" key or close the window to exit the
program. It will show the number of frames displayed when quitting.
::
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

      Backend.run (fun state -> function

        | Event.CanvasClosed { canvas = _; timestamp = _ } ->
            Backend.stop ();
            state, true

        | Event.KeyAction { canvas = _; timestamp = _; key;
                            char = _; flags = _; state = Down } ->
            if key = Event.KeyEscape then
              Backend.stop ();
            state, true

        | Event.ButtonAction { canvas = _; timestamp = _; position = (x, y);
                               button = _; state = Down } ->
            Canvas.setFillColor c Color.red;
            Canvas.clearPath c;
            Canvas.arc c ~center:(float_of_int x, float_of_int y)
              ~radius:5.0 ~theta1:0.0 ~theta2:(pi *. 2.0) ~ccw:false;
            Canvas.fill c ~nonzero:false;
            state, true

        | Event.Frame { canvas = _; timestamp = _ } ->
            Int64.add state Int64.one, true

        | _ ->
            state, false

      ) (fun state ->
          Printf.printf "Displayed %Ld frames. Goodbye !\n" state
      ) 0L
