#!/usr/bin/env bash

# This install script is for macOS, using Homebrew to install dependencies.

# Always run `make configure`
make configure

./configure \
    LDFLAGS="-L/opt/homebrew/opt/ncurses/lib" \
    CPPFLAGS="-I/opt/homebrew/opt/ncurses/include" \
    PKG_CONFIG_PATH="/opt/homebrew/opt/ncurses/lib/pkgconfig"

# Build and install
make prefix=$HOME/.local
make install prefix=$HOME/.local

# Ensure the .config directory and tig configuration file exists
mkdir -p ~/.config/tig
tigrc_path=~/.config/tig/tigrc

# Configure `tig` to use `delta` in diffs, if not already done
if ! grep -q "set diff-highlight = \"delta\"" "$tigrc_path"; then
    echo "set diff-highlight = \"delta\"" >>"$tigrc_path"
fi
