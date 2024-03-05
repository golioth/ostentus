# How to create splashscreens

## Add a custom splash to stock Ostentus board

1. Create a 200x200 black and white image
2. Invert the colors
3. Convert the image to a .bin file (see below)
4. Use thonny to add `splashcreen.bin` to the filesystem

## Convert image to bin file

Use the `convert.py` file found in the pimoroni pico submodule. The input file
should be a 200x200 pixel black and white image with colors inverted. The .png
format works well for this step.

```
python submodules/pimoroni-pico/examples/badger2040/image_converter/convert.py --binary utility/splashscreen_rd.png
```

## Update the default splashscreen

Use the helper script to convert and position the default splash screen which
will be included in the Ostentus binary build.

The image should be a 200x200 pixel black and white image with colors inverted. The .png
format works well for this step.

```
cd utility
./make_default_splash.sh splashscreen_rd.png
```

Once completed, the file will be output to `module_py/splashscreen_rd.py`.
Rebuild Ostentus to include it.
