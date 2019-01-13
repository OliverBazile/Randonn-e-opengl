#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "GL4D/gl4droid.h"
#include "image.h"
#include <assert.h>
#include <dlfcn.h>

#define  LOG_TAG    "RANDO"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define W 256
#define H 256
#define NBARBRES 20

PFNGLBINDVERTEXARRAYOESPROC bindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC deleteVertexArraysOES;
PFNGLGENVERTEXARRAYSOESPROC genVertexArraysOES;
PFNGLISVERTEXARRAYOESPROC isVertexArrayOES;

/*
 * Prototypes des fonctions statiques contenues dans ce fichier C
 */

static void triangle_edge(uint8_t *im, int x, int y, int w, int h, int width);
static void initData(void);
static void loop(GLfloat*, GLfloat*, GLfloat a0);
//static void manageEvents(SDL_Window * win);


//static GLuint _program;
static GLuint _vPositionHandle, _vTextureHandle, _vNormalHandle;
static GLfloat _yScale = 1.0f;
static GLfloat _windowWidth = 1.0f, _windowHeight = 1.0f;
//static int _windowWidth = 800, _windowHeight = 600;

/*!\brief identifiant du (futur) vertex array object */
static GLuint _vao[8] = {0,0,0,0,0,0,0,0};
/*!\brief identifiant du (futur) buffer de data */
static GLuint _buffer = 0;
/*!\brief identifiant du (futur) GLSL program */
static GLuint _pId[3] = {0,0};
static GLuint _pIdN[3] = {0,0};
/*!\brief identifiant de la texture */
static GLuint _tId[7] = {0,0,0,0,0,0,0};
static const GLfloat * data, dataLune[], dataSoleil[];
GLfloat camera[16], modelView[16], modelViewProjection[16];
GLfloat * eyeViews, *eyePerspectives;
GLfloat * headview, * forward, * up, * right;
float forward1, forward2;
static GLfloat _ratio_x = 1.0f, _ratio_y = 1.0f;

GLuint buffData, buffLune, buffSoleil;

GLuint texEau,texSable, texHerbe, texRoche, texNeige;
GLuint texMArbre, texGArbre;
GLuint texLune, texSoleil;

double Imax,Jmax,Id,I,Jd,J,Id1,Jd1;

static AAssetManager* asset_manager;

uint8_t Pixels[W * H];

static GLuint _pause = 0;
static GLuint _activeToon = 0;
static GLuint _activeNight = 0;
uint32_t t0;
uint32_t t1, t2;

GLfloat pas;
GLuint pasOn = 0;

enum kyes_t {
  KLEFT = 0,
  KRIGHT,
  KUP,
  KDOWN
};
static GLuint _keys[] = {0, 0, 0, 0};

typedef struct cam_t cam_t;
struct cam_t {
  GLfloat x,y,z;
  GLfloat theta;
};

static cam_t _cam = {0, 1, -100, 0};

typedef struct arbre_t arbre_t;
struct arbre_t{
  GLfloat x,z,y;
  GLuint type;
};
static arbre_t _arbre[NBARBRES];

typedef struct Lune_t Lune_t;
struct Lune_t{
  GLfloat x,z,y;
  GLfloat theta;
};
static Lune_t _Lune = {-3,300,0,0};

typedef struct Soleil_t Soleil_t;
struct Soleil_t{
	GLfloat x, y, z;
	GLfloat theta;	
};
static Soleil_t _Soleil = {-3,0,300,0};

GLfloat S,S2;


