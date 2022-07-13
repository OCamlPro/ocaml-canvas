/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

//Provides: Val_focus_in_out
//Requires: FOCUS,FOCUS_TAG

var focus_to_tag = new joo_global_object.Map([
  [FOCUS.OUT, FOCUS_TAG.OUT],
  [FOCUS.IN,  FOCUS_TAG.IN],
]);

function Val_focus_in_out(focus) {
  return focus_to_tag.get(focus);
}

//Provides: Val_key_code
//Requires: KEY,KEY_TAG

var kc_to_tag = new joo_global_object.Map([
  [KEY.ESCAPE,               KEY_TAG.ESCAPE],
  [KEY.F1,                   KEY_TAG.F1],
  [KEY.F2,                   KEY_TAG.F2],
  [KEY.F3,                   KEY_TAG.F3],
  [KEY.F4,                   KEY_TAG.F4],
  [KEY.F5,                   KEY_TAG.F5],
  [KEY.F6,                   KEY_TAG.F6],
  [KEY.F7,                   KEY_TAG.F7],
  [KEY.F8,                   KEY_TAG.F8],
  [KEY.F9,                   KEY_TAG.F9],
  [KEY.F10,                  KEY_TAG.F10],
  [KEY.F11,                  KEY_TAG.F11],
  [KEY.F12,                  KEY_TAG.F12],
  [KEY.PRINTSCREEN,          KEY_TAG.PRINTSCREEN],
  [KEY.SCROLLLOCK,           KEY_TAG.SCROLLLOCK],
  [KEY.PAUSE,                KEY_TAG.PAUSE],
  [KEY.GRAVE_TILDE,          KEY_TAG.GRAVE_TILDE],
  [KEY._1_EXCLAMATION,       KEY_TAG._1_EXCLAMATION],
  [KEY._2_AT,                KEY_TAG._2_AT],
  [KEY._3_NUMBER,            KEY_TAG._3_NUMBER],
  [KEY._4_DOLLAR,            KEY_TAG._4_DOLLAR],
  [KEY._5_PERCENT,           KEY_TAG._5_PERCENT],
  [KEY._6_CARET,             KEY_TAG._6_CARET],
  [KEY._7_AMPERSAND,         KEY_TAG._7_AMPERSAND],
  [KEY._8_ASTERISK,          KEY_TAG._8_ASTERISK],
  [KEY._9_LPARENTHESIS,      KEY_TAG._9_LPARENTHESIS],
  [KEY._0_RPARENTHESIS,      KEY_TAG._0_RPARENTHESIS],
  [KEY.MINUS_UNDERSCORE,     KEY_TAG.MINUS_UNDERSCORE],
  [KEY.EQUAL_PLUS,           KEY_TAG.EQUAL_PLUS],
  [KEY.BACKSPACE,            KEY_TAG.BACKSPACE],
  [KEY.TAB,                  KEY_TAG.TAB],
  [KEY.Q,                    KEY_TAG.Q],
  [KEY.W,                    KEY_TAG.W],
  [KEY.E,                    KEY_TAG.E],
  [KEY.R,                    KEY_TAG.R],
  [KEY.T,                    KEY_TAG.T],
  [KEY.Y,                    KEY_TAG.Y],
  [KEY.U,                    KEY_TAG.U],
  [KEY.I,                    KEY_TAG.I],
  [KEY.O,                    KEY_TAG.O],
  [KEY.P,                    KEY_TAG.P],
  [KEY.LBRACKET_CURLY,       KEY_TAG.LBRACKET_CURLY],
  [KEY.RBRACKET_CURLY,       KEY_TAG.RBRACKET_CURLY],
  [KEY.BACKSLASH_PIPE,       KEY_TAG.BACKSLASH_PIPE],
  [KEY.CAPSLOCK,             KEY_TAG.CAPSLOCK],
  [KEY.A,                    KEY_TAG.A],
  [KEY.S,                    KEY_TAG.S],
  [KEY.D,                    KEY_TAG.D],
  [KEY.F,                    KEY_TAG.F],
  [KEY.G,                    KEY_TAG.G],
  [KEY.H,                    KEY_TAG.H],
  [KEY.J,                    KEY_TAG.J],
  [KEY.K,                    KEY_TAG.K],
  [KEY.L,                    KEY_TAG.L],
  [KEY.SEMICOLON_COLON,      KEY_TAG.SEMICOLON_COLON],
  [KEY.QUOTE_DOUBLEQUOTE,    KEY_TAG.QUOTE_DOUBLEQUOTE],
  [KEY.NONUS_NUMBER_TILDE,   KEY_TAG.NONUS_NUMBER_TILDE],
  [KEY.RETURN,               KEY_TAG.RETURN],
  [KEY.LSHIFT,               KEY_TAG.LSHIFT],
  [KEY.NONUS_BACKSLASH_PIPE, KEY_TAG.NONUS_BACKSLASH_PIPE],
  [KEY.Z,                    KEY_TAG.Z],
  [KEY.X,                    KEY_TAG.X],
  [KEY.C,                    KEY_TAG.C],
  [KEY.V,                    KEY_TAG.V],
  [KEY.B,                    KEY_TAG.B],
  [KEY.N,                    KEY_TAG.N],
  [KEY.M,                    KEY_TAG.M],
  [KEY.COMMA_LESS,           KEY_TAG.COMMA_LESS],
  [KEY.PERIOD_GREATER,       KEY_TAG.PERIOD_GREATER],
  [KEY.SLASH_QUESTION,       KEY_TAG.SLASH_QUESTION],
  [KEY.RSHIFT,               KEY_TAG.RSHIFT],
  [KEY.LCONTROL,             KEY_TAG.LCONTROL],
  [KEY.LMETA,                KEY_TAG.LMETA],
  [KEY.LALT,                 KEY_TAG.LALT],
  [KEY.SPACEBAR,             KEY_TAG.SPACEBAR],
  [KEY.RALT,                 KEY_TAG.RALT],
  [KEY.RMETA,                KEY_TAG.RMETA],
  [KEY.MENU,                 KEY_TAG.MENU],
  [KEY.RCONTROL,             KEY_TAG.RCONTROL],
  [KEY.INSERT,               KEY_TAG.INSERT],
  [KEY.HOME,                 KEY_TAG.HOME],
  [KEY.PAGEUP,               KEY_TAG.PAGEUP],
  [KEY.DELETEFORWARD,        KEY_TAG.DELETEFORWARD],
  [KEY.END,                  KEY_TAG.END],
  [KEY.PAGEDOWN,             KEY_TAG.PAGEDOWN],
  [KEY.UPARROW,              KEY_TAG.UPARROW],
  [KEY.LEFTARROW,            KEY_TAG.LEFTARROW],
  [KEY.DOWNARROW,            KEY_TAG.DOWNARROW],
  [KEY.RIGHTARROW,           KEY_TAG.RIGHTARROW],
  [KEY.PAD_NUMLOCK_CLEAR,    KEY_TAG.PAD_NUMLOCK_CLEAR],
  [KEY.PAD_EQUALS,           KEY_TAG.PAD_EQUALS],
  [KEY.PAD_DIVIDE,           KEY_TAG.PAD_DIVIDE],
  [KEY.PAD_MULTIPLY,         KEY_TAG.PAD_MULTIPLY],
  [KEY.PAD_MINUS,            KEY_TAG.PAD_MINUS],
  [KEY.PAD_7_HOME,           KEY_TAG.PAD_7_HOME],
  [KEY.PAD_8_UPARROW,        KEY_TAG.PAD_8_UPARROW],
  [KEY.PAD_9_PAGEUP,         KEY_TAG.PAD_9_PAGEUP],
  [KEY.PAD_PLUS,             KEY_TAG.PAD_PLUS],
  [KEY.PAD_4_LEFTARROW,      KEY_TAG.PAD_4_LEFTARROW],
  [KEY.PAD_5,                KEY_TAG.PAD_5],
  [KEY.PAD_6_RIGHTARROW,     KEY_TAG.PAD_6_RIGHTARROW],
  [KEY.PAD_COMMA,            KEY_TAG.PAD_COMMA],
  [KEY.PAD_1_END,            KEY_TAG.PAD_1_END],
  [KEY.PAD_2_DOWNARROW,      KEY_TAG.PAD_2_DOWNARROW],
  [KEY.PAD_3_PAGEDOWN,       KEY_TAG.PAD_3_PAGEDOWN],
  [KEY.PAD_0_INSERT,         KEY_TAG.PAD_0_INSERT],
  [KEY.PAD_DECIMAL_DELETE,   KEY_TAG.PAD_DECIMAL_DELETE],
  [KEY.PAD_ENTER,            KEY_TAG.PAD_ENTER],
  [KEY.F13,                  KEY_TAG.F13],
  [KEY.F14,                  KEY_TAG.F14],
  [KEY.F15,                  KEY_TAG.F15],
  [KEY.F16,                  KEY_TAG.F16],
  [KEY.F17,                  KEY_TAG.F17],
  [KEY.F18,                  KEY_TAG.F18],
  [KEY.F19,                  KEY_TAG.F19],
  [KEY.F20,                  KEY_TAG.F20],
  [KEY.F21,                  KEY_TAG.F21],
  [KEY.F22,                  KEY_TAG.F22],
  [KEY.F23,                  KEY_TAG.F23],
  [KEY.F24,                  KEY_TAG.F24],
  [KEY.INTERNATIONAL1,       KEY_TAG.INTERNATIONAL1],
  [KEY.INTERNATIONAL2,       KEY_TAG.INTERNATIONAL2],
  [KEY.INTERNATIONAL3,       KEY_TAG.INTERNATIONAL3],
  [KEY.INTERNATIONAL4,       KEY_TAG.INTERNATIONAL4],
  [KEY.INTERNATIONAL5,       KEY_TAG.INTERNATIONAL5],
  [KEY.INTERNATIONAL6,       KEY_TAG.INTERNATIONAL6],
  [KEY.INTERNATIONAL7,       KEY_TAG.INTERNATIONAL7],
  [KEY.INTERNATIONAL8,       KEY_TAG.INTERNATIONAL8],
  [KEY.INTERNATIONAL9,       KEY_TAG.INTERNATIONAL9],
  [KEY.LANG1,                KEY_TAG.LANG1],
  [KEY.LANG2,                KEY_TAG.LANG2],
  [KEY.LANG3,                KEY_TAG.LANG3],
  [KEY.LANG4,                KEY_TAG.LANG4],
  [KEY.LANG5,                KEY_TAG.LANG5],
  [KEY.HELP,                 KEY_TAG.HELP],
  [KEY.MUTE,                 KEY_TAG.MUTE],
  [KEY.VOLUMEUP,             KEY_TAG.VOLUMEUP],
  [KEY.VOLUMEDOWN,           KEY_TAG.VOLUMEDOWN],
]);

