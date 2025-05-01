{
  description = "GraphNixOS";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
    };
    lib = pkgs.lib;
    name = "graphnixos";
  in {

    packages.${system}.default = pkgs.stdenv.mkDerivation {
      inherit name system;
      builder = "${pkgs.clang}";
      args = [
        "$( pkg-config --cflags raylib )"
        "-o"
        "./graphnixos"
        "-lm"
        "./main.c"
        "$( pkg-config --libs raylib )"
      ];
      nativeBuildInputs = with pkgs; [ 
        raylib
        pkg-config
      ];
      buildInputs = with pkgs; [ 
        cryptsetup
      ];
    };
  };
}