static void triangle_edge(uint8_t *im, int x, int y, int w, int h, int width) {
  int v;
  int p[9][2], i, w_2 = w >> 1, w_21 = w_2 + (w&1), h_2 = h >> 1, h_21 = h_2 + (h&1);
  p[0][0] = x;       p[0][1] = y;
  p[1][0] = x + w;   p[1][1] = y;
  p[2][0] = x + w;   p[2][1] = y + h;
  p[3][0] = x;       p[3][1] = y + h;
  p[4][0] = x + w_2; p[4][1] = y;
  p[5][0] = x + w;   p[5][1] = y + h_2;
  p[6][0] = x + w_2; p[6][1] = y + h;
  p[7][0] = x;       p[7][1] = y + h_2;
  p[8][0] = x + w_2; p[8][1] = y + h_2;
  for(i = 4; i < 8; i++) {
    if(im[p[i][0] + p[i][1] * width])
      continue;
    v = (((int)im[p[i - 4][0] + p[i - 4][1] * width]) +
         ((int)im[p[(i - 3) % 4][0] + p[(i - 3) % 4][1] * width])) >> 1;
    v += gl4dmSURand() * w_2; /* gl4dmSURand() -> al�atoire entre -1 et +1 */
    v = MIN(MAX(v, 1), 255);
    im[p[i][0] + p[i][1] * width] = (uint8_t)v;
  }
  if(!im[p[i][0] + p[i][1] * width]) {
    v = (((int)im[p[0][0] + p[0][1] * width]) +
         ((int)im[p[1][0] + p[1][1] * width]) +
         ((int)im[p[2][0] + p[2][1] * width]) +
         ((int)im[p[3][0] + p[3][1] * width])) >> 2;
    v += gl4dmSURand() * w_2 * sqrt(2);
    v = MIN(MAX(v, 1), 255);
    im[p[8][0] + p[8][1] * width] = (uint8_t)v;
  }
  if(w_2 > 1 || h_2 > 1)
    triangle_edge(im, p[0][0], p[0][1], w_2, h_2, width);
  if(w_21 > 1 || h_2 > 1)
    triangle_edge(im, p[4][0], p[4][1], w_21, h_2, width);
  if(w_21 > 1 || h_21 > 1)
    triangle_edge(im, p[8][0], p[8][1], w_21, h_21, width);
  if(w_2 > 1 || h_21 > 1)
    triangle_edge(im, p[7][0], p[7][1], w_2, h_21, width);
}


static void reshape() {

   // glViewport(0, 0, _windowWidth, _windowHeight);
    gl4duBindMatrix("projectionMatrix");
    gl4duLoadIdentityf();


    //gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 1.0, 1000.0);
}

static int init(const char * vs, const char * fs, const char * toons, const char * fnights, const char * fnighttoons) {
    gl4dInitTime0();

    LOGD("Version d'OpenGL : %s", glGetString(GL_VERSION));
    LOGD("Version de shaders supportes : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    LOGD("Init c");

    void *libhandle = dlopen("libGLESv2.so", RTLD_LAZY);

    bindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) dlsym(libhandle,
                                                             "glBindVertexArrayOES");
    deleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) dlsym(libhandle,
                                                                   "glDeleteVertexArraysOES");
    genVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)dlsym(libhandle,
                                                            "glGenVertexArraysOES");
    isVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)dlsym(libhandle,
                                                        "glIsVertexArrayOES");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    _pId[0] = gl4droidCreateProgram(vs, toons);
    _pId[1] = gl4droidCreateProgram(vs, fnighttoons);

    if (!_pId[0] && !_pId[1] /*&& !_pIdN[0] && !_pIdN[1]*/)
        return 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    glUniform1i(glGetUniformLocation(_pId[0], "u_alphatestenable"), 1);
//    glUniform1i(glGetUniformLocation(_pId[1], "u_alphatestenable"), 1);
//    gl4duGenMatrix(GL_FLOAT, "projmat");
//    gl4duGenMatrix(GL_FLOAT, "mmat");
//    gl4duGenMatrix(GL_FLOAT, "vmat");


    gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
    gl4duGenMatrix(GL_FLOAT, "projectionMatrix");

    reshape();

    _vPositionHandle = glGetAttribLocation(_pId[0], "vsiPosition");
    _vNormalHandle = glGetAttribLocation(_pId[0], "vsiNormal");
    _vTextureHandle = glGetAttribLocation(_pId[0], "vsiTexCoord");

    LOGD("created programs");

    LOGD("_vPositionHandle: %d   _vNormalHandle: %d    _vTextureHandle: %d",
         _vPositionHandle, _vNormalHandle, _vTextureHandle);


    initData();

    return 1;
}


