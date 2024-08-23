#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lodepng.h"

void	*LoadPNG(char *filename, LodePNGColorType out_colorType, int out_bpp, void *palette, int *w, int *h, LodePNGColorType *in_colorType, int *in_bpp)
{
	unsigned char	*buffer;
	unsigned char	*image;
	size_t			buffersize;
	LodePNGState	decoder;

	lodepng_load_file(&buffer, &buffersize, filename);
	lodepng_state_init(&decoder);
	decoder.info_raw.colortype = out_colorType;
	decoder.info_raw.bitdepth = out_bpp;
	lodepng_decode(&image, w, h, &decoder, buffer, buffersize);
	free(buffer);

	if (decoder.error)
		return (NULL);

	if (in_colorType)	*in_colorType = decoder.info_png.color.colortype;
	if (in_bpp)
	{
		switch (decoder.info_png.color.colortype)
		{
			case LCT_RGB:
				*in_bpp = decoder.info_png.color.bitdepth * 3;
			break;

			case LCT_RGBA:
				*in_bpp = decoder.info_png.color.bitdepth * 4;
			break;

			default:
				*in_bpp = decoder.info_png.color.bitdepth;
			break;
		}
	}
	if (palette && (decoder.info_raw.colortype == LCT_PALETTE))
		memcpy(palette, decoder.info_png.color.palette, sizeof(int) * (1 << decoder.info_png.color.bitdepth));

	lodepng_state_cleanup(&decoder);

	return (image);
}

void	Utils_Write32(FILE *f, uint32_t data)
{
	fwrite(&data, sizeof(data), 1, f);
}

void	Utils_Write16(FILE *f, uint16_t data)
{
	fwrite(&data, sizeof(data), 1, f);
}

int main(int argc, char *argv[])
{
	uint8_t		*filename = NULL, *ig, *ptr, black_t = 0;
	int			i, igw, igh, bpp, vramW, cx = 0, cy = 0, px = 0, py = 0;
	char		timname[256];
	uint32_t	palette[256];

	// Handle arguments list
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
				case 't':	// Semi transparent bit
					black_t = 1;
				break;

				case 'p':	// Pixel position
					if (argc >= i + 2)
					{
						px = atoi(argv[i + 1]);
						py = atoi(argv[i + 2]);
						i += 2;
					}
					else
					{
						printf("-p option needs x y parameters following\n");
						goto usage;
					}
				break;

				case 'c':	// CLUT position
					if (argc >= i + 2)
					{
						cx = atoi(argv[i + 1]);
						cy = atoi(argv[i + 2]);
						i += 2;
					}
					else
					{
						printf("-c option needs x y parameters following\n");
						goto usage;
					}
				break;
			}
		}
		else
		{
			filename = argv[i];
			break;
		}
	}

	if (!filename)
		goto usage;

	ig = LoadPNG(filename, LCT_RGBA, 8, NULL, &igw, &igh, NULL, &bpp);
	if (ig)
	{
		FILE	*out;

		if (bpp >= 24)
			bpp = 16;
		vramW = (igw * bpp) / 16;

		if ((px < 0) || ((px+vramW) > 1024) || (py < 0) || ((py+igh) > 512))
		{
			printf("Pixel VRAM coordinates must be within 1024x512 !\n");
			free(ig);
			goto usage;
		}

		strcpy(timname, filename);
		strcpy(strstr(timname, ".png"), ".tim");

		if ((bpp == 8) || (bpp == 4))
		{
			free(ig);
			ig = LoadPNG(filename, LCT_PALETTE, bpp, palette, &igw, &igh, NULL, &bpp);
			if (!ig)
			{
				printf("Error loading PNG as paletted !\n");
				return (-1);
			}
		}
		else if (bpp != 16)
		{
			free(ig);
			printf("PNG file must be 4/8/24bits. (current PNG is %dbits)\n", bpp);
			return (-1);
		}

		out = fopen(timname, "wb");
		if (out)
		{
			int		size;
			uint8_t	*outdata, r, g, b, a;

			size = (igw * igh * bpp) / 8;
			outdata = malloc(size);

			// Write TIM header
			Utils_Write32(out, 0x10);	// ID
			if (bpp != 16)	// 4/8bits CLUT/palette based image
			{
				Utils_Write32(out, (1<<3) | ((bpp == 4) ? 0 : 1));	// flags
				Utils_Write32(out, 4+4+4+(2*(1 << bpp)));	// bnum
				Utils_Write16(out, cx);
				Utils_Write16(out, cy);
				Utils_Write16(out, 1 << bpp);
				Utils_Write16(out, 1);
				for (i = 0; i < (1 << bpp); i++)
				{
					uint16_t	color;

					r = palette[i] & 0xFF;
					g = (palette[i] >> 8) & 0xFF;
					b = (palette[i] >> 16) & 0xFF;
					a = black_t ? 0 : ((palette[i] >> 24) & 0xFF);
					color = ((b >> (8-5)) << 10) | ((g >> (8-5)) << 5) | (r >> (8-5)) | ((a ? 1 : 0) << 15);
					Utils_Write16(out, color);
				}
				if (bpp == 4)	// invert 4bits pixels data from PNG output
				{
					for (i = 0; i < size; i++)
						outdata[i] = ((ig[i] >> 4) & 0xF) | ((ig[i] & 0xF) << 4);
				}
				else	// straight copy of 8bits pixels data from PNG output
					memcpy(outdata, ig, size);
			}
			else
			{
				uint16_t	*outdata16 = (uint16_t*)outdata;

				Utils_Write32(out, 2);	// 15bits image flag
				// convert 32bits PNG pixels data to 16bits pixel PS1 format A1B5G5R5
				for (i = 0; i < igw * igh; i++)
				{
					r = ig[i*4];
					g = ig[(i*4)+1];
					b = ig[(i*4)+2];
					a = black_t ? 0 : ig[(i*4)+3];
					outdata16[i] = ((b >> (8-5)) << 10) | ((g >> (8-5)) << 5) | (r >> (8-5)) | ((a ? 1 : 0) << 15);
				}
			}

			Utils_Write32(out, 4+4+4+size);	// bnum
			Utils_Write16(out, px);
			Utils_Write16(out, py);
			Utils_Write16(out, (igw * bpp) / 16);
			Utils_Write16(out, igh);

			fwrite(outdata, 1, size, out);
			free(outdata);

			fclose(out);
			printf("Image converted to '%s'\n", timname);
		}
		else
			printf("cannot create '%s'\n", timname);

		free(ig);
	}
	else
		printf("'%s' not found or not a png !\n", filename);

	return (0);

usage:
	printf("PNG TO TIM convert utility v0.2 - by Orion_ [2024]\nhttps://orionsoft.games/\n\n");
	printf("Usage: png2tim [-p x y|-c x y] file.png\n\n");
	printf("PNG file supported 4/8/24/32bits. TIM format output 4/8/16bits only.\n"
			"Use -p x y option to specify pixel x y position in vram.\n"
			"Use -c x y option to specify CLUT (palette) x y position in vram.\n"
			"Use -t option to force black color to be transparent (override alpha layer from PNG).\n");

	return (0);
}
