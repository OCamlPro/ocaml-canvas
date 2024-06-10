/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/



/* Utility functions */

//Provides: _ml_canvas_initialized
var _ml_canvas_initialized = false;

//Provides: _ml_canvas_ensure_initialized
//Requires: _ml_canvas_initialized
//Requires: caml_named_value, caml_raise_constant
function _ml_canvas_ensure_initialized() {
  if (_ml_canvas_initialized == false) {
    caml_raise_constant(caml_named_value("Not_initialized"));
  }
}

//Provides: _ml_canvas_valid_canvas_size
function _ml_canvas_valid_canvas_size(width, height) {
  return 0 < width && width <= 32767 && 0 < height && height <= 32767;
}

// Provides: _internal_canvas
var _internal_canvas = document.createElement("canvas");

// Provides: _internal_ctxt
// Requires: _internal_canvas
var _internal_ctxt = _internal_canvas.getContext("2d");



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

//Provides: _resize
//Requires: _resize_handler
var _resize = new window.ResizeObserver(_resize_handler);

//Provides: _make_key_event
//Requires: _focus, keyname_to_keycode, Val_key_code, Val_key_state, EVENT_TAG
//Requires: caml_int64_of_float
function _make_key_event(e, state) {
  var char = (e.key.length === 1) ? e.key.charCodeAt(0) : 0;
  var flags = [0, e.shiftKey, e.altKey, e.ctrlKey, e.metaKey,
                  e.getModifierState("CapsLock"),
                  e.getModifierState("NumLock"),
                  (e.key === "Dead")];
  var evt = [EVENT_TAG.KEY_ACTION,
             [0, _focus, caml_int64_of_float(e.timeStamp * 1000.0),
              Val_key_code(keyname_to_keycode(e.code)),
              char, flags, Val_key_state(state)]];
  return evt;
}

//Provides: _key_down_handler
//Requires: _focus, _make_key_event, _ml_canvas_process_event, KEY_STATE
function _key_down_handler(e) {
  if (_focus !== null) {
    var evt = _make_key_event(e, KEY_STATE.DOWN);
    _ml_canvas_process_event(evt);
  }
  return false;
}

//Provides: _key_up_handler
//Requires: _focus, _make_key_event, _ml_canvas_process_event, KEY_STATE
function _key_up_handler(e) {
  if (_focus !== null) {
    var evt = _make_key_event(e, KEY_STATE.UP);
    _ml_canvas_process_event(evt);
  }
  return false;
}

//Provides: _header_down_handler
//Requires: _focus, _move, ml_canvas_close, _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
function _header_down_handler(e) {
  if (e.target !== null) {
    _focus = e.target.canvas;
    if ((e.offsetX >= e.target.canvas.width - 20) &&
        (e.offsetX <= e.target.canvas.width - 10) &&
        (e.offsetY >= 10) && (e.offsetY <= 20)) {
      var evt = [EVENT_TAG.CANVAS_CLOSED,
                 [0, e.target.canvas,
                  caml_int64_of_float(e.timeStamp * 1000.0)]];
      _ml_canvas_process_event(evt);
      ml_canvas_close(e.target.canvas);
      _focus = null;
      return false;
    }
    _move.moving = true;
    _move.target = e.target.canvas.frame;
    _move.prev_x = e.pageX;
    _move.prev_y = e.pageY;
    e.target.canvas.target.insertBefore(_move.target, null);
  }
  return false;
}

//Provides: _surface_down_handler
//Requires: _focus, _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
function _surface_down_handler(e) {
  if (e.target !== null) {
    _focus = e.target.canvas;
    e.target.canvas.target.insertBefore(e.target.canvas.frame, null);
    var evt = [EVENT_TAG.BUTTON_ACTION,
               [0, e.target.canvas,
                caml_int64_of_float(e.timeStamp * 1000.0),
                [0, e.offsetX, e.offsetY], e.button + 1, 1]];
    _ml_canvas_process_event(evt);
  }
  return false;
}

//Provides: _up_handler
//Requires: _move, _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
function _up_handler(e) {
  _move.moving = false;
  if (e.target.canvas !== undefined) {
    var evt = [EVENT_TAG.BUTTON_ACTION,
               [0, e.target.canvas,
                caml_int64_of_float(e.timeStamp * 1000.0),
                [0, e.offsetX, e.offsetY], e.button + 1, 0]];
    _ml_canvas_process_event(evt);
  }
  return false; // = prevent default behavior
}

//Provides: _move_handler
//Requires: _move, _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
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
  } else if (e.target.canvas !== undefined) {
    var evt = [EVENT_TAG.MOUSE_MOVE,
               [0, e.target.canvas,
                caml_int64_of_float(e.timeStamp * 1000.0),
                [0, e.offsetX, e.offsetY]]];
    _ml_canvas_process_event(evt);
  }
  return false;
}

//Provides: _resize_handler
//Requires: _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
function _resize_handler(entries) {
  entries.forEach(function (e) {
    var evt = [EVENT_TAG.CANVAS_RESIZED,
               [0, e.target.canvas,
                caml_int64_of_float(e.timeStamp * 1000.0),
                [0, e.target.clientWidth, e.target.clientHeight]]];
    _ml_canvas_process_event(evt);
  });
  return false;
}

//Provides: _frame_handler
//Requires: _ml_canvas_process_event, EVENT_TAG
//Requires: caml_int64_of_float
function _frame_handler(timestamp) {
  var surfaces = document.getElementsByTagName("canvas");
  for (var i = 0; i < surfaces.length; ++i) {
    var evt = [EVENT_TAG.FRAME,
               [0, surfaces[i].canvas,
                caml_int64_of_float(timestamp * 1000.0)]];
    _ml_canvas_process_event(evt);
  }
  window.requestAnimationFrame(_frame_handler);
}



/* Image Data (aka Pixmaps) */

//Provides: _ml_canvas_image_of_png_file
//Requires: caml_read_file_content
function _ml_canvas_image_of_png_file(filename) {
  var file = caml_read_file_content(filename);
  if (file === null) {
    return null;
  }
  var fc = caml_read_file_content(filename);
  var data = window.btoa(fc.toUft16 === undefined ? fc.c : fc);
  var img = new window.Image();
  img.loading = 'eager';
  img.decoding = 'sync';
  img.src = 'data:image/png;base64,' + data;
  return [img.decode(), img];
}

