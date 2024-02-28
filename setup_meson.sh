#!/bin/sh

meson setup --wipe builddir -Denable-example=true -Denable-fuzz=true

# ninja -C builddir