function Val_key_code(keycode) {
  return kc_to_tag.get(keycode);
}

//Provides: Key_code_val
//Requires: KEY,KEY_TAG

var tag_to_kc = new joo_global_object.Map([
  [KEY_TAG.ESCAPE,               KEY.ESCAPE],
  [KEY_TAG.F1,                   KEY.F1],
  [KEY_TAG.F2,                   KEY.F2],
  [KEY_TAG.F3,                   KEY.F3],
  [KEY_TAG.F4,                   KEY.F4],
  [KEY_TAG.F5,                   KEY.F5],
  [KEY_TAG.F6,                   KEY.F6],
  [KEY_TAG.F7,                   KEY.F7],
  [KEY_TAG.F8,                   KEY.F8],
  [KEY_TAG.F9,                   KEY.F9],
  [KEY_TAG.F10,                  KEY.F10],
  [KEY_TAG.F11,                  KEY.F11],
  [KEY_TAG.F12,                  KEY.F12],
  [KEY_TAG.PRINTSCREEN,          KEY.PRINTSCREEN],
  [KEY_TAG.SCROLLLOCK,           KEY.SCROLLLOCK],
  [KEY_TAG.PAUSE,                KEY.PAUSE],
  [KEY_TAG.GRAVE_TILDE,          KEY.GRAVE_TILDE],
  [KEY_TAG._1_EXCLAMATION,       KEY._1_EXCLAMATION],
  [KEY_TAG._2_AT,                KEY._2_AT],
  [KEY_TAG._3_NUMBER,            KEY._3_NUMBER],
  [KEY_TAG._4_DOLLAR,            KEY._4_DOLLAR],
  [KEY_TAG._5_PERCENT,           KEY._5_PERCENT],
  [KEY_TAG._6_CARET,             KEY._6_CARET],
  [KEY_TAG._7_AMPERSAND,         KEY._7_AMPERSAND],
  [KEY_TAG._8_ASTERISK,          KEY._8_ASTERISK],
  [KEY_TAG._9_LPARENTHESIS,      KEY._9_LPARENTHESIS],
  [KEY_TAG._0_RPARENTHESIS,      KEY._0_RPARENTHESIS],
  [KEY_TAG.MINUS_UNDERSCORE,     KEY.MINUS_UNDERSCORE],
  [KEY_TAG.EQUAL_PLUS,           KEY.EQUAL_PLUS],
  [KEY_TAG.BACKSPACE,            KEY.BACKSPACE],
  [KEY_TAG.TAB,                  KEY.TAB],
  [KEY_TAG.Q,                    KEY.Q],
  [KEY_TAG.W,                    KEY.W],
  [KEY_TAG.E,                    KEY.E],
  [KEY_TAG.R,                    KEY.R],
  [KEY_TAG.T,                    KEY.T],
  [KEY_TAG.Y,                    KEY.Y],
  [KEY_TAG.U,                    KEY.U],
  [KEY_TAG.I,                    KEY.I],
  [KEY_TAG.O,                    KEY.O],
  [KEY_TAG.P,                    KEY.P],
  [KEY_TAG.LBRACKET_CURLY,       KEY.LBRACKET_CURLY],
  [KEY_TAG.RBRACKET_CURLY,       KEY.RBRACKET_CURLY],
  [KEY_TAG.BACKSLASH_PIPE,       KEY.BACKSLASH_PIPE],
  [KEY_TAG.CAPSLOCK,             KEY.CAPSLOCK],
  [KEY_TAG.A,                    KEY.A],
  [KEY_TAG.S,                    KEY.S],
  [KEY_TAG.D,                    KEY.D],
  [KEY_TAG.F,                    KEY.F],
  [KEY_TAG.G,                    KEY.G],
  [KEY_TAG.H,                    KEY.H],
  [KEY_TAG.J,                    KEY.J],
  [KEY_TAG.K,                    KEY.K],
  [KEY_TAG.L,                    KEY.L],
  [KEY_TAG.SEMICOLON_COLON,      KEY.SEMICOLON_COLON],
  [KEY_TAG.QUOTE_DOUBLEQUOTE,    KEY.QUOTE_DOUBLEQUOTE],
  [KEY_TAG.NONUS_NUMBER_TILDE,   KEY.NONUS_NUMBER_TILDE],
  [KEY_TAG.RETURN,               KEY.RETURN],
  [KEY_TAG.LSHIFT,               KEY.LSHIFT],
  [KEY_TAG.NONUS_BACKSLASH_PIPE, KEY.NONUS_BACKSLASH_PIPE],
  [KEY_TAG.Z,                    KEY.Z],
  [KEY_TAG.X,                    KEY.X],
  [KEY_TAG.C,                    KEY.C],
  [KEY_TAG.V,                    KEY.V],
  [KEY_TAG.B,                    KEY.B],
  [KEY_TAG.N,                    KEY.N],
  [KEY_TAG.M,                    KEY.M],
  [KEY_TAG.COMMA_LESS,           KEY.COMMA_LESS],
  [KEY_TAG.PERIOD_GREATER,       KEY.PERIOD_GREATER],
  [KEY_TAG.SLASH_QUESTION,       KEY.SLASH_QUESTION],
  [KEY_TAG.RSHIFT,               KEY.RSHIFT],
  [KEY_TAG.LCONTROL,             KEY.LCONTROL],
  [KEY_TAG.LMETA,                KEY.LMETA],
  [KEY_TAG.LALT,                 KEY.LALT],
  [KEY_TAG.SPACEBAR,             KEY.SPACEBAR],
  [KEY_TAG.RALT,                 KEY.RALT],
  [KEY_TAG.RMETA,                KEY.RMETA],
  [KEY_TAG.MENU,                 KEY.MENU],
  [KEY_TAG.RCONTROL,             KEY.RCONTROL],
  [KEY_TAG.INSERT,               KEY.INSERT],
  [KEY_TAG.HOME,                 KEY.HOME],
  [KEY_TAG.PAGEUP,               KEY.PAGEUP],
  [KEY_TAG.DELETEFORWARD,        KEY.DELETEFORWARD],
  [KEY_TAG.END,                  KEY.END],
  [KEY_TAG.PAGEDOWN,             KEY.PAGEDOWN],
  [KEY_TAG.UPARROW,              KEY.UPARROW],
  [KEY_TAG.LEFTARROW,            KEY.LEFTARROW],
  [KEY_TAG.DOWNARROW,            KEY.DOWNARROW],
  [KEY_TAG.RIGHTARROW,           KEY.RIGHTARROW],
  [KEY_TAG.PAD_NUMLOCK_CLEAR,    KEY.PAD_NUMLOCK_CLEAR],
  [KEY_TAG.PAD_EQUALS,           KEY.PAD_EQUALS],
  [KEY_TAG.PAD_DIVIDE,           KEY.PAD_DIVIDE],
  [KEY_TAG.PAD_MULTIPLY,         KEY.PAD_MULTIPLY],
  [KEY_TAG.PAD_MINUS,            KEY.PAD_MINUS],
  [KEY_TAG.PAD_7_HOME,           KEY.PAD_7_HOME],
  [KEY_TAG.PAD_8_UPARROW,        KEY.PAD_8_UPARROW],
  [KEY_TAG.PAD_9_PAGEUP,         KEY.PAD_9_PAGEUP],
  [KEY_TAG.PAD_PLUS,             KEY.PAD_PLUS],
  [KEY_TAG.PAD_4_LEFTARROW,      KEY.PAD_4_LEFTARROW],
  [KEY_TAG.PAD_5,                KEY.PAD_5],
  [KEY_TAG.PAD_6_RIGHTARROW,     KEY.PAD_6_RIGHTARROW],
  [KEY_TAG.PAD_COMMA,            KEY.PAD_COMMA],
  [KEY_TAG.PAD_1_END,            KEY.PAD_1_END],
  [KEY_TAG.PAD_2_DOWNARROW,      KEY.PAD_2_DOWNARROW],
  [KEY_TAG.PAD_3_PAGEDOWN,       KEY.PAD_3_PAGEDOWN],
  [KEY_TAG.PAD_0_INSERT,         KEY.PAD_0_INSERT],
  [KEY_TAG.PAD_DECIMAL_DELETE,   KEY.PAD_DECIMAL_DELETE],
  [KEY_TAG.PAD_ENTER,            KEY.PAD_ENTER],
  [KEY_TAG.F13,                  KEY.F13],
  [KEY_TAG.F14,                  KEY.F14],
  [KEY_TAG.F15,                  KEY.F15],
  [KEY_TAG.F16,                  KEY.F16],
  [KEY_TAG.F17,                  KEY.F17],
  [KEY_TAG.F18,                  KEY.F18],
  [KEY_TAG.F19,                  KEY.F19],
  [KEY_TAG.F20,                  KEY.F20],
  [KEY_TAG.F21,                  KEY.F21],
  [KEY_TAG.F22,                  KEY.F22],
  [KEY_TAG.F23,                  KEY.F23],
  [KEY_TAG.F24,                  KEY.F24],
  [KEY_TAG.INTERNATIONAL1,       KEY.INTERNATIONAL1],
  [KEY_TAG.INTERNATIONAL2,       KEY.INTERNATIONAL2],
  [KEY_TAG.INTERNATIONAL3,       KEY.INTERNATIONAL3],
  [KEY_TAG.INTERNATIONAL4,       KEY.INTERNATIONAL4],
  [KEY_TAG.INTERNATIONAL5,       KEY.INTERNATIONAL5],
  [KEY_TAG.INTERNATIONAL6,       KEY.INTERNATIONAL6],
  [KEY_TAG.INTERNATIONAL7,       KEY.INTERNATIONAL7],
  [KEY_TAG.INTERNATIONAL8,       KEY.INTERNATIONAL8],
  [KEY_TAG.INTERNATIONAL9,       KEY.INTERNATIONAL9],
  [KEY_TAG.LANG1,                KEY.LANG1],
  [KEY_TAG.LANG2,                KEY.LANG2],
  [KEY_TAG.LANG3,                KEY.LANG3],
  [KEY_TAG.LANG4,                KEY.LANG4],
  [KEY_TAG.LANG5,                KEY.LANG5],
  [KEY_TAG.HELP,                 KEY.HELP],
  [KEY_TAG.MUTE,                 KEY.MUTE],
  [KEY_TAG.VOLUMEUP,             KEY.VOLUMEUP],
  [KEY_TAG.VOLUMEDOWN,           KEY.VOLUMEDOWN],
]);

