#include <stdlib.h>
#include "md2.h"
#include <ctype.h>
#include <string>

/* Table of precalculated normals */
vec3_t anorms_table[162] = {
#include "anorms.h"
};

int MD2::Load( const char *mFile, const char *txFile ) {
    // Load Texture
    Texture tx ;
    if ( !tx.Load( txFile ) ) {

		fprintf(stderr, "Error: couldn't open texture : \"%s\"!\n", txFile);
        return 0 ; 
    }
	tex_id = tx.id ;

    // Load Model File
    FILE *fp;
    int i;

    fp = fopen ( mFile, "rb");
    if (!fp)
    {
      fprintf (stderr, "Error: couldn't open \"%s\"!\n", mFile);
      return 0;
    }

  /* Read header */
  fread (&header, 1, sizeof (struct md2_header_t), fp);

  if ((header.ident != 844121161) ||
      (header.version != 8))
    {
      /* Error! */
      fprintf (stderr, "Error: bad version or identifier\n");
      fclose (fp);
      return 0;
    }

  /* Memory allocations */
   skins = (struct md2_skin_t *)
   malloc (sizeof (struct md2_skin_t) * header.num_skins);
   texcoords = (struct md2_texCoord_t *) malloc (sizeof (struct md2_texCoord_t) *  header.num_st);
   triangles = (struct md2_triangle_t *) malloc (sizeof (struct md2_triangle_t) *  header.num_tris);
   frames = (struct md2_frame_t *) malloc (sizeof (struct md2_frame_t) *  header.num_frames);
   glcmds = (int *) malloc (sizeof (int) *  header.num_glcmds);

  /* Read model data */
  fseek (fp,  header.offset_skins, SEEK_SET);
  fread ( skins, sizeof (struct md2_skin_t), header.num_skins, fp);
  
  fseek (fp,  header.offset_st, SEEK_SET);
  fread ( texcoords, sizeof (struct md2_texCoord_t), header.num_st, fp);

  fseek (fp,  header.offset_tris, SEEK_SET);
  fread ( triangles, sizeof (struct md2_triangle_t), header.num_tris, fp);

  fseek (fp,  header.offset_glcmds, SEEK_SET);
  fread ( glcmds, sizeof (int),  header.num_glcmds, fp);

  /* Read frames */
  fseek (fp,  header.offset_frames, SEEK_SET);
  for (i = 0; i <  header.num_frames; ++i)
    {
      /* Memory allocation for vertices of this frame */
       frames[i].verts = (struct md2_vertex_t *) malloc (sizeof (struct md2_vertex_t) *  header.num_vertices);

      /* Read frame data */
      fread ( frames[i].scale, sizeof (vec3_t), 1, fp);
      fread ( frames[i].translate, sizeof (vec3_t), 1, fp);
      fread ( frames[i].name, sizeof (char), 16, fp);
      fread ( frames[i].verts, sizeof (struct md2_vertex_t), header.num_vertices, fp);
    }

  fclose (fp);
  ParseAnimations();
  return 1;
}

MD2::~MD2() {
      int i;

  if ( skins)
    {
      free ( skins);
       skins = NULL;
    }

  if ( texcoords)
    {
       free ( texcoords);
       texcoords = NULL;
    }

  if ( triangles)
    {
      free ( triangles);
       triangles = NULL;
    }

  if ( glcmds)
    {
      free ( glcmds);
       glcmds = NULL;
    }

  if ( frames)
    {
      for (i = 0; i <  header.num_frames; ++i)
    {
      free ( frames[i].verts);
       frames[i].verts = NULL;
    }

      free ( frames);
       frames = NULL;
    }
}


