/***********************************************************************************

	The program reads an BMP image file and creates a new
	image that is the black and white version of the input file.

***********************************************************************************/

#include "qdbmp.h"
#include <stdio.h>

int main( int argc, char* argv[] )
{
	/* variables */	
	UCHAR	r, g, b;
	UINT	width, height;
	UINT	x, y;
	BMP*	bmp;

	/* Check arguments  : argv[1] = input file; argv[2] = output file */
	if ( argc != 3 )
	{
		fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[ 0 ] );
		return 0;
	}
	
	/* Read an image file */
	bmp = BMP_ReadFile( argv[ 1 ] );
	BMP_CHECK_ERROR( stdout, -1 );

	/* Get image's dimensions */
	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

	/* Iterate through all the image's pixels */
	for ( x = 0 ; x < width ; ++x )
	{
		for ( y = 0 ; y < height ; ++y )
		{
			/* Get pixel's RGB values */
			BMP_GetPixelRGB( bmp, x, y, &r, &g, &b );
			/* Compute the mean value of RGB values */
			UCHAR netb= (r+g+b)/3;
			/* Set new black and white RGB values */
			BMP_SetPixelRGB( bmp, x, y, netb, netb, netb );
		}
	}
	/* Save result */
	BMP_WriteFile( bmp, argv[ 2 ] );
	BMP_CHECK_ERROR( stdout, -2 );

	/* Free all memory allocated for the image */
	BMP_Free( bmp );

	return 0;
}