function Key_code_val(keycode) {
  return tag_to_kc.get(keycode);
}

//Provides: Val_key_state
//Requires: KEY_STATE,KEY_STATE_TAG

var key_state_to_tag = new joo_global_object.Map([
  [KEY_STATE.UP,   KEY_STATE_TAG.UP],
  [KEY_STATE.DOWN, KEY_STATE_TAG.DOWN],
]);

function Val_key_state(state) {
  return key_state_to_tag.get(state);
}

//Provides: Val_button
//Requires: BUTTON,BUTTON_TAG

var button_to_tag = new joo_global_object.Map([
  [BUTTON.NONE,       BUTTON_TAG.NONE],
  [BUTTON.LEFT,       BUTTON_TAG.LEFT],
  [BUTTON.MIDDLE,     BUTTON_TAG.MIDDLE],
  [BUTTON.MIDDLE,     BUTTON_TAG.RIGHT],
  [BUTTON.WHEEL_UP,   BUTTON_TAG.WHEEL_UP],
  [BUTTON.WHEEL_DOWN, BUTTON_TAG.WHEEL_DOWN],
]);

function Val_button(button) {
  return button_to_tag.get(button);
}

//Provides: Val_button_state
//Requires: BUTTON_STATE,BUTTON_STATE_TAG

