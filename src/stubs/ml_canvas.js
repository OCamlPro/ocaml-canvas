/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/



/* JS Event handlers */

//Provides: _focus
var _focus = null;

//Provides: _move
var _move = {
  moving: false,
  target: null,
  prev_x: 0,
  prev_y: 0
}

//Provides: _make_key_event
//Requires: _focus,keyname_to_keycode,Val_key_code,Val_key_state,EVENT_TAG
function _make_key_event(e, state) {
  var char = e.key.length === 1 ? e.key.charCodeAt(0) : 0;
  var flags = [ 0, e.shiftKey, e.altKey, e.ctrlKey, e.metaKey,
                   e.getModifierState("CapsLock"),
                   e.getModifierState("NumLock"),
                   e.key === "Dead" ];
  var evt = [ EVENT_TAG.KEY_ACTION,
              [ 0, _focus, e.timeStamp * 1000.0,
                Val_key_code(keyname_to_keycode(e.code)),
                char, flags, Val_key_state(state) ] ];
  return evt;
/*
  e.isComposing
  e.location
  e.getModifierState()
*/
}

//Provides: _key_down_handler
//Requires: _focus,_make_key_event,_ml_canvas_mlEventListener,KEY_STATE
function _key_down_handler(e) {
  if (_focus !== null) {
    var evt = _make_key_event(e, KEY_STATE.DOWN);
    _ml_canvas_mlEventListener(evt);
  }
  return false;
}

//Provides: _key_up_handler
//Requires: _focus,_make_key_event,_ml_canvas_mlEventListener,KEY_STATE
function _key_up_handler(e) {
  if (_focus !== null) {
    var evt = _make_key_event(e, KEY_STATE.UP);
    _ml_canvas_mlEventListener(evt);
  }
  return false;
}

//Provides: _header_down_handler
//Requires: _focus,_move,ml_canvas_destroy,_ml_canvas_mlEventListener,EVENT_TAG
function _header_down_handler(e) {
  if (e.target !== null) {
    _focus = e.target.canvas;
    if ((e.offsetX >= e.target.canvas.width - 20) &&
        (e.offsetX <= e.target.canvas.width - 10) &&
        (e.offsetY >= 10) && (e.offsetY <= 20)) {
      var evt = [ EVENT_TAG.CANVAS_CLOSED,
                  [ 0, e.target.canvas, e.timeStamp * 1000.0 ] ];
      _ml_canvas_mlEventListener(evt);
      ml_canvas_destroy(e.target.canvas);
      _focus = null;
      return false;
    }
    _move.moving = true;
    _move.target = e.target.canvas.frame;
    _move.prev_x = e.pageX;
    _move.prev_y = e.pageY;
    document.body.insertBefore(_move.target, null);
  }
  return false;
}

//Provides: _surface_down_handler
//Requires: _focus,_ml_canvas_mlEventListener,EVENT_TAG
function _surface_down_handler(e) {
  if (e.target !== null) {
    _focus = e.target.canvas;
    document.body.insertBefore(e.target.canvas.frame, null);
    var evt = [ EVENT_TAG.BUTTON_ACTION,
                [ 0, e.target.canvas, e.timeStamp * 1000.0,
                  [ 0, e.offsetX, e.offsetY ], e.button + 1, 1 ] ];

    _ml_canvas_mlEventListener(evt);
  }
  return false;
}

//Provides: _up_handler
//Requires: _move
function _up_handler(e) {
  _move.moving = false;
  return false; // = prevent default behavior
}

//Provides: _move_handler
//Requires: _move
function _move_handler(e) {
  if (_move.moving) {
    var new_x = e.pageX;
    var new_y = e.pageY;
    var dx = new_x - _move.prev_x;
    var dy = new_y - _move.prev_y;
    _move.prev_x = new_x;
    _move.prev_y = new_y;
    var canvas = _move.target.canvas;
    canvas.x += dx;
    canvas.y += dy;
    _move.target.style.left = canvas.x + "px";
    _move.target.style.top = canvas.y + "px";
  }
  return false;
}

