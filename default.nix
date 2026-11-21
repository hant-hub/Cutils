{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
    name = "cutils"; 
    src = ./.;

    buildInputs = [ pkgs.glibc ];
    nativeBuildInputs = [ pkgs.breakpointHook pkgs.gdb ];

    installPhase = ''
        cd /tmp
        cp -r --no-preserve=mode $src/* .
        rm -rf build
        cc build.c -g -o sb
        mkdir -p build/
        ./sb
        mkdir -p $out/lib
        mkdir -p $out/include
        cp build/cutils.a $out/lib/
        cp -r include/* $out/include/
    '';
}

