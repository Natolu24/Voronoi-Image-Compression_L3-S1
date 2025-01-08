#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

/* pour Bocal */
#include <GL/gl.h> 
#include <GL/glut.h>
#include <GL/freeglut.h>  

/* pour Mac */
// #include <GLUT/glut.h>




struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    GLubyte *data;
};
typedef struct Image Image;
typedef unsigned short utab [3][3][3];

int ImageLoad_PPM(char *filename, Image *image);
void imagesave_PPM(char *filename, Image *image);
void upsidedown(Image *im);
void voronoi_compressor(Image *image, int nmbSites);