//Provides: _frame_handler
//Requires: _ml_canvas_mlEventListener,EVENT_TAG
function _frame_handler(timestamp) {

  var surfaces = document.getElementsByTagName("canvas");

  for (var i = 0; i < surfaces.length; ++i) {
    var evt = [ EVENT_TAG.FRAME,
                [ 0, surfaces[i].canvas, timestamp * 1000.0 ] ];
    _ml_canvas_mlEventListener(evt);
  }

  window.requestAnimationFrame(_frame_handler);
}



/* Image Data (aka Pixmaps) */

//Provides: _ml_canvas_ba_of_img
//Requires: caml_ba_create_unsafe
function _ml_canvas_ba_of_img(img) {
  var surface = document.createElement("canvas");
  surface.width = img.width;
  surface.height = img.height;
  var ctxt = surface.getContext("2d");
  ctxt.drawImage(img, 0, 0);
  var image = ctxt.getImageData(0, 0, img.width, img.height);
  var ta = new window.Uint8Array(image.data.buffer);
  return caml_ba_create_unsafe(3 /* Uint8Array */, 0 /* c_layout */,
                               [img.height, img.width, 4], ta);
}

//Provides: _ml_canvas_surface_of_ba
//Requires: caml_ba_to_typed_array,caml_ba_dim_1,caml_ba_dim_2
function _ml_canvas_surface_of_ba(data) {
  var surface = document.createElement("canvas");
  surface.width = caml_ba_dim_2(data);
  surface.height = caml_ba_dim_1(data);
  var ctxt = surface.getContext("2d");
  var ta = new window.Uint8ClampedArray(caml_ba_to_typed_array(data).buffer);
  if (window.ImageData === undefined) {
    var image = ctxt.createImageData(surface.width, surface.height);
    image.data.set(ta);
  } else {
    var image = new window.ImageData(ta, surface.width, surface.height);
  }
  ctxt.putImageData(image, 0, 0);
  return surface;
}

//Provides: ml_canvas_image_data_create_from_png
//Requires: _ml_canvas_image_of_png_file,_ml_canvas_ba_of_img
function ml_canvas_image_data_create_from_png(filename) {
  var img = _ml_canvas_image_of_png_file(filename);
  if (img === null) {
    return null;
  }
  return _ml_canvas_ba_of_img(img);
}

//Provides: ml_canvas_image_data_import_png
//Requires: _ml_canvas_surface_of_ba,_ml_canvas_image_of_png_file
//Requires: caml_ba_to_typed_array,caml_ba_dim_1,caml_ba_dim_2
function ml_canvas_image_data_import_png(data, pos, filename) {
  var surface = _ml_canvas_surface_of_ba(data);
  var img = _ml_canvas_image_of_png_file(filename);
  if ((surface !== null) && (img !== null)) {
    var ctxt = surface.getContext("2d");
    ctxt.drawImage(img, pos[1], pos[2]);
    var image = ctxt.getImageData(0, 0, surface.width, surface.height);
    var sta = new window.Uint8Array(image.data.buffer);
    var dta = caml_ba_to_typed_array(data);
    for (var i = 0; i < dta.length; i++) {
      dta[i] = sta[i];
    }
  }
}

//Provides: ml_canvas_image_data_export_png
//Requires: _ml_canvas_surface_of_ba,caml_create_file
function ml_canvas_image_data_export_png(data, filename) {
  var surface = _ml_canvas_surface_of_ba(data);
  if (surface !== null) {
    var data = surface.toDataURL("image/png").substring(22);
    caml_create_file(filename, window.atob(data));
  }
}



/* Gradients */

//Provides: ml_canvas_create_linear_gradient
function ml_canvas_create_linear_gradient(canvas, pos1, pos2) {
  return canvas.ctxt.createLinearGradient(pos1[1], pos1[2], pos2[1], pos2[2]);
}

//Provides: ml_canvas_create_radial_gradient
function ml_canvas_create_radial_gradient(canvas,
                                          center1, rad1,
                                          center2, rad2) {
  return canvas.ctxt.createRadialGradient(center1[1], center1[2], rad1,
                                          center2[1], center2[2], rad2);
}

//Provides: ml_canvas_create_conic_gradient
function ml_canvas_create_conic_gradient(canvas, center, angle) {
  return canvas.ctxt.createConicGradient(angle, center[1], center[2]);
}

