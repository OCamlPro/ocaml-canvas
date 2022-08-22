
module C = Configurator.V1
module P = C.Pkg_config

let query_or_default c package c_flags link_flags =
  match P.get c with
  | None ->
      c_flags, link_flags
  | Some (pc) ->
      match P.query pc ~package with
      | Some (p) -> p.P.cflags, p.P.libs
      | None -> c_flags, link_flags

let c_test c ?(c_flags=[]) ?(link_flags=[])
    test_code (test_cflags, test_link_flags) =
  let c_flags = c_flags @ test_cflags in
  let link_flags = link_flags @ test_link_flags in
  C.c_test c ~c_flags ~link_flags test_code

let march_config _c =
  [ "-march=native" ], [ ]

let gdi_config _c =
  [ "-DHAS_GDI"; "-DUNICODE"; "-D_UNICODE" ],
  [ "-lkernel32"; "-lgdi32"; "-lgdiplus" ]

let qtz_config _c =
  [ "-DHAS_QUARTZ"; "-Qunused-arguments";
    "-Wno-missing-braces"; "-Wno-deprecated-declarations" ;
    "-framework"; "Cocoa"; "-x"; "objective-c"; ],
  [ "-framework"; "Cocoa"; "-framework"; "Carbon" (*"-x"; "objective-c" *) ]

let x11_config c =
  let cflags, libs =
    query_or_default c "xcb xcb-image xcb-shm xcb-xkb xcb-keysyms"
      [] [ "-lxcb"; "-lxcb-image"; "-lxcb-shm"; "-lxcb-xkb"; "-lxcb-keysyms" ]
  in
  "-DHAS_X11" :: cflags, libs

let wl_config c =
  let cflags, libs =
    query_or_default c "wayland-client wayland-cursor xkbcommon"
      [] [ "-lwayland-client"; "-lwayland-cursor"; "-lxkbcommon" ]
  in
  "-DHAS_WAYLAND" :: cflags, "-lrt" :: libs

let xdg_decoration_config _c =
  [ "-DHAS_XDG_DECORATION" ], []

let fc_config c =
  let cflags, libs =
    query_or_default c "fontconfig"
      [] [ "-lfontconfig" ]
  in
  cflags, libs

let ft_config c =
  let cflags, libs =
    query_or_default c "freetype2"
      [ "-I/usr/include/freetype2"; ]
      [ "-lfreetype" ]
  in
  cflags, libs

let png_config c =
  let cflags, libs =
    query_or_default c "libpng"
      [ "-I/usr/include/libpng16"; ]
      [ "-lpng16"; "-lz" ]
  in
  cflags, libs

let march_test = {|
#include <stdio.h>
int main()
{
  printf("Hello\n");
  return 0;
}
|}

let gdi_test = {|
#include <windows.h>
int main()
{
  GetModuleHandle(NULL);
  GdiFlush();
  return 0;
}
|}

let qtz_test = {|
#import <Foundation/Foundation.h>
int main () {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [pool drain];
  return 0;
}
|}

let x11_test = {|
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
int main()
{
  xcb_connection_t *c = xcb_connect(NULL, NULL);
  xcb_image_t *img = xcb_image_create_native(c, 640, 480,
                       XCB_IMAGE_FORMAT_Z_PIXMAP, 32, NULL, 0, NULL);
  return 0;
}
|}

let wl_test = {|
#include <wayland-client.h>
#include <wayland-cursor.h>
int main()
{
  wl_display_connect(NULL);
  return 0;
}
|}

let xdg_decoration_test = {|
#include <wayland-client.h>
#include <wayland-cursor.h>
#define HAS_XDG_DECORATION
#include "../implem/wayland/xdg-decor-protocol.h"
#include "../implem/wayland/xdg-decor-protocol.c"
int main()
{
  wl_display_connect(NULL);
  return 0;
}
|}

let fc_test = {|
#include <fontconfig/fontconfig.h>
int main()
{
  FcConfig *fc = FcInitLoadConfigAndFonts();
  FcConfigDestroy(fc);
  FcFini();
  return 0;
}
|}

let ft_test = {|
#include <ft2build.h>
#include FT_FREETYPE_H
int main()
{
  FT_Library ft_library;
  FT_Init_FreeType(&ft_library);
  return 0;
}
|}

let png_test = {|
#include <png.h>
int main()
{
  png_uint_32 v = png_access_version_number();
  return 0;
}
|}

let () =
  C.main ~name:"canvas" (fun c ->
    let c_flags = [] in
    let link_flags = [] in
    let options =
      List.fold_left (fun options (config_fun, test_code) ->
          let config = config_fun c in
          if c_test c ~c_flags ~link_flags test_code config then
            fst options @ fst config, snd options @ snd config
          else
            options
        ) ([], [])
        [ (march_config, march_test);
          (gdi_config, gdi_test);
          (qtz_config, qtz_test);
          (x11_config, x11_test);
          (wl_config, wl_test);
          (xdg_decoration_config, xdg_decoration_test);
          (fc_config, fc_test);
          (ft_config, ft_test);
          (png_config, png_test); ]
    in
    C.Flags.write_sexp "ccopt.sexp" (fst options);
    C.Flags.write_sexp "cclib.sexp" (snd options))

