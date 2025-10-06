{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    pkg-config
    SDL2
    glm
    glew
    mesa # for OpenGL
  ];
}