void MD2::RenderFrame( int n1, int start1, int end1, 
	                   int n2, int start2, int end2,
					   float interp, float interpFrames, bool fwd ) {
	/**
 * Render the model with interpolation between frame n and n+1
 * using model's GL command list.
 * interp is the interpolation percent. (from 0.0 to 1.0)
 */
  int i, *pglcmds;
  vec3_t v_curr, v_next, v, norm;
  float *n_curr, *n_next;
  struct md2_frame_t *pframe1, *pframe2;
  struct md2_vertex_t *pvert1, *pvert2;

  vec3_t v_curr2, v_next2, v2, norm2;
  float *n_curr2, *n_next2;
  struct md2_frame_t *pframe12, *pframe22;
  struct md2_vertex_t *pvert12, *pvert22;

  struct md2_glcmd_t *packet;


  /* Check if n is in a valid range */
  if ((n1 < 0) || (n1 >=  header.num_frames  ))
    return;

  /* Enable model's texture */
  glBindTexture (GL_TEXTURE_2D,  tex_id);

  /* pglcmds points at the start of the command list */
  pglcmds =  glcmds;
  int next1, next2 = n2 ;
  if ( fwd ) {
  if ( n1  == end1 -1 ) next1 = start1 ;
	  else next1 = n1 + 1 ;

  if ( prev.active ) { 
         if ( n2  == end2 - 1  ) next2 = start2 ;
	     else next2 = n2 + 1 ;
	  }
  } 
 
  
   //printf( "%d %d\n", n, next ) ;

  /* Draw the model */
  while ((i = *(pglcmds++)) != 0)
    {
      if (i < 0)
	{
	  glBegin (GL_TRIANGLE_FAN);
	  i = -i;
	}
      else
	{
	  glBegin (GL_TRIANGLE_STRIP);
	}
	 
      /* Draw each vertex of this group */
      for (/* Nothing */; i > 0; --i, pglcmds += 3)
	{
	  packet = (struct md2_glcmd_t *)pglcmds;
	  pframe1 = & frames[n1];
	  pframe2 = & frames[next1];
	  pvert1 = &pframe1->verts[packet->index];
	  pvert2 = &pframe2->verts[packet->index];

	  if ( prev.active ) {
	  pframe12 = & frames[n2];
	  pframe22 = & frames[next2];
	  pvert12 = &pframe12->verts[packet->index];
	  pvert22 = &pframe22->verts[packet->index];
	  }


	  /* Pass texture coordinates to OpenGL */
	  glTexCoord2f (packet->s, 1 - packet->t);

	  /* Interpolate normals */
	  n_curr = anorms_table[pvert1->normalIndex];
	  n_next = anorms_table[pvert2->normalIndex];

	  norm[0] = n_curr[0] + interp * (n_next[0] - n_curr[0]);
	  norm[1] = n_curr[1] + interp * (n_next[1] - n_curr[1]);
	  norm[2] = n_curr[2] + interp * (n_next[2] - n_curr[2]);

	  glNormal3fv (norm);

	  /* Interpolate vertices */
	  v_curr[0] = pframe1->scale[0] * pvert1->v[0] + pframe1->translate[0];
	  v_curr[1] = pframe1->scale[1] * pvert1->v[1] + pframe1->translate[1];
	  v_curr[2] = pframe1->scale[2] * pvert1->v[2] + pframe1->translate[2];

	  v_next[0] = pframe2->scale[0] * pvert2->v[0] + pframe2->translate[0];
	  v_next[1] = pframe2->scale[1] * pvert2->v[1] + pframe2->translate[1];
	  v_next[2] = pframe2->scale[2] * pvert2->v[2] + pframe2->translate[2];

	  
	  v[0] = v_curr[0] + interp * (v_next[0] - v_curr[0]);
	  v[1] = v_curr[1] + interp * (v_next[1] - v_curr[1]);
	  v[2] = v_curr[2] + interp * (v_next[2] - v_curr[2]);

	  if ( prev.active ) {
	  v_curr2[0] = pframe12->scale[0] * pvert12->v[0] + pframe12->translate[0];
	  v_curr2[1] = pframe12->scale[1] * pvert12->v[1] + pframe12->translate[1];
	  v_curr2[2] = pframe12->scale[2] * pvert12->v[2] + pframe12->translate[2];

	  v_next2[0] = pframe22->scale[0] * pvert22->v[0] + pframe22->translate[0];
	  v_next2[1] = pframe22->scale[1] * pvert22->v[1] + pframe22->translate[1];
	  v_next2[2] = pframe22->scale[2] * pvert22->v[2] + pframe22->translate[2];

	  v2[0] = v_curr2[0] + interp * (v_next2[0] - v_curr2[0]);
	  v2[1] = v_curr2[1] + interp * (v_next2[1] - v_curr2[1]);
	  v2[2] = v_curr2[2] + interp * (v_next2[2] - v_curr2[2]);
	   
	  //interpFrame = 1.0 ; 

	  v[0] = v2[0] + interpFrames * ( v[0] - v2[0] ) ;
	  v[1] = v2[1] + interpFrames * ( v[1] - v2[1] ) ;
	  v[2] = v2[2] + interpFrames * ( v[2] - v2[2] ) ;
	  }
	  glVertex3fv (v);
	}

      glEnd ();
	 
    }
}