static void tnormale(GLfloat * triangle, GLfloat * n) {
  GLfloat * u = triangle;
  GLfloat * v = &triangle[3];
  GLfloat * w = &triangle[6];
  GLfloat uv[3] = {v[0] - u[0], v[1] - u[1], v[2] - u[2]}, vw[3] = {w[0] - v[0], w[1] - v[1], w[2] - v[2]}; 
  MVEC3CROSS(n, uv, vw);
  MVEC3NORMALIZE(n);
}

static GLdouble hauteur(uint8_t * pixels, int i) {
  S2 = 100.0f;
  return S2 * pixels[i] / 255.0;
}

static GLdouble cote(int i, int w) {
  S = 300.0f; GLfloat a;
  return S * (2.0 * (i / (GLfloat)w) - 1.0);
}

static void normale(uint8_t * pixels, int x, int z, GLfloat * n, int w, int h) {
  int dir[][2] = {
    {1, 0},
    {0, -1},
    {1, 1},
    {1, 0},
    {0, 1},
    {1, 1},
    {-1, 0},
    {0, 1},
    {-1, -1},
    {-1, 0},
    {0, -1},
    {-1, -1}
  }, i, k;
  GLfloat t[9], tn[3];
  
  n[0] = n[1] = n[2] = 0;
  for(i = 0; i < 6; i++) {
    int x1 = x + dir[2 * i][0], z1 = z + dir[2 * i][1],
      x2 = x + dir[2 * i + 1][0], z2 = z + dir[2 * i + 1][1];
    if( x1 < 0 || z1 < 0 || x2 < 0 || z2 < 0 ||
	x1 >= w || z1 >= h || x2 >= w || z2 >= h )
      continue;
    k = 0;
    t[k++] = cote(x, w);
    t[k++] = hauteur(pixels, z * w + x);
    t[k++] = cote(z, h);
    t[k++] = cote(x1, w);
    t[k++] = hauteur(pixels, z1 * w + x1);
    t[k++] = cote(z1, h);
    t[k++] = cote(x2, w);
    t[k++] = hauteur(pixels, z2 * w + x2);
    t[k++] = cote(z2, h);
    tnormale(t, tn);
    n[0] += tn[0];
    n[1] += tn[1];
    n[2] += tn[2];
  }
  MVEC3NORMALIZE(n);
}

double diff (int a, int b){
	double res;
	if(a>b) res= a;
	else res = b;
	return res;
}

