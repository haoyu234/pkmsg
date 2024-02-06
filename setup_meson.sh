#!/bin/sh

meson setup --wipe builddir -Denable-shared-library=true -Denable-example=true -Denable-fuzz=true

# ninja -C builddir