//Provides: ml_canvas_gradient_add_color_stop
//Requires: _color_of_int
function ml_canvas_gradient_add_color_stop(grad, color, pos) {
  grad.addColorStop(pos, _color_of_int(color));
}



/* Canvas */

/* Comparison */

// Provides: ml_canvas_hash
function ml_canvas_hash(canvas) {
  return canvas.id;
}

// Provides: ml_canvas_compare
function ml_canvas_compare(canvas1, canvas2) {
  if (canvas1.id < canvas2.id) {
    return -1;
  } else if (canvas1.id > canvas2.id) {
    return 1;
  } else {
    return 0;
  }
}



/* Creating / destruction */

//Provides: _next_id
var _next_id = 0;

//Provides: _ml_canvas_decorate
//Requires: caml_jsstring_of_string
function _ml_canvas_decorate(header, title) {
  var width = header.width;
  var ctxt = header.getContext("2d");
  ctxt.fillStyle = "#585858";
  ctxt.fillRect(0, 0, width, 30);
  ctxt.font = "bold 16px Arial";
  ctxt.fillStyle = "#F0F0F0F0";
  ctxt.textAlign = "center";
  ctxt.fillText(caml_jsstring_of_string(title), width / 2, 21);
  ctxt.strokeStyle = "#F0F0F0F0";
  ctxt.lineWidth = 2.0;
  ctxt.beginPath();
  ctxt.moveTo(width - 20, 10);
  ctxt.lineTo(width - 10, 20);
  ctxt.moveTo(width - 20, 20);
  ctxt.lineTo(width - 10, 10);
  ctxt.stroke();
}

//Provides: ml_canvas_create_framed
//Requires: _next_id,_header_down_handler,_surface_down_handler,_up_handler,_move_handler,_ml_canvas_decorate
function ml_canvas_create_framed(title, pos, size) {

  var x = pos[1];
  var y = pos[2];
  var width = size[1];
  var height = size[2];

  var id = ++_next_id;

  var frame = document.createElement("div");
  frame.id = "f" + id;
  frame.style.position = "absolute";
  frame.style.left = x + "px";
  frame.style.top = y + "px";

  var header = document.createElement("canvas");
  header.id = "h" + id;
  header.width = width;
  header.height = 30;
  header.style.position = "absolute";
  _ml_canvas_decorate(header, title);

  var surface = document.createElement("canvas");
  surface.id = "s" + id;
  surface.width = width;
  surface.height = height;
  surface.style.position = "absolute"
  surface.style.top = "30px";

  var ctxt = surface.getContext("2d");
  ctxt.globalAlpha = 1.0;
  ctxt.lineWidth = 2.0;
  ctxt.fillStyle = "white";
  ctxt.strokeStyle = "black";

  header.onmousedown = _header_down_handler;
  surface.onmousedown = _surface_down_handler;
  frame.oncontextmenu = function() { return false; }

  var canvas = {
    name: title,
    frame: frame,
    header: header,
    surface: surface,
    ctxt: ctxt,
    x: x, y: y,
    width: width,
    height: height,
    id: id
  };

  frame.canvas = canvas;
  header.canvas = canvas;
  surface.canvas = canvas;

  frame.appendChild(header);
  frame.appendChild(surface);
  document.body.appendChild(frame);

  return canvas;
}

//Provides: ml_canvas_create_frameless
//Requires: _next_id,_header_down_handler,_surface_down_handler,_up_handler,_move_handler
function ml_canvas_create_frameless(pos, size) {

  var x = pos[1];
  var y = pos[2];
  var width = size[1];
  var height = size[2];

  var id = ++_next_id;

  var frame = document.createElement("div");
  frame.id = "f" + id;
  frame.style.visibility = "hidden";
  frame.style.position = "absolute";
  frame.style.left = x + "px";
  frame.style.top = y + "px";

  var surface = document.createElement("canvas");
  surface.id = "s" + id;
  surface.width = width;
  surface.height = height;
  surface.style.position = "absolute"

  var ctxt = surface.getContext("2d");
  ctxt.globalAlpha = 1.0;
  ctxt.lineWidth = 2.0;
  ctxt.fillStyle = "white";
  ctxt.strokeStyle = "black";

  surface.onmousedown = _surface_down_handler;
  frame.oncontextmenu = function() { return false; }

  var canvas = {
    name: null,
    frame: frame,
    header: null,
    surface: surface,
    ctxt: ctxt,
    x: x, y: y,
    width: width,
    height: height,
    id: id
  };

  frame.canvas = canvas;
  surface.canvas = canvas;

  frame.appendChild(surface);
  document.body.appendChild(frame);

  return canvas;
}