GLuint load_texture(GLuint texture_object_id, const GLsizei width, const GLsizei height,const GLenum type, const GLvoid* pixels) {

    glGenTextures(1, &texture_object_id);
    assert(texture_object_id != 0);

    glBindTexture(GL_TEXTURE_2D, texture_object_id);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(
            GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_object_id;
}

GLuint load_png_asset_into_texture(const char* relative_path, GLuint texture_id) {
    assert(relative_path != NULL);
    LOGD("init loading image %s", relative_path);
    const FileData png_file = get_asset_data(asset_manager, relative_path);
    LOGD("getting raw image");
    const RawImageData raw_image_data =
            get_raw_image_data_from_png(png_file.data, png_file.data_length);
    const GLuint texture_object_id = load_texture(
            texture_id,
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    release_raw_image_data(&raw_image_data);
    release_asset_data(&png_file);

    LOGD("fini loading image %s", relative_path);
    return texture_object_id;
}

static void initData(void){
 
  int i, j, k, c, I ,J;

  GLfloat * data;


  memset(Pixels, 0, W * H * sizeof *Pixels);
  Pixels[0] = 0;         Pixels[W - 1] = 0;
  Pixels[(H - 1) * W] = 0; Pixels[(H - 1) * W + W - 1] = 0;
  triangle_edge(Pixels, 0, 0, W - 1, H - 1, W);

  data = malloc((W - 1) * (H - 1) * 6 * 8 * sizeof * data);
  assert(data);


  for(i = 0, k = 0; i < H - 1; i++) {

    GLfloat z = cote(i, H), zp1 = cote(i + 1, H);

    for(j = 0; j < W - 1; j++) {
 
      GLfloat x = cote(j, W), xp1 = cote(j + 1, W);

      c = i * W + j;
      //0
      data[k++] = x;
      data[k++] = hauteur(Pixels, c);
      data[k++] = z;
      normale(Pixels, j, i, &data[k], W, H);
      k += 3;
      data[k++] = x; data[k++] = z;
//J et I___________________________________________________		
//_________________________________________________________

      //1
      data[k++] = xp1;
      data[k++] = hauteur(Pixels, c + W + 1);
      data[k++] = zp1;
      normale(Pixels, j + 1, i + 1, &data[k], W, H);
      k += 3;
      data[k++] = xp1; data[k++] = zp1;
//J+1 et I+1___________________________________________________
//_____________________________________________________________
	
      //2
      data[k++] = xp1;
      data[k++] = hauteur(Pixels, c + 1);
      data[k++] = z;
      normale(Pixels, j + 1, i, &data[k], W, H);
      k += 3;
      data[k++] = xp1; data[k++] = z;
//J et I+1___________________________________________________
//___________________________________________________________
      //0
      data[k++] = x;
      data[k++] = hauteur(Pixels, c);
      data[k++] = z;
      normale(Pixels, j, i, &data[k], W, H);
      k += 3;
      data[k++] = x; data[k++] = z;
//J et I___________________________________________________
//_________________________________________________________	
      //4
      data[k++] = x;
      data[k++] = hauteur(Pixels, c + W);
      data[k++] = zp1;
      normale(Pixels, j, i + 1, &data[k], W, H);
      k += 3;
      data[k++] = x; data[k++] = zp1;

//J+1 et I___________________________________________________
//___________________________________________________________
      //5
      data[k++] = xp1;
      data[k++] = hauteur(Pixels, c + W + 1);
      data[k++] = zp1;
      normale(Pixels, j + 1, i + 1, &data[k], W, H);
      k += 3;
      data[k++] = xp1; data[k++] = zp1;
//J+1 et I+1___________________________________________________			
//_____________________________________________________________
    }
			
  }

  GLfloat s4 = 5.0;
  GLfloat s5 = 5.0;
  GLfloat data3[] = {
    /* 4 coordonn�es de sommets */
    -s4, 0.0f, 0.0f,
    s4 , 0.0f, 0.0f,
    -s4, s5,  0.0f,
    s4 , s5,  0.0f,
    /* 4 normales */
    0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    /* 4 coordonn�es de texture, une par sommet */
    1.0f, 1.0f, 
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f
  };

  
  GLfloat d2= 100.0, dataSoleil[]={

		 /* 4 coordonn�es de sommets */
   -d2, 0.0f, 0.0f,
    d2 , 0.0f, 0.0f,
   -d2, 2*d2,  0.0f,
    d2 ,2*d2,  0.0f,
    /* 4 normales */
    0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    /* 4 coordonn�es de texture, une par sommet */
    0.0f, 0.0f, 
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};


  GLfloat d3= 100.0, dataLune[]={

		 /* 4 coordonn�es de sommets */
   -d3, 0.0f, 0.0f,
    d3, 0.0f, 0.0f,
   -d3,   2*d3, 0.0f,
    d3,   2*d3, 0.0f,
    /* 4 normales */
    0.0f, 2.0f, 0.0f, 
    0.0f, 2.0f, 0.0f,
    0.0f, 2.0f, 0.0f,
    0.0f, 2.0f, 0.0f,
    /* 4 coordonn�es de texture, une par sommet */
    0.0f, 0.0f, 
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    genVertexArraysOES(8, _vao);
  bindVertexArrayOES(_vao[0]);

  glEnableVertexAttribArray(_vPositionHandle);
  glEnableVertexAttribArray(_vNormalHandle);
  glEnableVertexAttribArray(_vTextureHandle);

  glGenBuffers(1, &buffData);
  glBindBuffer(GL_ARRAY_BUFFER, buffData);
  glBufferData(GL_ARRAY_BUFFER, (W - 1) * (H - 1) * 6 * 8 * sizeof *data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof *data, (const void *)0);
  glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_TRUE,  8 * sizeof *data, (const void *)(3 * sizeof *data));
  glVertexAttribPointer(_vTextureHandle, 2, GL_FLOAT, GL_FALSE, 8 * sizeof *data, (const void *)(6 * sizeof *data));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  bindVertexArrayOES(0);


  //TEXTURE EAU///////////////////////////////////////////////////


  if( (texEau = load_png_asset_into_texture("image/eau.png", texEau)) == 0){
      LOGD("Impossible d'ouvrir le fichier : %s", "image/eau.jpg");
        exit(1);
  }

  //TEXTURE SABLE////////////////////////////////////////////

 if( (texSable = load_png_asset_into_texture("image/sable.png", texSable)) == 0) {
     LOGD("Impossible d'ouvrir le fichier : %s", "image/sable.png");
     exit(1);
 }

  //TEXTURE HERBE////////////////////////////////////////////////

  if( (texHerbe = load_png_asset_into_texture("image/herbe.png", texHerbe)) == 0){
      LOGD("Impossible d'ouvrir le fichier : %s", "image/herbe.png");
    exit(1);
  }

  //TEXTURE ROCHE////////////////////////////////////////////////

  if( (texRoche = load_png_asset_into_texture("image/roche.png", texRoche)) == 0){
      LOGD("Impossible d'ouvrir le fichier : %s", "image/roche.png");
    exit(1);
  }

  //TEXTURE NEIGE////////////////////////////////////////////////

  if( (texNeige = load_png_asset_into_texture("image/neige.png", texNeige)) == 0){
      LOGD("Impossible d'ouvrir le fichier : %s", "image/neige.png");
    exit(1);
  }


//  TEXTURE SOLEIL ///////////////////////////////////////////////////////
  bindVertexArrayOES(_vao[6]);
  glEnableVertexAttribArray(_vPositionHandle);
  glEnableVertexAttribArray(_vNormalHandle);
  glEnableVertexAttribArray(_vTextureHandle);

  glGenBuffers(1, &buffSoleil);
  glBindBuffer(GL_ARRAY_BUFFER, buffSoleil);
  glBufferData(GL_ARRAY_BUFFER, sizeof dataSoleil, dataSoleil, GL_STATIC_DRAW);
  glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_TRUE,  0, (const void *)(4 * 3 * sizeof *dataSoleil));
  glVertexAttribPointer(_vTextureHandle, 2, GL_FLOAT, GL_FALSE, 0, (const void *)(4 * 6 * sizeof *dataSoleil));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
 bindVertexArrayOES(0);


    if( (texSoleil = load_png_asset_into_texture("image/Sun1.png", texSoleil)) == 0 ) {
        LOGD("Impossible d'ouvrir le fichier : %s", "image/Sun1.png");
    exit(1);
  }

//  TEXTURE LUNE ///////////////////////////////////////////////////////

 bindVertexArrayOES(_vao[7]);
  glEnableVertexAttribArray(_vPositionHandle);
  glEnableVertexAttribArray(_vNormalHandle);
  glEnableVertexAttribArray(_vTextureHandle);

  glGenBuffers(1, &buffLune);
  glBindBuffer(GL_ARRAY_BUFFER, buffLune);
  glBufferData(GL_ARRAY_BUFFER, sizeof dataLune, dataLune, GL_STATIC_DRAW);
  glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_TRUE,  0, (const void *)(4 * 3 * sizeof *dataLune));
  glVertexAttribPointer(_vTextureHandle, 2, GL_FLOAT, GL_FALSE, 0, (const void *)(4 * 6 * sizeof *dataLune));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
 bindVertexArrayOES(0);

  if( (texLune = load_png_asset_into_texture("image/Lune.png", texLune)) == 0 ) {
      LOGD("Impossible d'ouvrir le fichier : %s", "image/Lune.png");
    exit(1);
  }

    LOGD("init data fini");
}