//Provides: _ml_canvas_ba_of_img
//Requires: caml_ba_create_unsafe
function _ml_canvas_ba_of_img(img) {
  var surface = document.createElement("canvas");
  surface.width = img.width;
  surface.height = img.height;
  var ctxt = surface.getContext("2d");
  ctxt.drawImage(img, 0, 0);
  var image = ctxt.getImageData(0, 0, img.width, img.height);
  var sta = new window.Uint8Array(image.data.buffer);
  var dta = new window.Uint8Array(sta.length);
  // Convert from RGBA to BGRA
  for (var i = 0; i < sta.length; i += 4) {
    dta[i+0] = sta[i+2];
    dta[i+1] = sta[i+1];
    dta[i+2] = sta[i+0];
    dta[i+3] = sta[i+3];
  }
  return caml_ba_create_unsafe(3 /* Uint8Array */, 0 /* c_layout */,
                               [img.height, img.width, 4], dta);
}

//Provides: _ml_canvas_surface_of_ba
//Requires: caml_ba_dim, caml_ba_to_typed_array
function _ml_canvas_surface_of_ba(data) {
  var surface = document.createElement("canvas");
  surface.width = caml_ba_dim(data, 1);
  surface.height = caml_ba_dim(data, 0);
  var ctxt = surface.getContext("2d");
  var sta = new window.Uint8Array(caml_ba_to_typed_array(data).buffer);
  var dta = new window.Uint8ClampedArray(sta.length);
  // Convert from BGRA to RGBA
  for (var i = 0; i < sta.length; i += 4) {
    dta[i+0] = sta[i+2];
    dta[i+1] = sta[i+1];
    dta[i+2] = sta[i+0];
    dta[i+3] = sta[i+3];
  }
  if (window.ImageData === undefined) {
    var image = ctxt.createImageData(surface.width, surface.height);
    image.data.set(dta);
  } else {
    var image = new window.ImageData(dta, surface.width, surface.height);
  }
  ctxt.putImageData(image, 0, 0);
  return surface;
}

//Provides: ml_canvas_image_data_create_from_png
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_image_of_png_file, _ml_canvas_ba_of_img
//Requires: caml_raise_with_string, caml_named_value
function ml_canvas_image_data_create_from_png(filename, onload) {
  _ml_canvas_ensure_initialized();
  var img = _ml_canvas_image_of_png_file(filename);
  if (img === null) {
    caml_raise_with_string(caml_named_value("Read_png_failed"), filename);
    return 0;
  }
  img[0].then(function (__img) {
    var ba = _ml_canvas_ba_of_img(img[1]);
    onload(ba);
    return 0;
  }, function (__err) {
    return 0;
  });
  return 0;
}

//Provides: ml_canvas_image_data_get_size
//Requires: caml_ba_dim
function ml_canvas_image_data_get_size(data) {
  return [0, caml_ba_dim(data, 1), caml_ba_dim(data, 0)];
}

//Provides: ml_canvas_image_data_fill
//Requires: caml_ba_to_typed_array
function ml_canvas_image_data_fill(data, color) {
  var ta = new window.Uint32Array(caml_ba_to_typed_array(data).buffer);
  for (var i = 0; i < ta.length; i++) {
    ta[i] = color;
  }
  return 0;
}

//Provides: _ml_canvas_adjust_blit_info
function _ml_canvas_adjust_blit_info(dwidth, dheight, dx, dy,
                                     swidth, sheight, sx, sy, width, height) {
  if (dx < 0) { sx -= dx; width  += dx; dx = 0; }
  if (dy < 0) { sy -= dy; height += dy; dy = 0; }
  if (sx < 0) { dx -= sx; width  += sx; sx = 0; }
  if (sy < 0) { dy -= sy; height += sy; sy = 0; }
  if (dx + width  > dwidth)  { width  = dwidth  - dx; }
  if (dy + height > dheight) { height = dheight - dy; }
  if (sx + width  > swidth)  { width  = swidth  - sx; }
  if (sy + height > sheight) { height = sheight - sy; }
  return [dx, dy, sx, sy, width, height];
}

//Provides: ml_canvas_image_data_sub
//Requires: _ml_canvas_valid_canvas_size, _ml_canvas_adjust_blit_info
//Requires: caml_ba_dim, caml_ba_to_typed_array, caml_ba_create_unsafe, caml_invalid_argument
function ml_canvas_image_data_sub(src_data, spos, size) {
  var dwidth = size[1];
  var dheight = size[2];
  if (!_ml_canvas_valid_canvas_size(dwidth, dheight)) {
    caml_invalid_argument("ImageData.blit: invalid dimensions");
  }
  var swidth = caml_ba_dim(src_data, 1);
  var sheight = caml_ba_dim(src_data, 0);
  var ta = new window.Uint8Array(dwidth * dheight * 4);
  var dst_data = caml_ba_create_unsafe(3 /* Uint8Array */, 0 /* c_layout */,
                                       [dheight, dwidth, 4], ta);
  var blit_info =
    _ml_canvas_adjust_blit_info(dwidth, dheight, 0, 0,
                                swidth, sheight, spos[1], spos[2],
                                dwidth, dheight);
  var dx = blit_info[0];
  var dy = blit_info[1];
  var sx = blit_info[2];
  var sy = blit_info[3];
  var width = blit_info[4];
  var height = blit_info[5];
  if ((width > 0) && (height > 0)) {
    var dta = new window.Uint32Array(caml_ba_to_typed_array(dst_data).buffer);
    var sta = new window.Uint32Array(caml_ba_to_typed_array(src_data).buffer);
    for (var y = 0; y < height; ++y) {
      for (var x = 0; x < width; ++x) {
        dta[(dy + y) * dwidth + (dx + x)] =
          sta[(sy + y) * swidth + (sx + x)];
      }
    }
  }
  return dst_data;
}

//Provides: ml_canvas_image_data_blit
//Requires: _ml_canvas_valid_canvas_size, _ml_canvas_adjust_blit_info
//Requires: caml_ba_dim, caml_ba_to_typed_array, caml_invalid_argument
function ml_canvas_image_data_blit(dst_data, dpos, src_data, spos, size) {
  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("ImageData.blit: invalid dimensions");
  }
  var dwidth = caml_ba_dim(dst_data, 1);
  var dheight = caml_ba_dim(dst_data, 0);
  var swidth = caml_ba_dim(src_data, 1);
  var sheight = caml_ba_dim(src_data, 0);
  var blit_info =
    _ml_canvas_adjust_blit_info(dwidth, dheight, dpos[1], dpos[2],
                                swidth, sheight, spos[1], spos[2],
                                width, height);
  var dx = blit_info[0];
  var dy = blit_info[1];
  var sx = blit_info[2];
  var sy = blit_info[3];
  var width = blit_info[4];
  var height = blit_info[5];
  if ((width > 0) && (height > 0)) {
    var dta = new window.Uint32Array(caml_ba_to_typed_array(dst_data).buffer);
    var sta = new window.Uint32Array(caml_ba_to_typed_array(src_data).buffer);
    for (var y = 0; y < height; ++y) {
      for (var x = 0; x < width; ++x) {
        dta[(dy + y) * dwidth + (dx + x)] =
          sta[(sy + y) * swidth + (sx + x)];
      }
    }
  }
  return 0;
}

