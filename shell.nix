let
  nixpkgsCommit = "15b85dedcbaf9997bea11832106adb2195486443";
  nixpkgsSrc = fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/${nixpkgsCommit}.tar.gz";
    sha256 = "sha256:1s2ih6ch5rhz1m3s9vkbxrdvvmxvxkmyck9zyc3q87gg3hsn10jb";
  };
  pkgs = import nixpkgsSrc {};
in

pkgs.mkShell {
  packages = with pkgs; [
    pkg-config ncurses
    gcc-arm-embedded
    bison flex bc
    openssl
  ];
}