/*!\brief Boucle infinie principale : g�re les �v�nements, dessine,
 * imprime le FPS et swap les buffers.
 *
 * \param win le pointeur vers la fen�tre SDL pour laquelle nous avons
 * attach� le contexte OpenGL.
 */
static void draw(GLfloat * eyeViews, GLfloat * eyePerspectives) {
  GLfloat a = 0.0, dt = 0.0, dt1 = 0.0, dt2 = 0.0 , dtheta = M_PI, dtheta2 = 2*M_PI; pas = 5.0;
  t0 = gl4dGetElapsedTime();
  uint32_t t,t3,ti,ti2,t4;



    dt = ((t = gl4dGetElapsedTime()) - t0) / 1000.0;
    dt1 = ((ti = gl4dGetElapsedTime()) - t3) / 20000.0;
    //dt2 = ((ti2 = gl4dGetElapsedTime()) - t4) / 20000.0;
    t0 = t;
    t3 = ti;
    t4 = ti2;
    //manageEvents(win);

    triangle_edge(Pixels, 0, 0, W - 1, H - 1, W);

    I = (int) ((H-1) *(((_cam.z/S)+1)/2));
    Id = ((H-1) *(((_cam.z/S)+1.0)/2.0));

    J = (int) ((W-1) *(((_cam.x/S)+1)/2));
    Jd = ((W-1) *(((_cam.x/S)+1.0)/2.0));

    _cam.y = hauteur(Pixels,(I)*W+(J))+5.0;

//    _Soleil.theta += dt1 * dtheta2;
//    _Soleil.z += -dt1 * 1800 * sin(_Soleil.theta);
//    _Soleil.y += -dt1 * 1800 * cos(_Soleil.theta);
//
//    _Lune.theta += dt1 * dtheta2;
//    _Lune.z += -dt1 * 1800 * sin(_Lune.theta);
//    _Lune.y += -dt1 * 1800 * cos(_Lune.theta );

//    if(_keys[KLEFT]) {
//      _cam.theta += dt * dtheta;
//    }
//    if(_keys[KRIGHT]) {
//      _cam.theta -= dt * dtheta;
//    }
//    if(_keys[KUP]) {
//      _cam.x += -dt * pas * sin(_cam.theta);
//      _cam.z += -dt * pas * cos(_cam.theta);
//    }
//    if(_keys[KDOWN]) {
//      _cam.x += dt * pas * sin(_cam.theta);
//      _cam.z += dt * pas * cos(_cam.theta);
//    }


    if(pasOn == 1){
      pas = 50.0;
    }
    else{
      pas = 5.0;
    }

    loop(eyeViews, eyePerspectives, a + 0);
    gl4duPrintFPS(stderr);

    gl4duUpdateShaders();
    if(!_pause)
      a += 0.1 * 2.0 * M_PI * dt;
}


