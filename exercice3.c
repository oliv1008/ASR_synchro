#include "qdbmp.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h> 

typedef struct 
{
	char image_source[256];
	char image_dest[256];
} dataMaster;

typedef struct 
{
	int indice;
	BMP * bmp;
	UINT width;
	UINT height;
} dataSlave;

UINT nb_thread;

void * task(void * a)
{
	dataSlave * dSlave = (dataSlave *) a;
	int yLimit;  
	UCHAR	r, g, b;
	UINT	x, y;

	if (dSlave->indice == nb_thread - 1)
	{
		yLimit = dSlave->height;
	}
	else
	{
		yLimit = (dSlave->height/nb_thread) * (dSlave->indice + 1);
	}

	/* Iterate through all the image's pixels */
	for ( x = 0 ; x < dSlave->width ; ++x )
	{
		for ( y = (dSlave->height/nb_thread) * dSlave->indice ; y < yLimit ; ++y )
		{
			/* Get pixel's RGB values */
			BMP_GetPixelRGB( dSlave->bmp, x, y, &r, &g, &b );
			/* Compute the mean value of RGB values */
			UCHAR netb= (r+g+b)/3;
			/* Set new black and white RGB values */
			BMP_SetPixelRGB( dSlave->bmp, x, y, netb, netb, netb );
		}
	}

	return NULL;
}

void * master(void * a)
{
	BMP* bmp;
	UINT width, height;
	dataMaster * d = (dataMaster*) a; 
	dataSlave dSlave[nb_thread];
	int res;

	/* Read an image file */
	bmp = BMP_ReadFile( d->image_source );
	BMP_CHECK_ERROR( stdout, -1 );

	/* Get image's dimensions */
	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

	//Créations threads
	pthread_t * threads = (pthread_t*) malloc(sizeof(pthread_t) * nb_thread);

	for(int i = 0; i < nb_thread; i++)
    {
    	dSlave[i].indice = i;
    	dSlave[i].bmp = bmp;
    	dSlave[i].width = width;
    	dSlave[i].height = height;

        res = pthread_create(&threads[i], NULL, task, (void*) &dSlave[i]);
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
	int nombre_fichier = 0;
	char image_dest[256] = "nb_";
	pthread_t * masterThread;
	dataMaster * d;
	struct dirent * doc;
	DIR * dir;
	clock_t start, finish;
	double duration; 

	if ( argc != 3 )
	{
		fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[ 0 ] );
		return 0;
	}
	start = clock(); 

	nb_thread = atoi(argv[2]);
	dir = opendir(argv[1]);

	while( (doc = readdir(dir)) != NULL)
	{
		if (doc->d_name[0] != '.')
		{
			nombre_fichier++;
		}
	}

	d = (dataMaster *) malloc(sizeof(dataMaster) * nombre_fichier);
	masterThread = (pthread_t *) malloc(sizeof(pthread_t) * nombre_fichier);

	rewinddir(dir);
	int i = 0;
	while ( (doc = readdir(dir)) != NULL)
	{

		if (doc->d_name[0] != '.')
		{
			sprintf(d[i].image_source, "%s/%s", argv[1], doc->d_name);
			strcat(image_dest, doc->d_name);
			sprintf(d[i].image_dest, "%s/%s", argv[1], image_dest);

			printf("Fichier %i :\n", i);
			printf("d[%i].image_source = %s\n", i, d[i].image_source);
			printf("d[%i].image_dest = %s\n", i, d[i].image_dest);

			pthread_create(&masterThread[i], NULL, master, (void *) &d[i]);

			sprintf(image_dest, "nb_");
			i++;
		}
	}

	for (int j = 0; j < nombre_fichier; j++)
	{
		pthread_join(masterThread[j], NULL);
	}

	finish = clock(); 
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "%2.5f seconds\n", duration ); 

	return 0;
}