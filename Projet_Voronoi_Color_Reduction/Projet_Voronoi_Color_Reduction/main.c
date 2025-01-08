#include <unistd.h>     
#include <math.h>
#include "ima.h"

Image *image;
int nmbSites;

#define ESCAPE 27

void Keyboard(unsigned char key, int x, int y)  {
  switch(key){
  case ESCAPE :
    exit(0);                   
    break;
  default:
    fprintf(stderr, "Unused key\n");
  }
}

void Mouse(int button, int state, int x, int y) {
  switch(button){
  case GLUT_LEFT_BUTTON:
    break;
  case GLUT_MIDDLE_BUTTON:
    break;
  case GLUT_RIGHT_BUTTON:
    break;    
  }
  glutPostRedisplay();
}

int Init(char *s) {
  image = (Image *) malloc(sizeof(Image));
  if (image == NULL) {
    fprintf(stderr, "Out of memory\n");
    return(-1);
  }
  if (ImageLoad_PPM(s, image)==-1) 
	return(-1);
  printf("tailles %d %d\n",(int) image->sizeX, (int) image->sizeY);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glutReshapeWindow(image->sizeX, image->sizeY);
  
  return (0);
}
int ReInit() {
  /*
  if (ImageLoad_PPM(s, image)==-1) 
	return(-1);
  */
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glutReshapeWindow(image->sizeX, image->sizeY);
  
  return (0);
}

void Display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  //GLint w = glutGet(GLUT_WINDOW_WIDTH);
  //GLint h = glutGet(GLUT_WINDOW_HEIGHT);
  glDrawPixels(image->sizeX, image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, 
	       image->data);

  glFlush();
}

void Reshape(int w, int h) {
  glViewport(0, 0, (GLsizei)w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble)h);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void menuFunc(int item) {
  char s[256];
  switch(item){
  case 0:
    free(image);
    exit(0);
    break;
  case 1:
    voronoi_compressor(image, nmbSites);
    Display();
    break;
  case 2:
    for (int i = 0; i < 100; i++) {
      voronoi_compressor(image, nmbSites);
      Display();
    }
    break;
  case 3:
    printf("Entrer le nom pour l'image dans cette taille\n");
    scanf("%s", &s[0]);
    imagesave_PPM(s, image);
    break;
  case 4:
    printf("Taille de l image : %d %d\n", (int) image->sizeX, (int) image->sizeY);
    break;
  default:
    break;
  }
}

int main(int argc, char **argv) {  

  if (argc<3) {
    fprintf(stderr, "Usage : voronoi nom_de_fichier nombre_de_sites\n-1 pour le nombre maximum de sites par rapport à l'image)\n");
    exit(0);
  }

  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutInitWindowSize(640,480);  
  glutInitWindowPosition(100, 100);  
  glutCreateWindow("VPUP8");  

  Init(argv[1]);
  nmbSites = atoi(argv[2]);

  glutCreateMenu(menuFunc);
  glutAddMenuEntry("Quit", 0);
  glutAddMenuEntry("Voronoi Compressor", 1);
  glutAddMenuEntry("Voronoi x100 Loop :)", 2);
  glutAddMenuEntry("Save", 3);
  glutAddMenuEntry("Infos", 4);
  glutAttachMenu(GLUT_LEFT_BUTTON);

  glutDisplayFunc(Display);  
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(Keyboard);
  
  glutMouseFunc(Mouse);

  glutMainLoop();  

  return 1;
}