void printMat(GLfloat * mat, char* name) {
    LOGD("%s0: %.2f  %s1: %.2f  %s2: %.2f "
                 " %s3: %.2f  %s4: %.2f  %s5: %.2f "
                 " %s6: %.2f  %s7: %.2f  %s8: %.2f "
                 " %s9: %.2f  %s10: %.2f %s11: %.2f "
                 " %s12: %.2f  %s13: %.2f %s14: %.2f "
                 "  %s15: %.2f",
         name,mat[0],name,mat[1],name, mat[2],
         name,mat[3],name,mat[4],name,mat[5],
         name,mat[6],name,mat[7],name,mat[8],
         name,mat[9],name, mat[10],name,mat[11],
         name,mat[12], name,mat[13],name,mat[14],
         name,mat[15]);
}


void addvecteur( float * res , float * vect1 , float * vect2){

    res[0] = vect1[0] + vect2[0];
    //res[1] = vect1[1] + vect2[1];
    res[2] = vect1[2] + vect2[2];

}

void rmvecteur( float * res , float * vect1 , float * vect2){

    res[0] = vect1[0]- vect2[0];
    //res[1] = vect1[1]- vect2[1];
    res[2] = vect1[2]- vect2[2];

}

void setCamera(int up, int down, int left, int rightt) {
    GLfloat /*tmp[16], forward[3], side[3],*/position[3];


    position[0] = _cam.x;
    position[1] = _cam.y;
    position[2] = _cam.z;


    LOGD("forx: %.2f   forz: %.2f", (GLfloat) forward1, (GLfloat) forward2);

   // LOGD("sidex: %.2f   sidey: %.2f   sidez: %.2f", right[0], right[1], right[2]);

    if(up) {
        position[0] = position[0] + forward1;

        position[2] = position[2] + forward2;
        //addvecteur(position , position , forward);
    }
    if(down) {

        position[0] = position[0] - forward1;
        //res[1] = vect1[1]- vect2[1];
        position[2] = position[2]- forward2;
        //rmvecteur(position , position , forward);
    }
    if(left) {
        addvecteur(position , position , right);
    }
    if(rightt) {
        rmvecteur(position , position , right);
    }

    _cam.x = position[0];
    //_cam.y = position[1];
    _cam.z = position[2];

    LOGD("camx: %.2f   camy: %.2f   camz: %.2f", _cam.x, _cam.y, _cam.z);

}