//Provides: ml_canvas_create_offscreen
//Requires: _next_id
function ml_canvas_create_offscreen(size) {

  var width = size[1];
  var height = size[2];

  var id = ++_next_id;

  var surface = document.createElement("canvas");
  surface.id = "s" + id;
  surface.width = width;
  surface.height = height;
  surface.style.position = "absolute"

  var ctxt = surface.getContext("2d");
  ctxt.globalAlpha = 1.0;
  ctxt.lineWidth = 2.0;
  ctxt.fillStyle = "white";
  ctxt.strokeStyle = "black";

  var canvas = {
    name: null,
    frame: null,
    header: null,
    surface: surface,
    ctxt: ctxt,
    x: 0, y: 0,
    width: width,
    height: height
  };

  surface.canvas = canvas;

  return canvas;
}

//Provides: ml_canvas_create_offscreen_from_image_data
//Requires: ml_canvas_create_offscreen,caml_ba_to_typed_array,caml_ba_dim_1,caml_ba_dim_2
function ml_canvas_create_offscreen_from_image_data(data) {
  var width = caml_ba_dim_2(data);
  var height = caml_ba_dim_1(data);
  var canvas = ml_canvas_create_offscreen([ 0, width, height ])
  if (canvas === null) {
    return null;
  }
  var ta = new window.Uint8ClampedArray(caml_ba_to_typed_array(data).buffer);
  if (window.ImageData === undefined) {
    var image = canvas.ctxt.createImageData(width, height);
    image.data.set(ta);
  } else {
    var image = new window.ImageData(ta, width, height);
  }
  canvas.ctxt.putImageData(image, 0, 0);
  return canvas;
}

//Provides: ml_canvas_create_offscreen_from_png
//Requires: ml_canvas_create_offscreen,_ml_canvas_image_of_png_file
function ml_canvas_create_offscreen_from_png(filename) {
  var img = _ml_canvas_image_of_png_file(filename);
  if (img === null) {
    return null;
  }
  var canvas = ml_canvas_create_offscreen([ 0, img.width, img.height ])
  if (canvas === null) {
    return null;
  }
  canvas.ctxt.drawImage(img, 0, 0);
  return canvas;
}

// Provides: ml_canvas_destroy
function ml_canvas_destroy(canvas) {
  if (canvas.header !== null) {
    canvas.header.parentNode.removeChild(canvas.header);
  }
  if (canvas.surface !== null) {
    canvas.surface.parentNode.removeChild(canvas.surface);
  }
  if (canvas.frame !== null) {
    canvas.frame.parentNode.removeChild(canvas.frame);
  }
  canvas.name = null;
  canvas.frame = null;
  canvas.header = null;
  canvas.surface = null;
  canvas.ctxt = null;
  canvas.id = 0;
}



/* Visibility */

// Provides: ml_canvas_show
// Requires: _focus
function ml_canvas_show(canvas) {
  _focus = canvas;
  canvas.frame.style.visibility = "visible";
}

// Provides: ml_canvas_hide
// Requires: _focus
function ml_canvas_hide(canvas) {
  if (_focus === canvas) {
    _focus = null;
  }
  canvas.frame.style.visibility = "hidden";
}



/* Configuration */

// Provides: ml_canvas_get_id
function ml_canvas_get_id(canvas) {
  return canvas.id;
}

// Provides: ml_canvas_get_size
function ml_canvas_get_size(canvas) {
  return [ 0, canvas.width, canvas.height ];
}

// Provides: ml_canvas_set_size
// Requires: _ml_canvas_decorate
function ml_canvas_set_size(canvas, size) {
  var width = size[1];
  var height = size[2];
  var img = canvas.ctxt.getImageData(0, 0, canvas.width, canvas.height);
  if (canvas.header !== null) {
      canvas.header.width = width;
      _ml_canvas_decorate(canvas.header, canvas.name);
  }
  canvas.surface.width = canvas.width = width;
  canvas.surface.height = canvas.height = height;
  canvas.ctxt.fillRect(0, 0, width, height);
  canvas.ctxt.putImageData(img, 0, 0);
}

