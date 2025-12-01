{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    pkg-config
    cmake
    mesa # for OpenGL
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libGL
    libGLU
    pkgs.mesa
    python313Packages.jinja2
  ];
  shellHook = ''
    export LD_LIBRARY_PATH="\
      /run/opengl-driver/lib/:\
      $LD_LIBRARY_PATH"
  '';
}