//Provides: ml_canvas_image_data_get_pixel
//Requires: caml_ba_dim, caml_ba_to_typed_array
function ml_canvas_image_data_get_pixel(data, pos) {
  var ta = new window.Uint32Array(caml_ba_to_typed_array(data).buffer);
  return ta[pos[2] * caml_ba_dim(data, 1) + pos[1]]
}

//Provides: ml_canvas_image_data_put_pixel
//Requires: caml_ba_dim, caml_ba_to_typed_array
function ml_canvas_image_data_put_pixel(data, pos, color) {
  var ta = new window.Uint32Array(caml_ba_to_typed_array(data).buffer);
  ta[pos[2] * caml_ba_dim(data, 1) + pos[1]] = color;
  return 0;
}

//Provides: ml_canvas_image_data_import_png
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_surface_of_ba, _ml_canvas_image_of_png_file
//Requires: caml_ba_to_typed_array, caml_raise_with_string, caml_named_value
function ml_canvas_image_data_import_png(data, pos, filename, onload) {
  _ml_canvas_ensure_initialized();
  var img = _ml_canvas_image_of_png_file(filename);
  var surface = _ml_canvas_surface_of_ba(data);
  if ((img === null) || (surface === null)) {
    caml_raise_with_string(caml_named_value("Read_png_failed"), filename);
    return 0;
  }
  img[0].then(function (__img) {
    var ctxt = surface.getContext("2d");
    ctxt.drawImage(img[1], pos[1], pos[2]);
    var image = ctxt.getImageData(0, 0, surface.width, surface.height);
    var sta = new window.Uint8Array(image.data.buffer);
    var dta = caml_ba_to_typed_array(data);
    for (var i = 0; i < dta.length; i++) {
      dta[i] = sta[i];
    }
    onload(data);
    return 0;
  }, function (__err) {
    return 0;
  });
  return 0;
}

//Provides: ml_canvas_image_data_export_png
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_surface_of_ba
//Requires: caml_create_file, caml_raise_with_string, caml_named_value
function ml_canvas_image_data_export_png(data, filename) {
  _ml_canvas_ensure_initialized();
  try {
    var surface = _ml_canvas_surface_of_ba(data);
    if (surface !== null) {
      var data = surface.toDataURL("image/png").substring(22);
      caml_create_file(filename, window.atob(data));
    }
  } catch (exn) {
    caml_raise_with_string(caml_named_value("Write_png_failed"), filename);
  }
  return 0;
}



/* Path2D */

//Provides: ml_canvas_path_create
function ml_canvas_path_create() {
  return new window.Path2D();
}

//Provides: ml_canvas_path_close
function ml_canvas_path_close(path) {
  path.closePath();
  return 0;
}

//Provides: ml_canvas_path_move_to
function ml_canvas_path_move_to(path, p) {
  path.moveTo(p[1], p[2]);
  return 0;
}

//Provides: ml_canvas_path_line_to
function ml_canvas_path_line_to(path, p) {
  path.lineTo(p[1], p[2]);
  return 0;
}

//Provides: ml_canvas_path_arc
function ml_canvas_path_arc(path, p, radius, theta1, theta2, ccw) {
  path.arc(p[1], p[2], radius, theta1, theta2, ccw);
  return 0;
}

//Provides: ml_canvas_path_arc_to
function ml_canvas_path_arc_to(path, p1, p2, radius) {
  path.arcTo(p1[1], p1[2], p2[1], p2[2], radius);
  return 0;
}

//Provides: ml_canvas_path_quadratic_curve_to
function ml_canvas_path_quadratic_curve_to(path, cp, p) {
  path.quadraticCurveTo(cp[1], cp[2], p[1], p[2]);
  return 0;
}

//Provides : ml_canvas_path_bezier_curve_to
function ml_canvas_path_bezier_curve_to(path, cp1, cp2, p) {
  path.bezierCurveTo(cp1[1], cp1[2], cp2[1], cp2[2], p[1], p[2]);
  return 0;
}

//Provides: ml_canvas_path_rect
function ml_canvas_path_rect(path, pos, size) {
  path.rect(pos[1], pos[2], size[1], size[2]);
  return 0;
}

//Provides: ml_canvas_path_ellipse
function ml_canvas_path_ellipse(path, p, radius, rotation,
                                theta1, theta2, ccw) {
  path.ellipse(p[1], p[2], radius[1], radius[2],
               rotation, theta1, theta2, ccw);
  return 0;
}

//Provides: ml_canvas_path_add
function ml_canvas_path_add(path1, path2) {
  path1.addPath(path2);
  return 0;
}

//Provides: ml_canvas_path_add_transformed
function ml_canvas_path_add_transformed(path1, path2, t) {
  var m = new window.DOMMatrix([[1], t[2], t[3], t[4], t[5], t[6]]);
  path1.addPath(path2, m);
  return 0;
}



/* Gradients */

//Provides: ml_canvas_gradient_create_linear
//Requires : _internal_ctxt
function ml_canvas_gradient_create_linear(pos1, pos2) {
  return _internal_ctxt.createLinearGradient(pos1[1], pos1[2],
                                             pos2[1], pos2[2]);
}

//Provides: ml_canvas_gradient_create_radial
//Requires : _internal_ctxt
function ml_canvas_gradient_create_radial(center1, rad1, center2, rad2) {
  return _internal_ctxt.createRadialGradient(center1[1], center1[2], rad1,
                                             center2[1], center2[2], rad2);
}

//Provides: ml_canvas_gradient_create_conic
//Requires : _internal_ctxt
function ml_canvas_gradient_create_conic(center, angle) {
  return _internal_ctxt.createConicGradient(angle, center[1], center[2]);
}

//Provides: ml_canvas_gradient_add_color_stop
//Requires: _color_of_int
function ml_canvas_gradient_add_color_stop(gradient, color, pos) {
  gradient.addColorStop(pos, _color_of_int(color));
  return 0;
}



/* Patterns */

//Provides: ml_canvas_pattern_create
//Requires : Repeat_val, _ml_canvas_surface_of_ba, _internal_ctxt
function ml_canvas_pattern_create(image, repeat) {
  var img_canv = _ml_canvas_surface_of_ba(image);
  return _internal_ctxt.createPattern(img_canv, Repeat_val(repeat));
}



/* Canvas */

/* Comparison and hashing */