/*!\brief Cette fonction dessine dans le contexte OpenGL actif.
 */
static void loop(GLfloat * eyeViews, GLfloat * eyePerspectives, GLfloat a0) {

    static GLfloat temps = 0.0f;
  GLfloat * mv, temp[4] = {1.0, 100*sin(temps), 1.0, 1.0}, lumpos[4];
    temps += 0.01;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  t1 = gl4dGetElapsedTime();

//  if(t1 - t2 > 10000){
//    _activeNight = !_activeNight;
//    t2 = t1;
//  }

    if(_activeNight == 0){
        glUseProgram(_pId[0]);
        glClearColor(0.0f, 0.4f, 0.9f, 0.0f);
    }
    else{
        glUseProgram(_pId[1]);
        glClearColor(0.2f, 0.2f, 0.6f, 0.0f);
    }


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,texEau);
    glUniform1i(glGetUniformLocation(_pId[0], "myTexture0"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,texSable);
    glUniform1i(glGetUniformLocation(_pId[0], "myTexture1"), 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D,texHerbe);
    glUniform1i(glGetUniformLocation(_pId[0], "myTexture2"), 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D,texRoche);
    glUniform1i(glGetUniformLocation(_pId[0], "myTexture3"), 3);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D,texNeige);
    glUniform1i(glGetUniformLocation(_pId[0], "myTexture4"), 4);


    if(_activeNight == 0){
        glUniform1i(glGetUniformLocation(_pId[0], "myTexture"), 0);
        glUniform1i(glGetUniformLocation(_pId[0], "heightMap"), 0);
    }
    else{
        glUniform1i(glGetUniformLocation(_pId[1], "myTexture"), 0);
        glUniform1i(glGetUniformLocation(_pId[1], "heightMap"), 0);
    }

    gl4duBindMatrix("modelViewMatrix");
    gl4duLoadIdentityf();


    MMAT4INVERSE(eyeViews);


    //printMat(headview, "headview");

    gl4duMultMatrixf(eyeViews);

    //gl4duLookAtf(_cam.x, _cam.y, _cam.z, forward[0]+_cam.x, forward[1]+_cam.y, forward[2]+_cam.z, up[0], up[1], up[2]);

    gl4duTranslatef(_cam.x, -_cam.y, _cam.z);


    gl4duRotatef(180, 0, 0, 1);

    glUniformMatrix4fv(glGetUniformLocation(_pId[0], "perspective"), 1, GL_TRUE, eyePerspectives);

    mv = gl4duGetMatrixData();
    MMAT4XVEC4(lumpos, mv, temp);

    glUniform4fv(glGetUniformLocation(_pId[0], "lumpos"), 1, lumpos);

  gl4duSendMatrices();
  bindVertexArrayOES(_vao[0]);
   // glBindBuffer(GL_ARRAY_BUFFER, buffData);
  glDrawArrays(GL_TRIANGLES, 0, W * H * 8);

  gl4duPopMatrix();


 bindVertexArrayOES(0);
}

