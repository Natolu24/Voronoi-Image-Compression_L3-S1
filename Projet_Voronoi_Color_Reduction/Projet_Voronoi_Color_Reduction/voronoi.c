#include "ima.h"
#include <limits.h>
#include <string.h>


typedef struct Site {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    unsigned short x;
    unsigned short y;
} Site;

typedef struct VoronoiCLUT {
    Site *sites;
    unsigned long sitesNmb;
    unsigned short sizeX;
    unsigned short sizeY;
} VoronoiCLUT;


// Fonctions optionelles / abandonnés :

void check_duplicate(int *x, int *y, VoronoiCLUT clut) {
    int i;
    // check for duplicate
    for (i = 0; i < clut.sitesNmb; i++) {
        if ((clut.sites[i].x == *x) && (clut.sites[i].y == *y)) {
            // if found, just increment x or y and recheck
            if (*x != clut.sizeX-1)
                (*x)++;
            else
                (*y)++;
            check_duplicate(x, y, clut);
            return;
        }
    }
}

void draw_point(int x, int y, Image *im) {
    int i, j;
    for (i = -2; i < 3; i++) {
        for (j = -2; j < 3; j++) {
            // not draw outside the image
            int pos = (x+i + (y+j)*im->sizeX);
            if ((pos < 0) || (pos >= (im->sizeX*im->sizeY)))
                continue;
            im->data[pos*3 + 0] = 255;
            im->data[pos*3 + 1] = 0;
            im->data[pos*3 + 2] = 0;
        }
    }
}

void debug_sites(VoronoiCLUT clut, Image *im) {
    int i;
    for (i = 0; i < clut.sitesNmb; i++) {
        draw_point(clut.sites[i].x, clut.sites[i].y, im);
    }
}

void check_clut_color_duplicate(VoronoiCLUT clut) {
    int *colors = malloc(clut.sitesNmb * sizeof(int));
    int i, j, taille = 0, check = 0;
    for (i = 0; i < clut.sitesNmb; i++) {
        int tmp = clut.sites[i].r+(clut.sites[i].g<<8)+(clut.sites[i].b<<16);
        for (j = 0; j < taille; j++) {
            if (colors[j] == tmp) {
                check = 1;
                break;
            }
        }
        if (check) {
            check = 0;
        }
        else {
            colors[taille] = tmp;
            taille++;
        }
    }
    printf("Il existe %ld couleurs qui sont en doubles...\n", clut.sitesNmb-taille);
    free(colors);
}

Site search_closest_site(int x, int y, VoronoiCLUT clut) {
    int i, min_dist = clut.sizeX*clut.sizeY;
    Site min;
    for (i = 0; i < clut.sitesNmb; i++) {
        int dist = sqrt(((x-clut.sites[i].x)*(x-clut.sites[i].x)) + ((y-clut.sites[i].y)*(y-clut.sites[i].y)));
        if (dist < min_dist) {
            min = clut.sites[i];
            min_dist = dist;
        }
    }
    return min;
}

void apply_voronoi(VoronoiCLUT clut, Image *im) {
    int x, y;
    for (x = 0; x < im->sizeX; x++) {
        for (y = 0; y < im->sizeY; y++) {
            Site s = search_closest_site(x, y, clut);
            im->data[(x + y*im->sizeX)*3 + 0] = s.r;
            im->data[(x + y*im->sizeX)*3 + 1] = s.g;
            im->data[(x + y*im->sizeX)*3 + 2] = s.b;
        }
        if ((x%100) == 0)
            printf("ligne %d effectué\n", x);
    }
}





// Fonctions essentielles au programme :

VoronoiCLUT generate_voronoi(int sitesNmb, Image *im) {
    int i, x, y;
    VoronoiCLUT clut;
    clut.sizeX = im->sizeX;
    clut.sizeY = im->sizeY;
    clut.sitesNmb = 0;
    // if numbers is -1, add the maximum of sites, within the above limit (+ other shortcut)
    if (sitesNmb == -1) sitesNmb = im->sizeX*im->sizeY/10;
    else if (sitesNmb == -2) sitesNmb = im->sizeX*im->sizeY/100;
    else if (sitesNmb == -3) sitesNmb = im->sizeX*im->sizeY/1000;
    else if (sitesNmb < 0) sitesNmb = im->sizeX*im->sizeY/10;
    // limit the size of possible voronoi sites to the number of pixels divide by 10
    sitesNmb = ((sitesNmb*10) > im->sizeX*im->sizeY) ? im->sizeX*im->sizeY/10 : sitesNmb;
    clut.sites = malloc(sizeof(Site) * sitesNmb);
    srand(time(NULL));
    for (i = 0; i < sitesNmb; i++) {
        Site s;
        // random position
        x = rand() % im->sizeX;
        y = rand() % im->sizeY;
        // set the pixel choosed
        s.r = im->data[(x + y*im->sizeX)*3 + 0];
        s.g = im->data[(x + y*im->sizeX)*3 + 1];
        s.b = im->data[(x + y*im->sizeX)*3 + 2];
        s.x = x;
        s.y = y;
        // add the pixel to the "voronoi" clut
        clut.sites[i] = s;
        clut.sitesNmb++;
    }
    return clut;
}