// Provides: ml_canvas_get_position
function ml_canvas_get_position(canvas) {
  return [ 0, canvas.x, canvas.y ];
}

// Provides: ml_canvas_set_position
function ml_canvas_set_position(canvas, pos) {
  var x = pos[1];
  var y = pos[2];
  if (canvas.frame !== null) {
    canvas.x = x;
    canvas.y = y;
    canvas.frame.style.left = x + "px";
    canvas.frame.style.top = y + "px";
  }
}



/* Transform */

// Provides: ml_canvas_set_transform
function ml_canvas_set_transform(canvas, t) {
  canvas.ctxt.setTransform(t[1], t[2], t[3], t[4], t[5], t[6]);
}

// Provides: ml_canvas_transform
function ml_canvas_transform(canvas, t) {
  canvas.ctxt.transform(t[1], t[2], t[3], t[4], t[5], t[6]);
}

// Provides: ml_canvas_translate
function ml_canvas_translate(canvas, vec) {
  canvas.ctxt.translate(vec[1], vec[2]);
}

// Provides: ml_canvas_scale
function ml_canvas_scale(canvas, vec) {
  canvas.ctxt.scale(vec[1], vec[2]);
}

// Provides: ml_canvas_shear
function ml_canvas_shear(canvas, vec) {
  canvas.ctxt.transform(1.0, vec[2], vec[1], 1.0, 0.0, 0.0);
}

// Provides: ml_canvas_rotate
function ml_canvas_rotate(canvas, theta) {
  canvas.ctxt.rotate(theta);
}



/* State */

// Provides: ml_canvas_save
function ml_canvas_save(canvas) {
  canvas.ctxt.save();
}

// Provides: ml_canvas_restore
function ml_canvas_restore(canvas) {
  canvas.ctxt.restore();
}



/* Style / config */

//Provides: _color_of_int
function _color_of_int(i) {
  return "#" + (i & 0x00FFFFFF).toString(16).padStart(6, '0');
}

//Provides: _int_of_color
function _int_of_color(c) {
  return parseInt(c.substr(1, c.length - 1), 16);
}

//Provides: ml_canvas_get_line_width
function ml_canvas_get_line_width(canvas) {
  return canvas.ctxt.lineWidth;
}

//Provides: ml_canvas_set_line_width
function ml_canvas_set_line_width(canvas, width) {
  canvas.ctxt.lineWidth = width;
}

//Provides: ml_canvas_get_stroke_color
//Requires: _int_of_color
function ml_canvas_get_stroke_color(canvas) {
  if (typeof(canvas.ctxt.strokeStyle) == "string")
    return _int_of_color(canvas.ctxt.strokeStyle);
  else
    return 0;
}

//Provides: ml_canvas_set_stroke_color
//Requires: _color_of_int
function ml_canvas_set_stroke_color(canvas, color) {
  canvas.ctxt.strokeStyle = _color_of_int(color);
}

//Provides: ml_canvas_set_stroke_gradient
function ml_canvas_set_stroke_gradient(canvas, grad) {
  canvas.ctxt.strokeStyle = grad;
}

//Provides: ml_canvas_set_stroke_style
//Requires: _color_of_int,STYLE_TAG
function ml_canvas_set_stroke_style(canvas, style) {
  switch(style[0])
  {
    case STYLE_TAG.COLOR:
      canvas.ctxt.strokeStyle = _color_of_int(style[1]);
      break;
    case STYLE_TAG.GRADIENT:
      canvas.ctxt.strokeStyle = style[1];
      break;
    default:
      break;
  }
}

//Provides: ml_canvas_get_stroke_style
//Requires: STYLE_TAG
function ml_canvas_get_stroke_style(canvas)
{
  var st =  canvas.ctxt.strokeStyle;
  var a = [];
  if (typeof(st) == "string")
  {
    a[0] = STYLE_TAG.COLOR;
    a[1] = st;
    return a;
  }
  else
  {
    a[0] = STYLE_TAG.GRADIENT;
    a[1] = st;
    return a;
  }
}