var button_state_to_tag = new joo_global_object.Map([
  [BUTTON_STATE.UP,   BUTTON_STATE_TAG.UP],
  [BUTTON_STATE.DOWN, BUTTON_STATE_TAG.DOWN],
]);

function Val_button_state(state) {
  return button_state_to_tag.get(state);
}

//Provides: Val_repeat
//Requires: REPEAT_TAG

var repeat_to_tag = new joo_global_object.Map([
  ["no-repeat", REPEAT_TAG.NO_REPEAT],
  ["repeat-x",  REPEAT_TAG.REPEAT_X],
  ["repeat-y",  REPEAT_TAG.REPEAT_Y],
  ["repeat-xy", REPEAT_TAG.REPEAT_XY],
]);

function Val_repeat(repeat) {
  return repeat_to_tag.get(repeat);
}

//Provides: Repeat_val
//Requires: REPEAT_TAG

var tag_to_repeat = new joo_global_object.Map([
  [REPEAT_TAG.NO_REPEAT, "no-repeat"],
  [REPEAT_TAG.REPEAT_X,  "repeat-x"],
  [REPEAT_TAG.REPEAT_Y,  "repeat-y"],
  [REPEAT_TAG.REPEAT_XY, "repeat-xy"],
]);

function Repeat_val(repeat) {
  return tag_to_repeat.get(repeat);
}