static void quit() {
    if(_vao[0]) {
        glDeleteBuffers(8, _vao);
        _vao[0] = 0;
    }
    if(_tId[0]) {
        glDeleteTextures(sizeof _tId / sizeof *_tId, _tId);
        _tId[0] = 0;
    }
    gl4duClean(GL4DU_ALL);
    _pId[0] = _pId[1] = 0;
}


JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_init(JNIEnv * env, jobject obj
        , jobject assetManager, jstring vshader, jstring fshader, jstring toonshader, jstring fnightbasicshader, jstring fnightbasictoonshader) {
    LOGD("Init java");

    asset_manager = AAssetManager_fromJava(env, assetManager);

    char * vs = (*env)->GetStringUTFChars(env, vshader, NULL);
    char * fs = (*env)->GetStringUTFChars(env, fshader, NULL);
    char * toonss = (*env)->GetStringUTFChars(env, toonshader, NULL);
    char * fnights = (*env)->GetStringUTFChars(env, fnightbasicshader, NULL);
    char * fnighttoons = (*env)->GetStringUTFChars(env, fnightbasictoonshader, NULL);

    init(vs, fs, toonss, fnights, fnighttoons);
    (*env)->ReleaseStringUTFChars(env, vshader, vs);
    (*env)->ReleaseStringUTFChars(env, fshader, fs);
    (*env)->ReleaseStringUTFChars(env, toonshader, toonss);
    (*env)->ReleaseStringUTFChars(env, fnightbasicshader, fnights);
    (*env)->ReleaseStringUTFChars(env, fnightbasictoonshader, fnighttoons);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_reshape(JNIEnv * env, jobject obj,  jint width, jint height) {
    _windowWidth  = width;
    _windowHeight = height;

    reshape();
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_draw(JNIEnv * env, jobject obj, jfloatArray eyeView, jfloatArray eyePerspective) {
    eyeViews = (*env)->GetFloatArrayElements(env, eyeView, NULL);
    eyePerspectives = (*env)->GetFloatArrayElements(env, eyePerspective, NULL);

    draw(eyeViews, eyePerspectives);

    (*env)->ReleaseFloatArrayElements(env, eyeView, eyeViews, 0);
    (*env)->ReleaseFloatArrayElements(env, eyePerspective, eyePerspectives, 0);
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_setcamera(JNIEnv * env, jobject obj
        , jfloatArray headviewv,jfloatArray forwardv, jfloatArray upv, jfloatArray rightv, jfloat forward1v, jfloat forward2v) {

    forward1 = forward1v; forward2 = forward2v;

    headview = (*env)->GetFloatArrayElements(env, headviewv, NULL);
    forward = (*env)->GetFloatArrayElements(env, forwardv, NULL);
    up = (*env)->GetFloatArrayElements(env, upv, NULL);
    right = (*env)->GetFloatArrayElements(env, rightv, NULL);

    (*env)->ReleaseFloatArrayElements(env, headviewv, headview, 0);
    (*env)->ReleaseFloatArrayElements(env, forwardv, forward, 0);
    (*env)->ReleaseFloatArrayElements(env, upv, up, 0);
    (*env)->ReleaseFloatArrayElements(env, rightv, right, 0);

}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_event(JNIEnv * env, jobject obj
        ,  jint x_left, jint z_up,  jint x_right, jint z_down) {


    //setCamera(z_up,z_down,x_left,x_right);

    _cam.x-=x_right;
    _cam.z+=z_up;
    _cam.x+=x_left;
    _cam.z-=z_down;
}

JNIEXPORT void JNICALL Java_com_android_androidGL4D_AGL4DLib_quit(JNIEnv * env, jobject obj) {
    quit();
}