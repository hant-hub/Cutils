{ pkgs ? import <nixpkgs> {} }:

pkgs.clangStdenv.mkDerivation {
    name = "cutils"; 
    src = ./.;

    nativeBuildInputs = [ pkgs.breakpointHook pkgs.gdb ];
    buildInputs = [ pkgs.liburing pkgs.clang ];

    installPhase = ''
        cd /tmp
        cp -r --no-preserve=mode $src/* .
        rm -rf build
        cc build.c -g -o sb
        mkdir -p build/
        ./sb
        mkdir -p $out/lib
        cp build/cutils.a $out/lib/
    '';
}

