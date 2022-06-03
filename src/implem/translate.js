/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

//Provides: keyname_to_keycode
//Requires: KEY
var kn_to_kc = new joo_global_object.Map([
  ["",                KEY.UNDEFINED],
  ["Unidentified",    KEY.UNDEFINED],

  ["Escape",          KEY.ESCAPE],
  ["F1",              KEY.F1],
  ["F2",              KEY.F2],
  ["F3",              KEY.F3],
  ["F4",              KEY.F4],
  ["F5",              KEY.F5],
  ["F6",              KEY.F6],
  ["F7",              KEY.F7],
  ["F8",              KEY.F8],
  ["F9",              KEY.F9],
  ["F10",             KEY.F10],
  ["F11",             KEY.F11],
  ["F12",             KEY.F12],

  ["PrintScreen",     KEY.PRINTSCREEN],
  ["ScrollLock",      KEY.SCROLLLOCK],
  ["Pause",           KEY.PAUSE],

  ["Backquote",       KEY.GRAVE_TILDE],
  ["Digit1",          KEY._1_EXCLAMATION],
  ["Digit2",          KEY._2_AT],
  ["Digit3",          KEY._3_NUMBER],
  ["Digit4",          KEY._4_DOLLAR],
  ["Digit5",          KEY._5_PERCENT],
  ["Digit6",          KEY._6_CARET],
  ["Digit7",          KEY._7_AMPERSAND],
  ["Digit8",          KEY._8_ASTERISK],
  ["Digit9",          KEY._9_LPARENTHESIS],
  ["Digit0",          KEY._0_RPARENTHESIS],
  ["Minus",           KEY.MINUS_UNDERSCORE],
  ["Equal",           KEY.EQUAL_PLUS],
  ["IntlYen",         KEY.INTERNATIONAL3], /* | and Yen key left */
  ["Backspace",       KEY.BACKSPACE],

  ["Tab",             KEY.TAB],
  ["KeyQ",            KEY.Q],
  ["KeyW",            KEY.W],
  ["KeyE",            KEY.E],
  ["KeyR",            KEY.R],
  ["KeyT",            KEY.T],
  ["KeyY",            KEY.Y],
  ["KeyU",            KEY.U],
  ["KeyI",            KEY.I],
  ["KeyO",            KEY.O],
  ["KeyP",            KEY.P],
  ["BracketLeft",     KEY.LBRACKET_CURLY],
  ["BracketRight",    KEY.RBRACKET_CURLY],
  ["Backslash",       KEY.BACKSLASH_PIPE], /* or KEY.NONUS_NUMBER_TILDE */

  ["CapsLock",        KEY.CAPSLOCK],
  ["KeyA",            KEY.A],
  ["KeyS",            KEY.S],
  ["KeyD",            KEY.D],
  ["KeyF",            KEY.F],
  ["KeyG",            KEY.G],
  ["KeyH",            KEY.H],
  ["KeyJ",            KEY.J],
  ["KeyK",            KEY.K],
  ["KeyL",            KEY.L],
  ["Semicolon",       KEY.SEMICOLON_COLON],
  ["Quote",           KEY.QUOTE_DOUBLEQUOTE],
//["Backslash",       KEY.NONUS_NUMBER_TILDE], /* or KEY.BACKSLASH_PIPE */
  ["Enter",           KEY.RETURN],

  ["ShiftLeft",       KEY.LSHIFT],
  ["IntlBackslash",   KEY.NONUS_BACKSLASH_PIPE],
  ["KeyZ",            KEY.Z],
  ["KeyX",            KEY.X],
  ["KeyC",            KEY.C],
  ["KeyV",            KEY.V],
  ["KeyB",            KEY.B],
  ["KeyN",            KEY.N],
  ["KeyM",            KEY.M],
  ["Comma",           KEY.COMMA_LESS],
  ["Period",          KEY.PERIOD_GREATER],
  ["Slash",           KEY.SLASH_QUESTION],
  ["IntlRo",          KEY.INTERNATIONAL1], /* _ and \ key */
  ["ShiftRight",      KEY.RSHIFT],

  ["ControlLeft",     KEY.LCONTROL],
  ["MetaLeft",        KEY.LMETA],
  ["AltLeft",         KEY.LALT],
  ["NonConvert",      KEY.INTERNATIONAL5], /* Muhenkan key KB */
  ["Lang2",           KEY.LANG2], /* Hanja / Eisu (Mac) */
  ["Space",           KEY.SPACEBAR],
  ["Lang1",           KEY.LANG1], /* HangulMode / Kana (Mac) */
  ["Convert",         KEY.INTERNATIONAL4], /* Henkan key */
  ["KanaMode",        KEY.INTERNATIONAL2], /* Katakana/Hiragana key */
  ["AltRight",        KEY.RALT],
  ["MetaRight",       KEY.RMETA],
  ["ContextMenu",     KEY.MENU],
  ["ControlRight",    KEY.RCONTROL],

  ["Help",            KEY.HELP], /* on old Mac keyboards */
  ["Insert",          KEY.INSERT], /* not present on Mac keyboards */
  ["Home",            KEY.HOME],
  ["PageUp",          KEY.PAGEUP],
  ["Delete",          KEY.DELETEFORWARD],
  ["End",             KEY.END],
  ["PageDown",        KEY.PAGEDOWN],

  ["ArrowUp",         KEY.UPARROW],
  ["ArrowLeft",       KEY.LEFTARROW],
  ["ArrowDown",       KEY.DOWNARROW],
  ["ArrowRight",      KEY.RIGHTARROW],

  ["NumpadLock",      KEY.PAD_NUMLOCK_CLEAR], /* NumLock is Clear on Mac */
  ["NumpadEqual",     KEY.PAD_EQUALS], /* Mac only */
  ["NumpadDivide",    KEY.PAD_DIVIDE],
  ["NumpadMultiply",  KEY.PAD_MULTIPLY],
  ["NumpadSubtract",  KEY.PAD_MINUS],
  ["Numpad7",         KEY.PAD_7_HOME],
  ["Numpad8",         KEY.PAD_8_UPARROW],
  ["Numpad9",         KEY.PAD_9_PAGEUP],
  ["NumpadAdd",       KEY.PAD_PLUS],
  ["Numpad4",         KEY.PAD_4_LEFTARROW],
  ["Numpad5",         KEY.PAD_5],
  ["Numpad6",         KEY.PAD_6_RIGHTARROW],
  ["NumpadComma",     KEY.PAD_COMMA], /* specific to Brazilian keyboards */
  ["Numpad1",         KEY.PAD_1_END],
  ["Numpad2",         KEY.PAD_2_DOWNARROW],
  ["Numpad3",         KEY.PAD_3_PAGEDOWN],
  ["Numpad0",         KEY.PAD_0_INSERT],
  ["NumpadDecimal",   KEY.PAD_DECIMAL_DELETE],
//["??????",          KEY.INTERNATIONAL6], /* Kanma (comma) */
  ["NumpadEnter",     KEY.PAD_ENTER],

  ["F13",             KEY.F13],
  ["F14",             KEY.F14], // FF: also has key = LaunchApplication6
  ["F15",             KEY.F15], // FF: also has key = LaunchApplication7
  ["F16",             KEY.F16], // FF: also has key = LaunchApplication8
  ["F17",             KEY.F17], // FF: also has key = LaunchApplication9
  ["F18",             KEY.F18], // FF: also has key = LaunchApplication10
  ["F19",             KEY.F19],
  ["F20",             KEY.F20],
  ["F21",             KEY.F21],
  ["F22",             KEY.F22],
  ["F23",             KEY.F23],
  ["F24",             KEY.F24],

  ["Lang3",           KEY.LANG3], /* Katakana key (Japanese) */
  ["Lang4",           KEY.LANG4], /* Hiragana key (Japanese) */
  ["Lang5",           KEY.LANG5], /* Zenkaku/Hankaku key (Japanese) */

  ["Katakana",        KEY.LANG3], /* Katakana key (Japanese) */
  ["Hiragana",        KEY.LANG4], /* Hiragana key (Japanese) */

  ["AudioVolumeDown", KEY.VOLUMEDOWN],
  ["AudioVolumeUp",   KEY.VOLUMEUP],
  ["AudioVolumeMute", KEY.MUTE],

//["LaunchApplication1", ?], // key only (Chrome)
//["LaunchMyComputer",   ?], // key only (Firefox)
//["MediaRecord",        ?], // on FF: key only, on Chrome: both
//["Close",              ?], // FF only, key only
]);

function keyname_to_keycode(keyname) {
  return kn_to_kc.get(keyname);
}
