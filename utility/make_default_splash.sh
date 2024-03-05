#!/bin/bash
#
# Input: 200x200px black and white image with colors inverted
#
# Output: module_py/splashscreen_rd.py
#
# Output file will automatically be built into the binary

if [ "$#" -eq 0 ]; then
  echo "No input file provided."
  exit 1
elif [ ! -f "$1" ]; then
  echo "File not found: $1"
  exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

python $SCRIPT_DIR/../submodules/pimoroni-pico/examples/badger2040/image_converter/convert.py --binary $1
python $SCRIPT_DIR/../submodules/pimoroni-pico/examples/badger2040/image_converter/data_to_py.py splashscreen_rd.bin $SCRIPT_DIR/../module_py/splashscreen_rd.py