// Provides: ml_canvas_hash
// Requires: caml_named_value
function ml_canvas_hash(canvas) {
  var hash = caml_named_value("Hashtbl.hash");
  return hash(canvas.id);
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



/* Creation */

//Provides: _next_id
var _next_id = 0;

//Provides: _ml_canvas_decorate
//Requires: caml_jsstring_of_string
function _ml_canvas_decorate(header, minimize,
                             maximize, close, title) {
  var width = header.width;
  var ctxt = header.getContext("2d");
  ctxt.fillStyle = "#585858";
  ctxt.fillRect(0, 0, width, 30);
  if (title !== null) {
    ctxt.fillStyle = "#F0F0F0F0";
    ctxt.font = "bold 16px Arial";
    ctxt.textAlign = "center";
    ctxt.fillText(caml_jsstring_of_string(title), width / 2, 21);
  }
  ctxt.strokeStyle = "#F0F0F0F0";
  ctxt.lineWidth = 2.0;
  if (close == true) {
    ctxt.beginPath();
    ctxt.moveTo(width - 20, 10);
    ctxt.lineTo(width - 10, 20);
    ctxt.moveTo(width - 20, 20);
    ctxt.lineTo(width - 10, 10);
    ctxt.stroke();
  }
}

//Provides: ml_canvas_create_onscreen
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_valid_canvas_size, _resize, _next_id, _header_down_handler
//Requires: _surface_down_handler, _up_handler, _move_handler, _ml_canvas_decorate, Optional_bool_val, Optional_val
//Requires: caml_invalid_argument
function ml_canvas_create_onscreen(autocommit, decorated, resizeable, minimize,
                                   maximize, close, title, target, pos, size) {

  _ml_canvas_ensure_initialized();

  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.blit: invalid dimensions");
  }

  var pos = Optional_val(pos, [0, 0, 0]);
  var x = pos[1];
  var y = pos[2];

  var autocommit = Optional_bool_val(autocommit, true);
  var decorated = false; // Optional_bool_val(decorated, true);
  var resizeable = Optional_bool_val(resizeable, true);
  var minimize = Optional_bool_val(minimize, true);
  var maximize = Optional_bool_val(maximize, true);
  var close = Optional_bool_val(close, true);
  var title = Optional_val(title, null);
  var target = Optional_val(target, null);
  target = document.getElementById(target);
  if(target == null) {
    target = document.body;
  }

  var id = ++_next_id;

  var canvas = {
    name: title,
    target: target,
    frame: null,
    header: null,
    surface: null,
    ctxt: null,
    x: x, y: y,
    width: width,
    height: height,
    id: id,
    autocommit: autocommit,
    resizeable: resizeable,
    minimize: minimize,
    maximize: maximize,
    close: close
  };

  var header_height = (decorated == true ? 30 : 0);

  var frame = document.createElement("div");
  frame.id = "f" + id;
  if (resizeable == true) {
    frame.style.width = "100%";
    frame.style.height = "100%";
  } else {
    frame.style.width = width + "px";
    frame.style.height = height + header_height + "px";
  }
  frame.style.visibility = "hidden";
  frame.oncontextmenu = function() { return false; }
  frame.canvas = canvas;
  canvas.frame = frame;
  target.appendChild(frame);

  var header = null;
  if (decorated === true) {
    frame.style.border = "1px solid black";
    header = document.createElement("canvas");
    header.id = "h" + id;
    header.width = width;
    header.height = 30;
    _ml_canvas_decorate(header, minimize, maximize, close, title);
    header.onmousedown = _header_down_handler;
    header.canvas = canvas;
    canvas.header = header;
    frame.appendChild(header);
  }

  var surface = document.createElement("canvas");
  surface.id = "s" + id;
  surface.width = width;
  surface.height = height;
  surface.onmousedown = _surface_down_handler;
  surface.canvas = canvas;
  canvas.surface = surface;
  frame.appendChild(surface);

  if (resizeable === true) {
    surface.style.width = "100%";
    surface.style.height = "100%";
    _resize.observe(surface);
  }

  var ctxt = surface.getContext("2d");
  ctxt.globalAlpha = 1.0;
  ctxt.lineWidth = 1.0;
  ctxt.fillStyle = "white";
  ctxt.strokeStyle = "black";
  canvas.ctxt = ctxt;

  // Onscreen canvas are filled with white by default
  ctxt.fillRect(0, 0, width, height);

  return canvas;
}

//Provides: ml_canvas_create_offscreen
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_valid_canvas_size, _next_id
//Requires: caml_invalid_argument
function ml_canvas_create_offscreen(size) {

  _ml_canvas_ensure_initialized();

  var width = size[1];
  var height = size[2];

  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.createOffscreen: invalid dimensions");
  }

  var id = ++_next_id;

  var canvas = {
    name: null,
    frame: null,
    header: null,
    surface: surface,
    ctxt: ctxt,
    x: 0, y: 0,
    width: width,
    height: height,
    id: id,
    autocommit: false,
    resizeable: false,
    minimize: false,
    maximize: false,
    close: false
  };

  var surface = document.createElement("canvas");
  surface.id = "s" + id;
  surface.width = width;
  surface.height = height;
  surface.style.position = "absolute"
  surface.canvas = canvas;
  canvas.surface = surface;

  var ctxt = surface.getContext("2d");
  ctxt.globalAlpha = 1.0;
  ctxt.lineWidth = 2.0;
  ctxt.fillStyle = "white";
  ctxt.strokeStyle = "black";
  canvas.ctxt = ctxt;

  return canvas;
}

//Provides: ml_canvas_create_offscreen_from_image_data
//Requires: _ml_canvas_ensure_initialized, ml_canvas_create_offscreen
//Requires: caml_ba_to_typed_array, caml_ba_dim
function ml_canvas_create_offscreen_from_image_data(data) {
  _ml_canvas_ensure_initialized();
  var width = caml_ba_dim(data, 1);
  var height = caml_ba_dim(data, 0);
  var canvas = ml_canvas_create_offscreen([0, width, height])
  if (canvas === null) {
    return null;
  }
  var sta = new window.Uint8Array(caml_ba_to_typed_array(data).buffer);
  var dta = new window.Uint8ClampedArray(sta.length);
  // Convert from BGRA to RGBA
  for (var i = 0; i < sta.length; i += 4) {
    dta[i+0] = sta[i+2];
    dta[i+1] = sta[i+1];
    dta[i+2] = sta[i+0];
    dta[i+3] = sta[i+3];
  }
  if (window.ImageData === undefined) {
    var image = canvas.ctxt.createImageData(width, height);
    image.data.set(dta);
  } else {
    var image = new window.ImageData(dta, width, height);
  }
  canvas.ctxt.putImageData(image, 0, 0);
  return canvas;
}