void MD2::Animate2 (int start, int end, int *frame, float *interp, bool fwd)
{
  if ( attach ) {
	  *frame = parent->curr.frame ;
	  return ;
  }
  if ((*frame < start) || (*frame > end - 1))
    *frame = start;
   if (*interp >= 1.0f)
    {
      /* Move to next frame */
      *interp = 0.0f;
	  if ( fwd ) {
          (*frame)++;
		  if (*frame > end - 1    )
	         *frame = start;
      } 
	   else {
		   (*frame)--;
		   if ( *frame < start ) 
			   *frame = end - 1 ;
      }

      
    }
}



void MD2::Show( int frame ) {
	//RenderFrame( frame, 0, header.num_frames, 0 ) ; 
}

void MD2::ParseAnimations()
{
	tAnimationInfo animation;
	string strLastName = "";
		
	// Go through all of the frames of animation and parse each animation
	for(int i = 0; i < header.num_frames; i++)
	{
		// Assign the name of this frame of animation to a string object
		string strName  = frames[i].name;
		//printf( "Frame %d : %s\n", i ,strName.c_str() ) ;
		int frameNum = 0;
		
		// Go through and extract the frame numbers and erase them from the name
		for(int j = 0; j < strName.length(); j++)
		{
			// If the current index is a number and it's one of the last 2 characters of the name
			if( isdigit(strName[j]) && j >= strName.length() - 2)
			{
				// Use a C function to convert the character to a integer.
				// Notice we use the address to pass in the current character and on
				frameNum = atoi(&strName[j]);

				// Erase the frame number from the name so we extract the animation name
				strName.erase(j, strName.length() - j);
				break;
			}
		}

		// Check if this animation name is not the same as the last frame,
		// or if we are on the last frame of animation for this model
		if(strName != strLastName || i == header.num_frames - 1)
		{
			// If this animation frame is NOT the first frame
			if(strLastName != "")
			{
				// Copy the last animation name into our new animation's name
				strcpy(animation.strName, strLastName.c_str());

				// Set the last frame of this animation to i
				animation.endFrame = i;

				// Add the animation to our list and reset the animation object for next time
				pAnimations.push_back(animation);
				memset(&animation, 0, sizeof(tAnimationInfo));

				// Increase the number of animations for this model
				numOfAnimations++;
			}

			// Set the starting frame number to the current frame number we just found,
			// minus 1 (since 0 is the first frame) and add 'i'.
			animation.startFrame = frameNum - 1 + i;
		}

		// Store the current animation name in the strLastName string to check it latter
		strLastName = strName;
	}
}

void MD2::Play(bool loop, bool forward) {
    double current = (double) glutGet( GLUT_ELAPSED_TIME ) / 1000.0 ;
	float diff = current - last_time ;
	last_time = current ;
	
	if ( loop == false && curr.frame == pAnimations[curr.action].endFrame - 1  ) {
		prev.active = 1.0;
		over = true ;
		RenderFrame(  curr.frame, pAnimations[curr.action].startFrame, pAnimations[curr.action].endFrame,
		              curr.frame, pAnimations[curr.action].startFrame, pAnimations[curr.action].endFrame ,
		              interp, interpFrame, forward  );
		return;
	}

	if ( attach ) {
		curr = parent->curr;
		prev = parent->prev ;
		RenderFrame(  curr.frame, pAnimations[curr.action].startFrame, pAnimations[curr.action].endFrame,
		         prev.frame , pAnimations[prev.action].startFrame, pAnimations[prev.action].endFrame ,
		         parent->interp, parent->interpFrame, forward  );
		return ;
	}

	if ( prev.active ) {
	if ( interpFrame < 1.0  ) {
		interpFrame += diff * fps   ;
		Animate2( pAnimations[prev.action].startFrame, pAnimations[prev.action].endFrame , &prev.frame, &interp, forward ) ;
    } else {
		   prev.active = 0 ;
		   //interpFrame = 0;
    } 
	} else {
	  interp += diff * fps  ;
	  Animate2( pAnimations[curr.action].startFrame, pAnimations[curr.action].endFrame , &curr.frame, &interp , forward) ;
	 
	}
	
	RenderFrame(  curr.frame, pAnimations[curr.action].startFrame, pAnimations[curr.action].endFrame,
		         prev.frame , pAnimations[prev.action].startFrame, pAnimations[prev.action].endFrame ,
		         interp, interpFrame, forward  );
}

void MD2::Do( int newSt ) {
	if ( prev.active == 0 ) {
	 prev = curr ;
	 curr.action = newSt ;
	 curr.active = 1;
	 curr.frame = pAnimations[newSt].startFrame;
	 interpFrame = 0.0f ;
	 over = false;
	}
}