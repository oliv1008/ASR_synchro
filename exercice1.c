#include "qdbmp.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

UINT nb_thread;
BMP* bmp;
UINT width, height;

void * task(void * a)
{
	int indice = *((int*) a);
	int yLimit;  
	UCHAR	r, g, b;
	UINT	x, y;

	if (indice == nb_thread - 1)
	{
		yLimit = height;
	}
	else
	{
		yLimit = (height/nb_thread) * (indice + 1);
	}

	/* Iterate through all the image's pixels */
	for ( x = 0 ; x < width ; ++x )
	{
		for ( y = (height/nb_thread) * indice ; y < yLimit ; ++y )
		{
			/* Get pixel's RGB values */
			BMP_GetPixelRGB( bmp, x, y, &r, &g, &b );
			/* Compute the mean value of RGB values */
			UCHAR netb= (r+g+b)/3;
			/* Set new black and white RGB values */
			BMP_SetPixelRGB( bmp, x, y, netb, netb, netb );
		}
	}

	return NULL;
}

int main(int argc, char * argv[])
{
	int res;

	if ( argc != 4 )
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

	//Créations threads
	nb_thread = atoi(argv[3]);
	pthread_t * threads = (pthread_t*) malloc(sizeof(pthread_t) * nb_thread);
	int* tab = malloc(sizeof(int) * nb_thread);

	for(int i = 0; i < nb_thread; i++)
    {
    	tab[i] = i;
        res = pthread_create(&threads[i], NULL, task, (void*) &tab[i]);
        if(res != 0) perror("erreur creation thread");
    }

    for(int i = 0; i < nb_thread; i++)
    {
        res = pthread_join(threads[i], NULL);
        if(res != 0) perror("erreur join thread");
    }

    /* Save result */
	BMP_WriteFile( bmp, argv[ 2 ] );
	BMP_CHECK_ERROR( stdout, -2 );

	/* Free all memory allocated for the image */
	BMP_Free( bmp );
}