void write_compressed_file(VoronoiCLUT clut) {
    int i;
    FILE *file = fopen("compressed.vor", "w");
    if (file == NULL)
        exit(1);
    fwrite(&(clut.sizeX), sizeof(unsigned short), 1, file);
    fwrite(&(clut.sizeY), sizeof(unsigned short), 1, file);
    for (i = 0; i < clut.sitesNmb; i++) {
        fwrite(&(clut.sites[i].r), sizeof(GLubyte), 1, file);
        fwrite(&(clut.sites[i].g), sizeof(GLubyte), 1, file);
        fwrite(&(clut.sites[i].b), sizeof(GLubyte), 1, file);
        fwrite(&(clut.sites[i].x), sizeof(unsigned short), 1, file);
        fwrite(&(clut.sites[i].y), sizeof(unsigned short), 1, file);
    }
    fclose(file);
}

long read_compressed_file(VoronoiCLUT *clut) {
    int i;
    long size;
    FILE *file = fopen("compressed.vor", "r");
    if (file == NULL)
        exit(1);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 4)
        exit(1);
    clut->sitesNmb = (size-4)/7;
    clut->sites = malloc(sizeof(Site) * clut->sitesNmb);
    fread(&(clut->sizeX), sizeof(unsigned short), 1, file);
    fread(&(clut->sizeY), sizeof(unsigned short), 1, file);
    for (i = 0; i < clut->sitesNmb; i++) {
        if ((fread(&(clut->sites[i].r), sizeof(GLubyte), 1, file)) != 1)
            break;
        fread(&(clut->sites[i].g), sizeof(GLubyte), 1, file);
        fread(&(clut->sites[i].b), sizeof(GLubyte), 1, file);
        fread(&(clut->sites[i].x), sizeof(unsigned short), 1, file);
        fread(&(clut->sites[i].y), sizeof(unsigned short), 1, file);
    }
    fclose(file);
    return size;
}

void fill_with_unused_color(VoronoiCLUT clut, Image *im) {
    int i, found = 1;
    GLubyte color[3] = {0, 0, 0};
    while (found) {
        for (i = 0; i < clut.sitesNmb; i++) {
            found = 0;
            if ((clut.sites[i].r == color[0]) && (clut.sites[i].g == color[1]) && (clut.sites[i].b == color[2])) {
                found = 1;
                break;
            }
        }
        if (color[0] != 255) {
            color[0]++;
        }
        else if (color[1] != 255) {
            color[0] = 0;
            color[1]++;
        }
        else if (color[2] != 255) {
            color[0] = 0;
            color[1] = 0;
            color[2]++;
        }
        else {
            printf("Toutes les couleurs possibles sont déjà utilisé ?????\n");
            exit(1);
        }
    }
    for (i = 0; i < (im->sizeX*im->sizeY); i++) {
        im->data[i*3 + 0] = color[0];
        im->data[i*3 + 1] = color[1];
        im->data[i*3 + 2] = color[2];
    }
}

int draw_side(int side, int nmb, Site site, Image *im, GLubyte *color) {
    int i, result = 1, x = site.x, y = site.y;
    switch (side) {
        case 0: x -= nmb; break;
        case 1: y -= nmb; break;
        case 2: y += nmb; break;
        case 3: x += nmb; break;
    }
    for (i = 0; i < nmb; i++) {
        if ((x < 0) || (y < 0) || (x >= im->sizeX) || (y >= im->sizeY)) {
            x += side/2 ? -1 : 1;
            y += side%2 ? 1 : -1;
            continue;
        }
        if ((im->data[(x+y*im->sizeX)*3 + 0] == color[0]) && (im->data[(x+y*im->sizeX)*3 + 1] == color[1]) && (im->data[(x+y*im->sizeX)*3 + 2] == color[2])) {
            im->data[(x+y*im->sizeX)*3 + 0] = site.r;
            im->data[(x+y*im->sizeX)*3 + 1] = site.g;
            im->data[(x+y*im->sizeX)*3 + 2] = site.b;
            result = 0;
        }
        x += side/2 ? -1 : 1;
        y += side%2 ? 1 : -1;
    }
    return result;
}