//Provides: ml_canvas_get_fill_color
//Requires: _int_of_color
function ml_canvas_get_fill_color(canvas) {
  if (typeof(canvas.ctxt.fillStyle) == "string")
    return _int_of_color(canvas.ctxt.fillStyle);
  else
    return 0;
}

//Provides: ml_canvas_set_fill_color
//Requires: _color_of_int
function ml_canvas_set_fill_color(canvas, color) {
  canvas.ctxt.fillStyle = _color_of_int(color);
}

//Provides: ml_canvas_get_global_alpha
function ml_canvas_get_global_alpha(canvas) {
  return canvas.ctxt.globalAlpha;
}

//Provides: ml_canvas_set_global_alpha
function ml_canvas_set_global_alpha(canvas, global_alpha) {
  canvas.ctxt.globalAlpha = global_alpha;
}

//Provides: ml_canvas_set_fill_gradient
function ml_canvas_set_fill_gradient(canvas, grad) {
  canvas.ctxt.fillStyle = grad;
}

//Provides: ml_canvas_set_fill_style
//Requires: _color_of_int,STYLE_TAG
function ml_canvas_set_fill_style(canvas, style) {
  switch(style[0])
  {
    case STYLE_TAG.COLOR:
      canvas.ctxt.fillStyle = _color_of_int(style[1]);
      break;
    case STYLE_TAG.GRADIENT:
      canvas.ctxt.fillStyle = style[1];
      break;
    default:
      break;
  }
}

//Provides: ml_canvas_get_fill_style
function ml_canvas_get_fill_style(canvas)
{
  var st =  canvas.ctxt.fillStyle;
  var a = [];
  if (typeof(st) == "string")
  {
    a[0] = 0;
    a[1] = st;
    return a;
  }
  else
  {
    a[0] = 1;
    a[1] = st;
    return a;
  }
}

//Provides: ml_canvas_set_font
// Requires: Slant_val
function ml_canvas_set_font(canvas, family, size, slant, weight) {
  canvas.ctxt.font =
    Slant_val(slant) + " " + weight + " " + size + "pt " + family;
}



/* Paths */

//Provides: ml_canvas_clear_path
function ml_canvas_clear_path(canvas) {
  canvas.ctxt.beginPath();
}

//Provides: ml_canvas_close_path
function ml_canvas_close_path(canvas) {
  canvas.ctxt.closePath();
}

//Provides: ml_canvas_move_to
function ml_canvas_move_to(canvas, p) {
  canvas.ctxt.moveTo(p[1], p[2]);
}

//Provides: ml_canvas_line_to
function ml_canvas_line_to(canvas, p) {
  canvas.ctxt.lineTo(p[1], p[2]);
}

//Provides: ml_canvas_arc
function ml_canvas_arc(canvas, p, radius, theta1, theta2, ccw) {
  canvas.ctxt.arc(p[1], p[2], radius, theta1, theta2, ccw);
}

//Provides: ml_canvas_arc_to
function ml_canvas_arc_to(canvas, p1, p2, radius) {
  canvas.ctxt.arcTo(p1[1], p1[2], p2[1], p2[2], radius);
}

//Provides: ml_canvas_quadratic_curve_to
function ml_canvas_quadratic_curve_to(canvas, p1, p2) {
  canvas.ctxt.quadraticCurveTo(p1[1], p1[2], p2[1], p2[2]);
}

//Provides: ml_canvas_bezier_curve_to
function ml_canvas_bezier_curve_to(canvas, p1, p2, p3) {
  canvas.ctxt.bezierCurveTo(p1[1], p1[2], p2[1], p2[2], p3[1], p3[2]);
}

//Provides: ml_canvas_rect
function ml_canvas_rect(canvas, pos, size) {
  canvas.ctxt.rect(pos[1], pos[2], size[1], size[2]);
}

//Provides: ml_canvas_ellipse
function ml_canvas_ellipse(canvas, p, radius, rotation, theta1, theta2, ccw) {
  canvas.ctxt.ellipse(p[1], p[2], radius[1], radius[2],
                      rotation, theta1, theta2, ccw);
}



