PNG TO TIM convert utility v0.3 - by Orion_ [2024-2026]
https://orionsoft.games/

This tool allow you to convert any modern PNG image file to a Playstation 1 TIM image file.
Input PNG file supported are 4/8/24/32bits.
Output TIM file supported are 4/8/16bits only.

The Playstation 1 TIM image format support an alpha layer of 1 bit, this alpha layer will be filled accordingly if the input PNG is 32bits with an alpha layer.

Usage: png2tim [-p x y|-c x y|-t|] file.png

Use -p x y option to specify image x y position in vram.
Use -c x y option to specify CLUT (palette) x y position in vram.
Use -t option to force black color to be transparent (override alpha layer from PNG).
Use -m w h option to create a 4bits TIM image that uses up to 16 CLUTs from a 8bits input PNG image.
Each tile of given size w/h must only use 16 colors of one of the 16 CLUTs from the 8bits input image.
The CLUTs will be written in the same order of the tiles from left to right, up to bottom.
There must not be more than 16 tiles per image.
This option allows you to use the 2kb texture cache of the ps1 by loading one single texture of 4bits 256x256 on a single texture page but use 64x64 4bits textures with each texture having its own palette which maximize the use of colors.