//Provides: Val_draw_style
//Requires: _color_of_int,STYLE_TAG
function Val_draw_style(style) {
  var s = 0;
  switch (style[0]) {
    case STYLE_TAG.COLOR:
      s = _color_of_int(style[1]);
      break;
    case STYLE_TAG.GRADIENT:
    case STYLE_TAG.PATTERN:
      s = style[1];
      break;
    default:
      break;
  }
  return s;
}

//Provides: Draw_style_val
//Requires: STYLE_TAG
function Draw_style_val(style) {
  var s = [];
  s[1] = style;
  switch (typeof(style)) {
    case "string" :
      s[0] = STYLE_TAG.COLOR;
      break;
    case "CanvasGradient" :
      s[0] = STYLE_TAG.GRADIENT;
      break;
    case "CanvasPattern" :
      s[0] = STYLE_TAG.PATTERN;
      break;
  }
  return s;
}

//Provides: Slant_val
//Requires: SLANT,SLANT_TAG

var tag_to_slant = new joo_global_object.Map([
  [SLANT_TAG.ROMAN,   SLANT.ROMAN],
  [SLANT_TAG.ITALIC,  SLANT.ITALIC],
  [SLANT_TAG.OBLIQUE, SLANT.OBLIQUE],
]);

function Slant_val(slant) {
  return tag_to_slant.get(slant);
}