//Provides: ml_canvas_create_offscreen_from_png
//Requires: _ml_canvas_ensure_initialized, ml_canvas_create_offscreen, _ml_canvas_image_of_png_file
//Requires: caml_raise_with_string, caml_named_value
function ml_canvas_create_offscreen_from_png(filename, onload) {
  _ml_canvas_ensure_initialized();
  var img = _ml_canvas_image_of_png_file(filename);
  if (img === null) {
    caml_raise_with_string(caml_named_value("Read_png_failed"), filename);
    return 0;
  }
  img[0].then(function (__img) {
    var canvas = ml_canvas_create_offscreen([0, img[1].width, img[1].height])
    if (canvas === null) {
      return 0;
    }
    canvas.ctxt.drawImage(img[1], 0, 0);
    onload(canvas);
    return 0;
  }, function (__err) {
    return 0;
  });
  return 0;
}



/* Visibility */

// Provides: ml_canvas_show
// Requires: _focus
function ml_canvas_show(canvas) {
  if (canvas.frame !== null) {
    _focus = canvas;
    canvas.frame.style.visibility = "visible";
  }
  return 0;
}

// Provides: ml_canvas_hide
// Requires: _focus
function ml_canvas_hide(canvas) {
  if (canvas.frame !== null) {
    if (_focus === canvas) {
      _focus = null;
    }
    canvas.frame.style.visibility = "hidden";
  }
  return 0;
}

// Provides: ml_canvas_close
// Requires: ml_canvas_hide
function ml_canvas_close(canvas) {
  if (canvas.frame !== null) {
    ml_canvas_hide(canvas);
    canvas.name = null;
    if (canvas.surface != null) {
      canvas.surface.canvas = null;
      if (canvas.surface.parentNode !== null) {
        canvas.surface.parentNode.removeChild(canvas.surface);
      }
    }
    if (canvas.header != null) {
      canvas.header.canvas = null;
      if (canvas.header.parentNode !== null) {
        canvas.header.parentNode.removeChild(canvas.header);
      }
      canvas.header = null;
    }
    if (canvas.frame !== null) {
      canvas.frame.canvas = null;
      if (canvas.frame.parentNode !== null) {
        canvas.frame.parentNode.removeChild(canvas.frame);
      }
      canvas.frame = null;
    }
  }
  return 0;
}



/* Rendering */

// Provides: ml_canvas_commit
function ml_canvas_commit(canvas) {
  return 0;
}



/* Configuration */

// Provides: ml_canvas_get_id
function ml_canvas_get_id(canvas) {
  return canvas.id;
}

// Provides: ml_canvas_get_size
function ml_canvas_get_size(canvas) {
  return [0, canvas.width, canvas.height];
}

// Provides: ml_canvas_set_size
// Requires: _ml_canvas_valid_canvas_size, _ml_canvas_decorate
// Requires: caml_invalid_argument
function ml_canvas_set_size(canvas, size) {
  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.setSize: invalid dimensions");
  }
  var img = canvas.ctxt.getImageData(0, 0, canvas.width, canvas.height);
  if (canvas.header !== null) {
      canvas.header.width = width;
      _ml_canvas_decorate(canvas.header, canvas.minimize,
                          canvas.maximize, canvas.close, canvas.name);
  }
  canvas.surface.width = canvas.width = width;
  canvas.surface.height = canvas.height = height;
  canvas.ctxt.fillRect(0, 0, width, height);
  canvas.ctxt.putImageData(img, 0, 0);
  return 0;
}

// Provides: ml_canvas_get_position
function ml_canvas_get_position(canvas) {
  if (canvas.frame !== null) {
    return [0, canvas.x, canvas.y];
  } else {
    return [0, 0, 0];
  }
}

// Provides: ml_canvas_set_position
function ml_canvas_set_position(canvas, pos) {
  if (canvas.frame !== null) {
    var x = pos[1];
    var y = pos[2];
    canvas.x = x;
    canvas.y = y;
    canvas.frame.style.left = x + "px";
    canvas.frame.style.top = y + "px";
  }
  return 0;
}



/* Transform */

// Provides: ml_canvas_set_transform
function ml_canvas_set_transform(canvas, t) {
  canvas.ctxt.setTransform(t[1], t[2], t[3], t[4], t[5], t[6]);
  return 0;
}

// Provides: ml_canvas_transform
function ml_canvas_transform(canvas, t) {
  canvas.ctxt.transform(t[1], t[2], t[3], t[4], t[5], t[6]);
  return 0;
}

// Provides: ml_canvas_translate
function ml_canvas_translate(canvas, vec) {
  canvas.ctxt.translate(vec[1], vec[2]);
  return 0;
}

// Provides: ml_canvas_scale
function ml_canvas_scale(canvas, vec) {
  canvas.ctxt.scale(vec[1], vec[2]);
  return 0;
}

// Provides: ml_canvas_shear
function ml_canvas_shear(canvas, vec) {
  canvas.ctxt.transform(1.0, vec[2], vec[1], 1.0, 0.0, 0.0);
  return 0;
}

// Provides: ml_canvas_rotate
function ml_canvas_rotate(canvas, theta) {
  canvas.ctxt.rotate(theta);
  return 0;
}



/* State */

// Provides: ml_canvas_save
function ml_canvas_save(canvas) {
  canvas.ctxt.save();
  return 0;
}

// Provides: ml_canvas_restore
function ml_canvas_restore(canvas) {
  canvas.ctxt.restore();
  return 0;
}



/* Style / config */

//Provides: _color_of_int
function _color_of_int(i) {
  var a = ((i & 0xFF000000) >>> 24);
  if (a == 255) {
    return "#" + (i & 0x00FFFFFF).toString(16).padStart(6, '0');
  }
  var b = i & 0x000000FF;
  var g = (i & 0x0000FF00) >>> 8;
  var r = (i & 0x00FF0000) >>> 16;
  return "rgba(" + r + "," + g + "," + b + "," + a / 255.0 + ")";
}

//Provides: _int_of_color
function _int_of_color(c) {
  if (c[0] == '#') {
    return parseInt(c.substr(1, c.length - 1), 16);
  } else {
    var rgba = c.replace(/^rgba?\(|\s+|\)$/g, '').split(',');
    var r = parseInt(rgba[0]);
    var g = parseInt(rgba[1]);
    var b = parseInt(rgba[2]);
    var a = parseFloat(rgba[3]) * 255;
    return b | (g << 8) | (r << 16) | (a << 24);
  }
}

//Provides: ml_canvas_get_line_width
function ml_canvas_get_line_width(canvas) {
  return canvas.ctxt.lineWidth;
}

//Provides: ml_canvas_set_line_width
function ml_canvas_set_line_width(canvas, width) {
  canvas.ctxt.lineWidth = width;
  return 0;
}

//Provides: ml_canvas_get_line_cap
//Requires: Val_cap_type
function ml_canvas_get_line_cap(canvas) {
  return Val_cap_type(canvas.lineCap);
}

