{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    cmake
    pkg-config
    python313Packages.jinja2

    # OpenGL / GL headers
    mesa
    libGL
    libGLU

    # Full X11 support
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    xorg.libXext
    xorg.libXfixes
  ];

  shellHook = ''
    # ensure SDL2 can find OpenGL & X11 libraries at runtime
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/run/current-system/sw/lib"

    # Add each X11 package's lib path individually (in case they are in separate store paths)
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libX11}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXrandr}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXinerama}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXcursor}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXi}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXext}/lib"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.xorg.libXfixes}/lib"

    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.libGL}/lib"

    # OpenGL driver path
    export LIBGL_DRIVERS_PATH="/run/opengl-driver/lib/dri"

    # confirm DISPLAY is set
    if [ -z "$DISPLAY" ]; then
      echo "Warning: DISPLAY is not set! X11 programs will not work."
    fi
  '';
}