/* Path stroking/filling */

//Provides: ml_canvas_fill
function ml_canvas_fill(canvas, nonzero) {
  if (nonzero) {
    canvas.ctxt.fill("nonzero");
  } else {
    canvas.ctxt.fill(); // "evenodd"
  }
}

//Provides: ml_canvas_stroke
function ml_canvas_stroke(canvas) {
  canvas.ctxt.stroke();
}



/* Immediate drawing */

//Provides: ml_canvas_fill_rect
function ml_canvas_fill_rect(canvas, pos, size) {
  canvas.ctxt.fillRect(pos[1], pos[2], size[1], size[2]);
}

//Provides: ml_canvas_stroke_rect
function ml_canvas_stroke_rect(canvas, pos, size) {
  canvas.ctxt.strokeRect(pos[1], pos[2], size[1], size[2]);
}

/*
//Provides: init_font_avail
function init_font_avail() {
  var width;
  var body = document.body;

  var container = document.createElement('span');
  container.innerHTML = Array(100).join('wi');
  container.style.cssText = [
    'position:absolute',
    'width:auto',
    'font-size:128px',
    'left:-99999px'
  ].join(' !important;');

  var getWidth = function (fontFamily) {
    container.style.fontFamily = fontFamily;

    body.appendChild(container);
    width = container.clientWidth;
    body.removeChild(container);

    return width;
  };

  // Pre compute the widths of monospace, serif & sans-serif
  // to improve performance.
  var monoWidth  = getWidth('monospace');
  var serifWidth = getWidth('serif');
  var sansWidth  = getWidth('sans-serif');

  window.isFontAvailable = function (font) {
    return monoWidth !== getWidth(font + ',monospace') ||
      sansWidth !== getWidth(font + ',sans-serif') ||
      serifWidth !== getWidth(font + ',serif');
  };
}
*/

//Provides: ml_canvas_fill_text
//Requires: caml_jsstring_of_string
function ml_canvas_fill_text(canvas, text, pos) {
//init_font_avail();
//window.console.log(window.isFontAvailable("ComicSansMS"));
  canvas.ctxt.fillText(caml_jsstring_of_string(text), pos[1], pos[2]);
}

//Provides: ml_canvas_stroke_text
//Requires: caml_jsstring_of_string
function ml_canvas_stroke_text(canvas, text, pos) {
//init_font_avail();
//window.console.log(window.isFontAvailable("ComicSansMS"));
  canvas.ctxt.strokeText(caml_jsstring_of_string(text), pos[1], pos[2]);
}

//Provides: ml_canvas_blit
function ml_canvas_blit(dst_canvas, dpos, src_canvas, spos, size) {
  var width = size[1];
  var height = size[2];
  dst_canvas.ctxt.drawImage(src_canvas.surface,
                            spos[1], spos[2], width, height,
                            dpos[1], dpos[2], width, height);
}


/* Direct pixel access */

//Provides: ml_canvas_get_pixel
function ml_canvas_get_pixel(canvas, pos) {
  var image = canvas.ctxt.getImageData(pos[1], pos[2], 1, 1);
  return /*image.data[3] << 24 +*/
         image.data[0] << 16 +
         image.data[1] << 8 +
         image.data[2] << 0;
}

//Provides: ml_canvas_set_pixel
function ml_canvas_set_pixel(canvas, pos, color) {
  var image = canvas.ctxt.createImageData(1, 1);
  image.data[3] = 0xFF; //(color & 0xFF000000) >> 24; // A
  image.data[0] = (color & 0x00FF0000) >> 16; // R
  image.data[1] = (color & 0x0000FF00) >> 8; // G
  image.data[2] = (color & 0x000000FF) >> 0; // B
  canvas.ctxt.putImageData(image, pos[1], pos[2]);
}

//Provides: ml_canvas_get_image_data
//Requires: caml_ba_create_unsafe
function ml_canvas_get_image_data(canvas, pos, size) {
  var width = size[1];
  var height = size[2];
  var image = canvas.ctxt.getImageData(pos[1], pos[2], width, height);
  var ta = new window.Uint8Array(image.data.buffer);
  return caml_ba_create_unsafe(3 /* Uint8Array */, 0 /* c_layout */,
                               [height, width, 4], ta);
}