//Provides: ml_canvas_set_line_cap
//Requires: Cap_type_val
function ml_canvas_set_line_cap(canvas, cap) {
  canvas.ctxt.lineCap = Cap_type_val(cap);
  return 0;
}

//Provides: ml_canvas_get_line_dash_offset
function ml_canvas_get_line_dash_offset(canvas) {
  return canvas.ctxt.lineDashOffset;
}

//Provides: ml_canvas_set_line_dash_offset
function ml_canvas_set_line_dash_offset(canvas, offset) {
  canvas.ctxt.lineDashOffset = offset;
  return 0;
}

//Provides: ml_canvas_get_line_dash
function ml_canvas_get_line_dash(canvas) {
  var arr = canvas.ctxt.getLineDash();
  arr.unshift(254); // Double array tag
  return arr;
}

//Provides: ml_canvas_set_line_dash
function ml_canvas_set_line_dash(canvas, dash) {
  dash.shift();
  canvas.ctxt.setLineDash(dash);
  return 0;
}

//Provides: ml_canvas_get_line_join
//Requires: Val_join_type
function ml_canvas_get_line_join(canvas) {
  return Val_join_type(canvas.lineJoin);
}

//Provides: ml_canvas_set_line_join
//Requires: Join_type_val
function ml_canvas_set_line_join(canvas, join) {
  canvas.ctxt.lineJoin = Join_type_val(join);
  return 0;
}

//Provides: ml_canvas_get_miter_limit
function ml_canvas_get_miter_limit(canvas) {
  return canvas.ctxt.miterLimit;
}

//Provides: ml_canvas_set_miter_limit
function ml_canvas_set_miter_limit(canvas, limit) {
  canvas.ctxt.miterLimit = limit;
  return 0;
}

//Provides: ml_canvas_get_stroke_color
//Requires: _int_of_color
function ml_canvas_get_stroke_color(canvas) {
  if (typeof(canvas.ctxt.strokeStyle) == "string") {
    return _int_of_color(canvas.ctxt.strokeStyle);
  } else {
    return 0;
  }
}

//Provides: ml_canvas_set_stroke_color
//Requires: _color_of_int
function ml_canvas_set_stroke_color(canvas, color) {
  canvas.ctxt.strokeStyle = _color_of_int(color);
  return 0;
}

//Provides: ml_canvas_set_stroke_gradient
function ml_canvas_set_stroke_gradient(canvas, grad) {
  canvas.ctxt.strokeStyle = grad;
  return 0;
}

//Provides: ml_canvas_set_stroke_pattern
function ml_canvas_set_stroke_pattern(canvas, pattern) {
  canvas.ctxt.strokeStyle = pattern;
  return 0;
}

//Provides: ml_canvas_set_stroke_style
//Requires: Val_draw_style
function ml_canvas_set_stroke_style(canvas, style) {
  canvas.ctxt.strokeStyle = Val_draw_style(style);
  return 0;
}

//Provides: ml_canvas_get_stroke_style
//Requires: Draw_style_val
function ml_canvas_get_stroke_style(canvas) {
  return Draw_style_val(canvas.ctxt.strokeStyle);
  return 0;
}

//Provides: ml_canvas_get_fill_color
//Requires: _int_of_color
function ml_canvas_get_fill_color(canvas) {
  if (typeof(canvas.ctxt.fillStyle) == "string") {
    return _int_of_color(canvas.ctxt.fillStyle);
  } else {
    return 0;
  }
}

//Provides: ml_canvas_set_fill_color
//Requires: _color_of_int
function ml_canvas_set_fill_color(canvas, color) {
  canvas.ctxt.fillStyle = _color_of_int(color);
  return 0;
}

//Provides: ml_canvas_get_global_alpha
function ml_canvas_get_global_alpha(canvas) {
  return canvas.ctxt.globalAlpha;
}

//Provides: ml_canvas_set_global_alpha
function ml_canvas_set_global_alpha(canvas, global_alpha) {
  canvas.ctxt.globalAlpha = global_alpha;
  return 0;
}

//Provides: ml_canvas_set_fill_gradient
function ml_canvas_set_fill_gradient(canvas, grad) {
  canvas.ctxt.fillStyle = grad;
  return 0;
}

//Provides: ml_canvas_set_fill_pattern
function ml_canvas_set_fill_pattern(canvas, pattern) {
  canvas.ctxt.fillStyle = pattern;
  return 0;
}

//Provides: ml_canvas_set_fill_style
//Requires: Val_draw_style
function ml_canvas_set_fill_style(canvas, style) {
  canvas.ctxt.fillStyle = Val_draw_style(style);
  return 0;
}

//Provides: ml_canvas_get_fill_style
//Requires: Draw_style_val
function ml_canvas_get_fill_style(canvas) {
  return Draw_style_val(canvas.ctxt.fillStyle);
}

//Provides: ml_canvas_get_global_composite_operation
//Requires: Val_compop
function ml_canvas_get_global_composite_operation(canvas) {
  return Val_compop(canvas.ctxt.globalCompositeOperation);
}

//Provides: ml_canvas_set_global_composite_operation
//Requires: Compop_val
function ml_canvas_set_global_composite_operation(canvas, op) {
  canvas.ctxt.globalCompositeOperation = Compop_val(op);
  return 0;
}

//Provides: ml_canvas_get_shadow_color
//Requires: _int_of_color
function ml_canvas_get_shadow_color(canvas) {
  return _int_of_color(canvas.ctxt.shadowColor);
}

//Provides: ml_canvas_set_shadow_color
//Requires: _color_of_int
function ml_canvas_set_shadow_color(canvas, color) {
  canvas.ctxt.shadowColor = _color_of_int(color);
  return 0;
}

//Provides: ml_canvas_get_shadow_blur
function ml_canvas_get_shadow_blur(canvas) {
  return canvas.ctxt.shadowBlur;
}

//Provides: ml_canvas_set_shadow_blur
function ml_canvas_set_shadow_blur(canvas, blur) {
  canvas.ctxt.shadowBlur = blur;
  return 0;
}

//Provides: ml_canvas_get_shadow_offset
function ml_canvas_get_shadow_offset(canvas, offset) {
  return [0, canvas.ctxt.shadowOffsetX, canvas.ctxt.shadowOffsetY];
}

//Provides: ml_canvas_set_shadow_offset
function ml_canvas_set_shadow_offset(canvas, offset) {
  canvas.ctxt.shadowOffsetX = offset[1];
  canvas.ctxt.shadowOffsetY = offset[2];
  return 0;
}

//Provides: ml_canvas_set_font
//Requires: Slant_val, caml_jsstring_of_string
function ml_canvas_set_font(canvas, family, size, slant, weight) {
  canvas.ctxt.font =
    Slant_val(slant) + " " + weight + " " + size + "pt " +
        caml_jsstring_of_string(family);
  return 0;
}



