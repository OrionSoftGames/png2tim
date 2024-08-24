PNG TO TIM convert utility v0.2 - by Orion_ [2024]
https://orionsoft.games/

This tool allow you to convert any modern PNG image file to a Playstation 1 TIM image file.
Input PNG file supported are 4/8/24/32bits.
Output TIM file supported are 4/8/16bits only.

The Playstation 1 TIM image format support an alpha layer of 1 bit, this alpha layer will be filled accordingly if the input PNG is 32bits with an alpha layer.

Usage: png2tim [-p x y|-c x y] file.png

Use -p x y option to specify image x y position in vram.
Use -c x y option to specify CLUT (palette) x y position in vram.
Use -t option to force black color to be transparent (override alpha layer from PNG).