//Provides: Val_join_type
//Requires: LINE_JOIN_TAG

var join_type_to_tag = new joo_global_object.Map([
  ["round", LINE_JOIN_TAG.ROUND],
  ["miter", LINE_JOIN_TAG.MITER],
  ["bevel", LINE_JOIN_TAG.BEVEL],
]);

function Val_join_type(join) {
  return join_type_to_tag.get(join);
}

//Provides: Join_type_val
//Requires: LINE_JOIN_TAG

var tag_to_join_type = new joo_global_object.Map([
  [LINE_JOIN_TAG.ROUND, "round"],
  [LINE_JOIN_TAG.MITER, "miter"],
  [LINE_JOIN_TAG.BEVEL, "bevel"],
]);

function Join_type_val(join) {
  return tag_to_join_type.get(join);
}

//Provides: Val_cap_type
//Requires: LINE_CAP_TAG

var cap_type_to_tag = new joo_global_object.Map([
  ["butt",   LINE_CAP_TAG.BUTT],
  ["square", LINE_CAP_TAG.SQUARE],
  ["round",  LINE_CAP_TAG.ROUND],
]);

function Val_cap_type(cap) {
  return cap_type_to_tag.get(cap);
}

//Provides: Cap_type_val
//Requires: LINE_CAP_TAG