/* Paths */

//Provides: ml_canvas_clear_path
function ml_canvas_clear_path(canvas) {
  canvas.ctxt.beginPath();
  return 0;
}

//Provides: ml_canvas_close_path
function ml_canvas_close_path(canvas) {
  canvas.ctxt.closePath();
  return 0;
}

//Provides: ml_canvas_move_to
function ml_canvas_move_to(canvas, p) {
  canvas.ctxt.moveTo(p[1], p[2]);
  return 0;
}

//Provides: ml_canvas_line_to
function ml_canvas_line_to(canvas, p) {
  canvas.ctxt.lineTo(p[1], p[2]);
  return 0;
}

//Provides: ml_canvas_arc
function ml_canvas_arc(canvas, p, radius, theta1, theta2, ccw) {
  canvas.ctxt.arc(p[1], p[2], radius, theta1, theta2, ccw);
  return 0;
}

//Provides: ml_canvas_arc_to
function ml_canvas_arc_to(canvas, p1, p2, radius) {
  canvas.ctxt.arcTo(p1[1], p1[2], p2[1], p2[2], radius);
  return 0;
}

//Provides: ml_canvas_quadratic_curve_to
function ml_canvas_quadratic_curve_to(canvas, p1, p2) {
  canvas.ctxt.quadraticCurveTo(p1[1], p1[2], p2[1], p2[2]);
  return 0;
}

//Provides: ml_canvas_bezier_curve_to
function ml_canvas_bezier_curve_to(canvas, p1, p2, p3) {
  canvas.ctxt.bezierCurveTo(p1[1], p1[2], p2[1], p2[2], p3[1], p3[2]);
  return 0;
}

//Provides: ml_canvas_rect
function ml_canvas_rect(canvas, pos, size) {
  canvas.ctxt.rect(pos[1], pos[2], size[1], size[2]);
  return 0;
}

//Provides: ml_canvas_ellipse
function ml_canvas_ellipse(canvas, p, radius, rotation, theta1, theta2, ccw) {
  canvas.ctxt.ellipse(p[1], p[2], radius[1], radius[2],
                      rotation, theta1, theta2, ccw);
  return 0;
}


/* Path stroking/filling */

//Provides: ml_canvas_fill
function ml_canvas_fill(canvas, nonzero) {
  if (nonzero) {
    canvas.ctxt.fill("nonzero");
  } else {
    canvas.ctxt.fill(); // "evenodd"
  }
  return 0;
}

//Provides: ml_canvas_fill_path
function ml_canvas_fill_path(canvas, path, nonzero) {
  if (nonzero) {
    canvas.ctxt.fill(path, "nonzero");
  } else {
    canvas.ctxt.fill(path); // "evenodd"
  }
  return 0;
}

//Provides: ml_canvas_stroke
function ml_canvas_stroke(canvas) {
  canvas.ctxt.stroke();
  return 0;
}

//Provides: ml_canvas_stroke_path
function ml_canvas_stroke_path(canvas, path) {
  canvas.ctxt.stroke(path);
  return 0;
}

//Provides: ml_canvas_clip
function ml_canvas_clip(canvas, nonzero) {
  if (nonzero) {
    canvas.ctxt.clip("nonzero");
  } else {
    canvas.ctxt.clip(); // "evenodd"
  }
  return 0;
}

//Provides: ml_canvas_clip_path
function ml_canvas_clip_path(canvas, path, nonzero) {
  if (nonzero) {
    canvas.ctxt.clip(path, "nonzero");
  } else {
    canvas.ctxt.clip(path); // "evenodd"
  }
  return 0;
}



/* Immediate drawing */

//Provides: ml_canvas_fill_rect
function ml_canvas_fill_rect(canvas, pos, size) {
  canvas.ctxt.fillRect(pos[1], pos[2], size[1], size[2]);
  return 0;
}

//Provides: ml_canvas_stroke_rect
function ml_canvas_stroke_rect(canvas, pos, size) {
  canvas.ctxt.strokeRect(pos[1], pos[2], size[1], size[2]);
  return 0;
}


//Provides: ml_canvas_fill_text
//Requires: caml_jsstring_of_string
function ml_canvas_fill_text(canvas, text, pos) {
  canvas.ctxt.fillText(caml_jsstring_of_string(text), pos[1], pos[2]);
  return 0;
}

//Provides: ml_canvas_stroke_text
//Requires: caml_jsstring_of_string
function ml_canvas_stroke_text(canvas, text, pos) {
  canvas.ctxt.strokeText(caml_jsstring_of_string(text), pos[1], pos[2]);
  return 0;
}

//Provides: ml_canvas_blit
//Requires: _ml_canvas_valid_canvas_size
//Requires: caml_invalid_argument
function ml_canvas_blit(dst_canvas, dpos, src_canvas, spos, size) {
  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.blit: invalid dimensions");
  }
  dst_canvas.ctxt.drawImage(src_canvas.surface,
                            spos[1], spos[2], width, height,
                            dpos[1], dpos[2], width, height);
  return 0;
}


/* Direct pixel access */

//Provides: ml_canvas_get_pixel
function ml_canvas_get_pixel(canvas, pos) {
  var image = canvas.ctxt.getImageData(pos[1], pos[2], 1, 1);
  return ((image.data[3] << 24) >>> 0) |
          (image.data[0] << 16) |
          (image.data[1] << 8) |
          (image.data[2] << 0);
}

//Provides: ml_canvas_put_pixel
function ml_canvas_put_pixel(canvas, pos, color) {
  var image = canvas.ctxt.createImageData(1, 1);
  image.data[3] = (color & 0xFF000000) >>> 24; // A
  image.data[0] = (color & 0x00FF0000) >>> 16; // R
  image.data[1] = (color & 0x0000FF00) >>> 8; // G
  image.data[2] = (color & 0x000000FF) >>> 0; // B
  canvas.ctxt.putImageData(image, pos[1], pos[2]);
  return 0;
}

//Provides: ml_canvas_get_image_data
//Requires: _ml_canvas_valid_canvas_size
//Requires: caml_ba_create_unsafe, caml_invalid_argument
function ml_canvas_get_image_data(canvas, pos, size) {
  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.getImageData: invalid dimensions");
  }
  var image = canvas.ctxt.getImageData(pos[1], pos[2], width, height);
  var sta = new window.Uint8Array(image.data.buffer);
  var dta = new window.Uint8Array(sta.length);
  // Convert from RGNA to BGRA
  for (var i = 0; i < sta.length; i += 4) {
    dta[i+0] = sta[i+2];
    dta[i+1] = sta[i+1];
    dta[i+2] = sta[i+0];
    dta[i+3] = sta[i+3];
  }
  return caml_ba_create_unsafe(3 /* Uint8Array */, 0 /* c_layout */,
                               [height, width, 4], dta);
}

