{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation (finalAttrs: {
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
        cp build/cutils.a $out/lib/libcutils.a
        cp -r include/* $out/include/
    '';
})

