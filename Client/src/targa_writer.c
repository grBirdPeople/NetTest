
// vanessa writes a targa file

// compiling and running this file will produce a targa file

// a lot of this is based on Grant Emery's file https://www.tjhsst.edu/~dhyatt/superap/code/targa.c thanks dude

// author: vanessa pyne --- github.com/vipyne





#include <stdio.h>



#define BYTE_RANGE 256;



////// targa file header



typedef struct {

	char id_length;      // length of id field (number of bytes - max 255)

	char map_type;       // colormap field (0 or 1; no map or 256 entry palette)

	char image_type;     // ( 0 - no image data included

				 //	  1 - uncompressed, color mapped image

				 //	  2 - uncompressed, RGB image

				 //	  3 - uncompressed, black & white image

				 //	  9 - run-length encoded(RLE-lossless compression),color mapped image

				 //	 10 - RLE, RGB image

				 //	 11 - compressed, black & white image )



	int map_first;       // first entry index for color map

	int map_length;      // total number of entries in color map

	char map_entry_size; // number of bits per entry



	int x;               // x cooridinate of origin

	int y;               // y cooridinate of origin



	int width;           // width in pixels

	int height;          // height in pixels



	char bits_per_pixel; // number of bits per pixel



	char misc;           // srsly? "scan origin and alpha bits" this example uses scan origin

				 // honestly, don't know what's going on here. we pass in a hex value

				 // :shrug_emoji:		

} targa_header;



int little_endianify(int number)

{

	return number % BYTE_RANGE;

}



int big_endianify(int number)

{

	return number / BYTE_RANGE;

}



////// write header function



void write_header(targa_header header, FILE *tga)

{

	fputc(header.id_length, tga);

	fputc(header.map_type, tga);

	fputc(header.image_type, tga);



	fputc(little_endianify(header.map_first), tga);

	fputc(big_endianify(header.map_first), tga);



	fputc(little_endianify(header.map_length), tga);

	fputc(big_endianify(header.map_length), tga);



	fputc(header.map_entry_size, tga);



	fputc(little_endianify(header.x), tga);

	fputc(big_endianify(header.x), tga);

	fputc(little_endianify(header.y), tga);

	fputc(big_endianify(header.y), tga);



	fputc(little_endianify(header.width), tga);

	fputc(big_endianify(header.width), tga);

	fputc(little_endianify(header.height), tga);

	fputc(big_endianify(header.height), tga);



	fputc(header.bits_per_pixel, tga);

	fputc(header.misc, tga);

}