//Provides: ml_canvas_set_image_data
//Requires: caml_ba_to_typed_array,caml_ba_dim_1,caml_ba_dim_2
function ml_canvas_set_image_data(canvas, dpos, data, spos, size) {
  var ta = new window.Uint8ClampedArray(caml_ba_to_typed_array(data).buffer);
  if (window.ImageData === undefined) {
    var image =
      canvas.ctxt.createImageData(caml_ba_dim_2(data), caml_ba_dim_1(data));
    image.data.set(ta);
  } else {
    var image =
      new window.ImageData(ta, caml_ba_dim_2(data), caml_ba_dim_1(data));
  }
  canvas.ctxt.putImageData(image, dpos[1], dpos[2],
                           spos[1], spos[2], size[1], size[2]);
}

//Provides: ml_canvas_export_png
//Requires: caml_create_file
function ml_canvas_export_png(canvas, filename) {
  var data = canvas.surface.toDataURL("image/png").substring(22);
  caml_create_file(filename, window.atob(data));
}

//Provides: ml_canvas_import_png
//Requires: _ml_canvas_image_of_png_file
function ml_canvas_import_png(canvas, pos, filename) {
  var img = _ml_canvas_image_of_png_file(filename);
  if (img !== null) {
    canvas.ctxt.drawImage(img, pos[1], pos[2]);
    // image, sx, sy, sWitdh, sHeight, dx, dy, dWidth, dHeight
  }
}

//Provides: _ml_canvas_image_of_png_file
//Requires: caml_read_file_content
function _ml_canvas_image_of_png_file(filename) {
  var file = caml_read_file_content(filename);
  if (file === null) {
    return null;
  }
  var data = window.btoa(caml_read_file_content(filename));
  var img = new window.Image();
  img.src = 'data:image/png;base64,' + data;
  return img;
}



/* Event */

//Provides: ml_canvas_int_of_key
//Requires: Key_code_val
function ml_canvas_int_of_key(keycode) {
  return Key_code_val(keycode);
}

//Provides: ml_canvas_key_of_int
//Requires: Val_key_code
function ml_canvas_key_of_int(keycode) {
  return Val_key_code(keycode);
}



/* Backend */

//Provides: _ml_canvas_initialized
var _ml_canvas_initialized = false;

//Provides: ml_canvas_init
//Requires: _key_down_handler,_key_up_handler,_up_handler,_move_handler,_frame_handler,_ml_canvas_initialized,caml_list_to_js_array
function ml_canvas_init(mlOptions) {
  if (_ml_canvas_initialized) {
    return false;
  }

  var backends = caml_list_to_js_array(mlOptions[1]); /* options.js_backends */

  for (var i = 0; (i < backends.length) && (!_ml_canvas_initialized); ++i) {
    switch (backends[i]) {
      case 0: /* Canvas */
        document.onkeydown = _key_down_handler;
        document.onkeyup = _key_up_handler;
        document.onmouseup = _up_handler;
        document.onmousemove = _move_handler;
        window.requestAnimationFrame(_frame_handler);
        _ml_canvas_initialized = true;
        break;
      default: window.console.log("Invalid backend specified"); break;
    };
  }

  return _ml_canvas_initialized;
}

//Provides: _ml_canvas_mlEventListener
var _ml_canvas_mlEventListener = null;

//Provides: _ml_canvas_mlContinuation
var _ml_canvas_mlContinuation = null;

//Provides: ml_canvas_run
//Requires: _ml_canvas_mlEventListener,_ml_canvas_mlContinuation
function ml_canvas_run(mlEventListener, mlContinuation) {
  if (_ml_canvas_mlEventListener !== null) {
    return;
  }
  _ml_canvas_mlEventListener = mlEventListener;
  _ml_canvas_mlContinuation = mlContinuation;
}

//Provides: ml_canvas_stop
//Requires: _ml_canvas_mlEventListener,_ml_canvas_mlContinuation
function ml_canvas_stop() {
  _ml_canvas_mlEventListener = null;
  if (_ml_canvas_mlContinuation !== null) {
    _ml_canvas_mlContinuation();
    _ml_canvas_mlContinuation = null;
  }
}
