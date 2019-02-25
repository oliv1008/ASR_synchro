#include "qdbmp.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
	char image_source[256];
	char image_dest[256];
	char nb_thread_arg[256];
} data;

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

void * master(void * a)
{
	data * d = (data*) a; 
	int res;

	/* Read an image file */
	bmp = BMP_ReadFile( d->image_source );
	BMP_CHECK_ERROR( stdout, -1 );

	/* Get image's dimensions */
	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

	//Créations threads
	nb_thread = atoi(d->nb_thread_arg);
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
	BMP_WriteFile( bmp, d->image_dest);
	BMP_CHECK_ERROR( stdout, -2 );

	/* Free all memory allocated for the image */
	BMP_Free( bmp );

	return NULL;
}

int main(int argc, char * argv[])
{
	data * d;

	if ( argc != 4 )
	{
		fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[ 0 ] );
		return 0;
	}

	sprintf(d->image_source, "%s", argv[1]);
	sprintf(d->image_dest, "%s", argv[2]);
	sprintf(d->nb_thread_arg, "%s", argv[3]);

	pthread_t masterThread;
	pthread_create(&masterThread, NULL, master, (void *) d);
	pthread_join(masterThread, NULL);

	return 0;
}