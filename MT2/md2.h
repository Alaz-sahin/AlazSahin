#ifndef  __CTIS_MD2__
#define  __CTIS_MD2__

#include "texture.h"
#include <vector>

using namespace std ;

/* MD2 header */
struct md2_header_t
{
  int ident;                  /* magic number: "IDP2" */
  int version;                /* version: must be 8 */

  int skinwidth;              /* texture width */
  int skinheight;             /* texture height */

  int framesize;              /* size in bytes of a frame */

  int num_skins;              /* number of skins */
  int num_vertices;           /* number of vertices per frame */
  int num_st;                 /* number of texture coordinates */
  int num_tris;               /* number of triangles */
  int num_glcmds;             /* number of opengl commands */
  int num_frames;             /* number of frames */

  int offset_skins;           /* offset skin data */
  int offset_st;              /* offset texture coordinate data */
  int offset_tris;            /* offset triangle data */
  int offset_frames;          /* offset frame data */
  int offset_glcmds;          /* offset OpenGL command data */
  int offset_end;             /* offset end of file */
};

/* Vector */
typedef float vec3_t[3];
/* Texture name */
struct md2_skin_t
{
  char name[64];              /* texture file name */
};
/* Texture coords */
struct md2_texCoord_t
{
  short s;
  short t;
};
/* Triangle info */
struct md2_triangle_t
{
  unsigned short vertex[3];   /* vertex indices of the triangle */
  unsigned short st[3];       /* tex. coord. indices */
};
/* Compressed vertex */
struct md2_vertex_t
{
  unsigned char v[3];         /* position */
  unsigned char normalIndex;  /* normal vector index */
};
/* Model frame */
struct md2_frame_t
{
  vec3_t scale;               /* scale factor */
  vec3_t translate;           /* translation vector */
  char name[16];              /* frame name */
  struct md2_vertex_t *verts; /* list of frame's vertices */
};
/* GL command packet */
struct md2_glcmd_t
{
  float s;
  float t;
  int index;
};

struct tAnimationInfo
{
	char strName[255];			// This stores the name of the animation (Jump, Pain, etc..)
	int startFrame;				// This stores the first frame number for this animation
	int endFrame;				// This stores the last frame number for this animation
};

struct state_t {
	int frame ;
	int next ;
	int action ;
	int active ;
} ;

class MD2 {
private:
	  struct md2_header_t header;

	  struct md2_skin_t *skins;
	  struct md2_texCoord_t *texcoords;
	  struct md2_triangle_t *triangles;
	  struct md2_frame_t *frames;
	  int *glcmds;

	  GLuint tex_id;

	  double last_time ; 
	  
	  float interp ;
	  float interpFrame ;
	  int fps ; 
	  int numOfAnimations ;
    
	  void Animate2(int start, int end, int *frame, float *interp, bool fwd) ; 
	  // State Machine for Player
	  bool attach  ;
	  MD2  *parent ; 
	  
public:
	  bool over ;
	  //bool trans ; 
	  state_t curr, prev ; 
	  vector<tAnimationInfo> pAnimations;
	  MD2(int f = 10) { 
		  last_time = 0.0; 
		  fps = f ; numOfAnimations = 0; 
		  curr.action = 0 ; // idle state
		  curr.active = 1 ; // active
		  curr.frame = 0 ;
		  prev.active = 0 ; // not active 
		  interpFrame = 0.0f ; 
		  attach = false ;
		  parent = NULL ;
		  over = false ;

	  }
	  int Load( const char *mFile, const char *txFile ) ; 
	  void RenderFrame( int n1, int start1, int end1, 
	                   int n2, int start2, int end2,
					   float interp, float interpFrames, bool fwd );
	  void Show(int frame = 0) ; 
	  void ParseAnimations() ;
	  void Play( bool loop = true , bool forward = true) ; 
	  void Do( int newSt ) ;
	  int  Do() { return curr.action; }
	  void Fps( int f ) { fps = f; }
	  int  Fps() { return fps; }
	  void Attach( MD2 *p ) { attach = true ; parent = p;  } ; 
	  float Rate() {
		   return ((curr.frame - pAnimations[curr.action].startFrame ) * 1.0 + interp) / (pAnimations[curr.action].endFrame - pAnimations[curr.action].startFrame - 1) ; 
       }; // 0.0 at the beginning, 1.0 at the end, 0.5 at the middle of the current animation.
	  ~MD2() ;
};


#endif