var tag_to_cap_type = new joo_global_object.Map([
  [LINE_CAP_TAG.BUTT,   "butt"],
  [LINE_CAP_TAG.SQUARE, "square"],
  [LINE_CAP_TAG.ROUND,  "round"],
]);

function Cap_type_val(cap) {
  return tag_to_cap_type.get(cap);
}

//Provides: Val_compop
//Requires: COMPOP_TAG

var compop_to_tag = new joo_global_object.Map([
  ["source-over",      COMPOP_TAG.SOURCE_OVER],
  ["source-in",        COMPOP_TAG.SOURCE_IN],
  ["source-out",       COMPOP_TAG.SOURCE_OUT],
  ["source-atop",      COMPOP_TAG.SOURCE_ATOP],
  ["destination-over", COMPOP_TAG.DESTINATION_OVER],
  ["destination-in",   COMPOP_TAG.DESTINATION_IN],
  ["destination-out",  COMPOP_TAG.DESTINATION_OUT],
  ["destination-atop", COMPOP_TAG.DESTINATION_ATOP],
  ["lighter",          COMPOP_TAG.LIGHTER],
  ["copy",             COMPOP_TAG.COPY],
  ["xor",              COMPOP_TAG.XOR],
  ["multiply",         COMPOP_TAG.MULTIPLY],
  ["screen",           COMPOP_TAG.SCREEN],
  ["overlay",          COMPOP_TAG.OVERLAY],
  ["darken",           COMPOP_TAG.DARKEN],
  ["lighten",          COMPOP_TAG.LIGHTEN],
  ["color-dodge",      COMPOP_TAG.COLOR_DODGE],
  ["color-burn",       COMPOP_TAG.COLOR_BURN],
  ["hard-light",       COMPOP_TAG.HARD_LIGHT],
  ["soft-light",       COMPOP_TAG.SOFT_LIGHT],
  ["difference",       COMPOP_TAG.DIFFERENCE],
  ["exclusion",        COMPOP_TAG.EXCLUSION],
  ["hue",              COMPOP_TAG.HUE],
  ["saturation",       COMPOP_TAG.SATURATION],
  ["color",            COMPOP_TAG.COLOR],
  ["luminosity",       COMPOP_TAG.LUMINOSITY],
]);

