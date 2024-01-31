#!/bin/sh

meson setup --wipe builddir -Denable-shared-library=true -Denable-example=true

# ninja -C builddir
