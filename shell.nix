with import <nixpkgs> {};

pkgs.mkShell {
  packages = with pkgs; [
    pkg-config ncurses
    gcc-arm-embedded
    bison flex bc
    openssl
  ];
}