function Val_compop(compop) {
  return compop_to_tag.get(compop);
}

//Provides: Compop_val
//Requires: COMPOP_TAG

var tag_to_compop = new joo_global_object.Map([
  [COMPOP_TAG.SOURCE_OVER,      "source-over"],
  [COMPOP_TAG.SOURCE_IN,        "source-in"],
  [COMPOP_TAG.SOURCE_OUT,       "source-out"],
  [COMPOP_TAG.SOURCE_ATOP,      "source-atop"],
  [COMPOP_TAG.DESTINATION_OVER, "destination-over"],
  [COMPOP_TAG.DESTINATION_IN,   "destination-in"],
  [COMPOP_TAG.DESTINATION_OUT,  "destination-out"],
  [COMPOP_TAG.DESTINATION_ATOP, "destination-atop"],
  [COMPOP_TAG.LIGHTER,          "lighter"],
  [COMPOP_TAG.COPY,             "copy"],
  [COMPOP_TAG.XOR,              "xor"],
  [COMPOP_TAG.MULTIPLY,         "multiply"],
  [COMPOP_TAG.SCREEN,           "screen"],
  [COMPOP_TAG.OVERLAY,          "overlay"],
  [COMPOP_TAG.DARKEN,           "darken"],
  [COMPOP_TAG.LIGHTEN,          "lighten"],
  [COMPOP_TAG.COLOR_DODGE,      "color-dodge"],
  [COMPOP_TAG.COLOR_BURN,       "color-burn"],
  [COMPOP_TAG.HARD_LIGHT,       "hard-light"],
  [COMPOP_TAG.SOFT_LIGHT,       "soft-light"],
  [COMPOP_TAG.DIFFERENCE,       "difference"],
  [COMPOP_TAG.EXCLUSION,        "exclusion"],
  [COMPOP_TAG.HUE,              "hue"],
  [COMPOP_TAG.SATURATION,       "saturation"],
  [COMPOP_TAG.COLOR,            "color"],
  [COMPOP_TAG.LUMINOSITY,       "luminosity"],
]);

function Compop_val(cap) {
  return tag_to_compop.get(cap);
}
