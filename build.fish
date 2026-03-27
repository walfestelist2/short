#!/usr/bin/env fish

set -l src_dir "src"
set -l cc "gcc"
set -l flags -Wextra -Wpedantic -Werror
set -l opt -O2
set -l out "short"

set -l files main.c $src_dir/*.c

set -l install_path /usr/bin
set -l user_install_path $HOME/.local/bin

$cc $files -o $out $flags $opt

if test (count $argv) -eq 0
    exit 0
end

if test $argv[1] = "run"
    ./$out $argv[2..-1]
else if test $argv[1] = "install"
    sudo cp $out $install_path/$out
else if test $argv[1] = "user-install"
    cp $out $user_install_path/$out
end