void voronoi_decompressor(VoronoiCLUT clut, Image *im) {
    int i, range = 1, finished = 1;
    GLubyte color[3] = {im->data[0], im->data[1], im->data[2]};
    // check for all the sites, to continue propagate or finished
    int *check = calloc(clut.sitesNmb*4, sizeof(int));
    if (check == NULL)
        exit(1);
    // first set the color of the sites at their location
    for (i = 0; i < clut.sitesNmb; i++) {
        im->data[(clut.sites[i].x + clut.sites[i].y*im->sizeX)*3 + 0] = clut.sites[i].r;
        im->data[(clut.sites[i].x + clut.sites[i].y*im->sizeX)*3 + 1] = clut.sites[i].g;
        im->data[(clut.sites[i].x + clut.sites[i].y*im->sizeX)*3 + 2] = clut.sites[i].b;
    }
    // then propagate the color
    while (finished) {
        finished = 0;
        for (i = 0; i < clut.sitesNmb*4; i++) {
            if (check[i] == 0) {
                check[i] = draw_side(i%4, range, clut.sites[i/4], im, color);
                finished = 1;
            }
        }
        range++;
    }
    free(check);
}

void quality_loss_check(Image im1, Image im2) {
    int i, result = 0;
    for (i = 0; i < im1.sizeX*im2.sizeY; i++) {
        result += abs(im1.data[i*3 + 0] - im2.data[i*3 + 0]);
        result += abs(im1.data[i*3 + 1] - im2.data[i*3 + 1]);
        result += abs(im1.data[i*3 + 2] - im2.data[i*3 + 2]);
    }
    result /= im1.sizeX*im1.sizeY;
    printf("Qualité perdu :\n");
    printf("Moyenne de la distance des couleurs des pixels comparé à l'image original : %d\n", result);
}

void voronoi_compressor(Image *im, int nmbSites) {
    long compressed_size;
    clock_t begin, end;
    // Copie de l'image originale, pour pouvoir la comparé avec l'image compressé
    Image cpy;
    cpy.sizeX = im->sizeX;
    cpy.sizeY = im->sizeY;
    cpy.data = (GLubyte *) malloc ((size_t) im->sizeX*im->sizeY*3 * sizeof (GLubyte));
    memcpy(cpy.data, im->data, im->sizeX*im->sizeY*3);

    // Génération d'un "CLUT" étant les sites de voronois
    printf("\nGénération des sites de Voronoi... ");
    begin = clock();
    VoronoiCLUT clut = generate_voronoi(nmbSites, im);
    end = clock();
    printf("FINI en %.1fms\n", (double) (end-begin) / CLOCKS_PER_SEC * 1000);

    // Sauvegarde du CLUT dans un fichier (image compressé)
    printf("\nCréation du fichier compressé à parti du CLUT de Voronoi... ");
    begin = clock();
    write_compressed_file(clut);
    end = clock();
    printf("FINI en %.1fms\n", (double) (end-begin) / CLOCKS_PER_SEC * 1000);
    free(clut.sites);

    VoronoiCLUT save;
    // Lecture du fichier compressé pour reconstitué le CLUT
    printf("\nLecture du fichier compressé pour reconstitué le CLUT de Voronoi... ");
    begin = clock();
    compressed_size = read_compressed_file(&save);
    end = clock();
    printf("FINI en %.1fms\n", (double) (end-begin) / CLOCKS_PER_SEC * 1000);

    // Remplie l'image d'une couleur non utilisé par le CLUT (pour l'utilisé en tant que "fond vert")
    printf("\nTrouve et remplie l'image par une couleur inutilisé par le CLUT... ");
    begin = clock();
    fill_with_unused_color(save, im);
    end = clock();
    printf("FINI en %.1fms\n", (double) (end-begin) / CLOCKS_PER_SEC * 1000);

    // Utilise le CLUT pour reconstitué/décompressé l'image
    printf("\nReconstitue l'image à partir du CLUT... ");
    begin = clock();
    voronoi_decompressor(save, im);
    end = clock();
    printf("FINI en %.1fms\n\n", (double) (end-begin) / CLOCKS_PER_SEC * 1000);

    // Regarde la "perte en qualité" de l'image
    quality_loss_check(*im, cpy);

    // Ratio de compression
    printf("\nTaille original de l'image : %ld\n", im->sizeX*im->sizeY*3);
    printf("Taille de la compression : %ld\n", compressed_size);
    printf("Ratio de compression : %.2f%%\n", ((double)compressed_size/(im->sizeX*im->sizeY*3))*100);
    free(save.sites);
}