//Provides: ml_canvas_put_image_data
//Requires: _ml_canvas_valid_canvas_size
//Requires: caml_ba_to_typed_array, caml_ba_dim, caml_invalid_argument
function ml_canvas_put_image_data(canvas, dpos, data, spos, size) {
  var width = size[1];
  var height = size[2];
  if (!_ml_canvas_valid_canvas_size(width, height)) {
    caml_invalid_argument("Canvas.putImageData: invalid dimensions");
  }
  var sta = new window.Uint8Array(caml_ba_to_typed_array(data).buffer);
  var dta = new window.Uint8ClampedArray(sta.length);
  // Convert from BGRA to RGBA
  for (var i = 0; i < sta.length; i += 4) {
    dta[i+0] = sta[i+2];
    dta[i+1] = sta[i+1];
    dta[i+2] = sta[i+0];
    dta[i+3] = sta[i+3];
  }
  if (window.ImageData === undefined) {
    var image =
      canvas.ctxt.createImageData(caml_ba_dim(data, 1), caml_ba_dim(data, 0));
    image.data.set(dta);
  } else {
    var image =
      new window.ImageData(dta, caml_ba_dim(data, 1), caml_ba_dim(data, 0));
  }
  canvas.ctxt.putImageData(image, dpos[1], dpos[2],
                           spos[1], spos[2], width, height);
  return 0;
}

//Provides: ml_canvas_import_png
//Requires: _ml_canvas_image_of_png_file
//Requires: caml_raise_with_string, caml_named_value
function ml_canvas_import_png(canvas, pos, filename, onload) {
  //TODO: check read failure
  var img = _ml_canvas_image_of_png_file(filename);
  if (img === null) {
    caml_raise_with_string(caml_named_value("Read_png_failed"), filename);
    return 0;
  }
  img[0].then(function (__img) {
    canvas.ctxt.drawImage(img[1], pos[1], pos[2]);
    // image, sx, sy, sWitdh, sHeight, dx, dy, dWidth, dHeight
    onload(canvas);
    return 0;
  }, function (__err) {
    return 0;
  });
  return 0;
}

//Provides: ml_canvas_export_png
//Requires: caml_create_file, caml_raise_with_string, caml_named_value
function ml_canvas_export_png(canvas, filename) {
  try {
    var data = canvas.surface.toDataURL("image/png").substring(22);
    caml_create_file(filename, window.atob(data));
  } catch (exn) {
    caml_raise_with_string(caml_named_value("Write_png_failed"), filename);
  }
  return 0;
}



/* Event */

//Provides: ml_canvas_int_of_key
//Requires: Key_code_val
function ml_canvas_int_of_key(keycode) {
  return Key_code_val(keycode);
}

//Provides: ml_canvas_key_of_int
//Requires: Val_key_code
//Requires: caml_invalid_argument
function ml_canvas_key_of_int(keycode) {
  caml_invalid_argument("key_of_int: i must be in the 0-255 range");
  return Val_key_code(keycode);
}



/* Backend */

//Provides: ml_canvas_init
//Requires: _key_down_handler, _key_up_handler, _up_handler, _move_handler, _resize_handler, _frame_handler
//Requires: _ml_canvas_initialized, caml_list_to_js_array
function ml_canvas_init() {
  if (_ml_canvas_initialized === true) {
    return 0;
  }
  document.addEventListener("keydown", _key_down_handler, {passive: true});
  document.addEventListener("keyup", _key_up_handler, {passive: true});
  document.addEventListener("mouseup", _up_handler, {passive: true});
  document.addEventListener("mousemove", _move_handler, {passive: true});
  window.requestAnimationFrame(_frame_handler);
  _ml_canvas_initialized = true;
  return 0;
}

//Provides: _ml_canvas_mlProcessEvent
var _ml_canvas_mlProcessEvent = null;

//Provides: _ml_canvas_mlContinuation
var _ml_canvas_mlContinuation = null;

//Provides: _ml_canvas_mlException
var _ml_canvas_mlException = null;

//Provides: _ml_canvas_process_event
//Requires: _ml_canvas_mlProcessEvent, _ml_canvas_mlException, ml_canvas_stop
function _ml_canvas_process_event(mlEvent) {
  if (_ml_canvas_mlProcessEvent === null) {
    return false;
  }
  try {
    _ml_canvas_mlProcessEvent(mlEvent);
    return true;
  } catch (exn) {
    _ml_canvas_mlException = exn;
    ml_canvas_stop();
    return false;
  }
}

//Provides: ml_canvas_run
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_mlProcessEvent, _ml_canvas_mlContinuation, _ml_canvas_mlException, _ml_canvas_ensure_initialized
function ml_canvas_run(mlProcessEvent, mlContinuation) {
  _ml_canvas_ensure_initialized();
  if (_ml_canvas_mlProcessEvent !== null) {
    return 0;
  }
  _ml_canvas_mlProcessEvent = mlProcessEvent;
  _ml_canvas_mlContinuation = mlContinuation;
  _ml_canvas_mlException = null;
  return 0;
}

//Provides: ml_canvas_stop
//Requires: _ml_canvas_ensure_initialized, _ml_canvas_mlProcessEvent, _ml_canvas_mlContinuation, _ml_canvas_mlException, _ml_canvas_ensure_initialized
function ml_canvas_stop() {
  _ml_canvas_ensure_initialized();
  _ml_canvas_mlProcessEvent = null;
  if (_ml_canvas_mlException !== null) {
    var exn = _ml_canvas_mlException;
    _ml_canvas_mlContinuation = null;
    _ml_canvas_mlException = null;
    throw (exn);
  } else if (_ml_canvas_mlContinuation !== null) {
    try {
      _ml_canvas_mlContinuation();
      _ml_canvas_mlContinuation = null;
    } catch (exn) {
      _ml_canvas_mlContinuation = null;
      throw (exn);
    }
  }
  return 0;
}

//Provides: ml_canvas_get_canvas
//Requires: _ml_canvas_ensure_initialized
//Requires: caml_raise_not_found
function ml_canvas_get_canvas(id) {
  _ml_canvas_ensure_initialized();
  var surface = document.getElementById("s" + id);
  if (surface === null) {
    caml_raise_not_found();
    return 0;
  }
  return surface.canvas;
}

//Provides: ml_canvas_get_current_timestamp
//Requires: _ml_canvas_ensure_initialized
//Requires: caml_int64_of_float
function ml_canvas_get_current_timestamp() {
  _ml_canvas_ensure_initialized();
  var e = new window.Event("dummy");
  return caml_int64_of_float(e.timeStamp * 1000.0);
}
