//hq2x filter demo program
//----------------------------------------------------------
//Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

/**
 **	Hq2x.cc
 **
 **	Adapted for Exult: 4/7/07 - JSF
 **/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_video.h"

#include <stdlib.h>
#include <string.h>

/*
 *	Common defines and functions.
 */
static int   YUV1, YUV2;
const  int   Ymask = 0x00FF0000;
const  int   Umask = 0x0000FF00;
const  int   Vmask = 0x000000FF;
const  int   trY   = 0x00300000;
const  int   trU   = 0x00000700;
const  int   trV   = 0x00000006;

template<class Dest_pixel, class Manip_pixels>
inline void StoreRGB(Dest_pixel *pc, int c, const Manip_pixels& manip)
{
	*pc = manip.rgb((c&0xff0000)>>16, (c&0xff00)>>8, c&0xff);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp1(Dest_pixel *pc, int c1, int c2, const Manip_pixels& manip)
{
  StoreRGB(pc, (c1*3+c2) >> 2, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp2(Dest_pixel *pc, int c1, int c2, int c3, 
						const Manip_pixels& manip)
{
  StoreRGB(pc, (c1*2+c2+c3) >> 2, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp3(Dest_pixel *pc, int c1, int c2,
						const Manip_pixels& manip)
{
  //*((int*)pc) = (c1*7+c2)/8;

  StoreRGB(pc, ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3,
						manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp4(Dest_pixel *pc, int c1, int c2, int c3,
						const Manip_pixels& manip)
{
  //*((int*)pc) = (c1*2+(c2+c3)*7)/16;

  StoreRGB(pc, ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) 
							& 0x000FF000) +
               (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) 
							& 0x0FF00FF0)) >> 4,
								manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp5(Dest_pixel *pc, int c1, int c2, const Manip_pixels& manip)
{
  StoreRGB(pc, (c1+c2) >> 1, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp6(Dest_pixel *pc, int c1, int c2, int c3, 
						const Manip_pixels& manip)
{
  //StoreRGB(pc, (c1*5+c2*2+c3)/8);

  StoreRGB(pc, ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*2 + 
		(c3 & 0x00FF00) ) & 0x0007F800) +
                (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*2 + 
		(c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp7(Dest_pixel *pc, int c1, int c2, int c3, 
						const Manip_pixels& manip)
{
  //StoreRGB(pc, (c1*6+c2+c3)/8);

  StoreRGB(pc, ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + 
		(c3 & 0x00FF00) ) & 0x0007F800) +
                (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + 
		(c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp9(Dest_pixel *pc, int c1, int c2, int c3, 
						const Manip_pixels& manip)
{
  //StoreRGB(pc, (c1*2+(c2+c3)*3)/8);

  StoreRGB(pc, ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + 
		(c3 & 0x00FF00))*3 ) & 0x0007F800) +
                (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + 
		(c3 & 0xFF00FF))*3 ) & 0x07F807F8)) >> 3, manip);
}

template<class Dest_pixel, class Manip_pixels>
inline void Interp10(Dest_pixel *pc, int c1, int c2, int c3, 
						const Manip_pixels& manip)
{
  //StoreRGB(pc, (c1*14+c2+c3)/16);

  StoreRGB(pc, ((((c1 & 0x00FF00)*14 + (c2 & 0x00FF00) + 
		(c3 & 0x00FF00) ) & 0x000FF000) +
                (((c1 & 0xFF00FF)*14 + (c2 & 0xFF00FF) + 
		(c3 & 0xFF00FF) ) & 0x0FF00FF0)) >> 4, manip);
}

#define PTYPES	      Dest_pixel,Manip_pixels

static int RGBtoYUV(unsigned int r, unsigned int g, unsigned int b)
{
	int Y, u, v;
	Y = (r + g + b) >> 2;
	u = 128 + ((r - b) >> 2);
	v = 128 + ((-r + 2*g -b)>>3);
	return (Y<<16) + (u<<8) + v;
}

inline bool Diff(int YUV1, int YUV2)
{
  return ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
           ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
           ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}

template<class Manip_pixels>
inline int hqx_init(int *w, int *c, int *yuv, unsigned char *from,
	int x, int sline_pixels, int prevline, int nextline, 
	Manip_pixels& manip)
{
  int k;

  w[2] = *((from + prevline));
  w[5] = *(from);
  w[8] = *((from + nextline));

  if (x>0)
  {
    w[1] = *((from + prevline - 1));
    w[4] = *((from - 1));
    w[7] = *((from + nextline - 1));
  }
  else
  {
    w[1] = w[2];
    w[4] = w[5];
    w[7] = w[8];
  }

  if (x<sline_pixels-1)
  {
    w[3] = *((from + prevline + 1));
    w[6] = *((from + 1));
    w[9] = *((from + nextline + 1));
  }
  else
  {
    w[3] = w[2];
    w[6] = w[5];
    w[9] = w[8];
  }

  for (k=1; k<=9; k++) {
	unsigned int r, g, b;
	manip.split_source(w[k], r, g, b);
	// The following is so the Interp routines work correctly.
	r &= ~3; g &= ~3;
    c[k] = (r<<16) + (g<<8) + b;;
	yuv[k] = RGBtoYUV(r, g, b);
  }

  int pattern = 0;
  int flag = 1;

  YUV1 = yuv[5];

  for (k=1; k<=9; k++)
  {
    if (k==5) continue;

    if ( w[k] != w[5] )
    {
      YUV2 = yuv[k];
      if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
       ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
       ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
    pattern |= flag;
    }
    flag <<= 1;
  }
  return pattern;
}

/*
 *	2X scaler.
 */
#define PIXEL00_0     StoreRGB<PTYPES>(to, c[5], manip);
#define PIXEL00_10    Interp1<PTYPES>(to, c[5], c[1], manip);
#define PIXEL00_11    Interp1<PTYPES>(to, c[5], c[4], manip);
#define PIXEL00_12    Interp1<PTYPES>(to, c[5], c[2], manip);
#define PIXEL00_20    Interp2<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_21    Interp2<PTYPES>(to, c[5], c[1], c[2], manip);
#define PIXEL00_22    Interp2<PTYPES>(to, c[5], c[1], c[4], manip);
#define PIXEL00_60    Interp6<PTYPES>(to, c[5], c[2], c[4], manip);
#define PIXEL00_61    Interp6<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_70    Interp7<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_90    Interp9<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_100   Interp10<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL01_0     StoreRGB<PTYPES>(to+1, c[5], manip);
#define PIXEL01_10    Interp1<PTYPES>(to+1, c[5], c[3], manip);
#define PIXEL01_11    Interp1<PTYPES>(to+1, c[5], c[2], manip);
#define PIXEL01_12    Interp1<PTYPES>(to+1, c[5], c[6], manip);
#define PIXEL01_20    Interp2<PTYPES>(to+1, c[5], c[2], c[6], manip);
#define PIXEL01_21    Interp2<PTYPES>(to+1, c[5], c[3], c[6], manip);
#define PIXEL01_22    Interp2<PTYPES>(to+1, c[5], c[3], c[2], manip);
#define PIXEL01_60    Interp6<PTYPES>(to+1, c[5], c[6], c[2], manip);
#define PIXEL01_61    Interp6<PTYPES>(to+1, c[5], c[2], c[6], manip);
#define PIXEL01_70    Interp7<PTYPES>(to+1, c[5], c[2], c[6], manip);
#define PIXEL01_90    Interp9<PTYPES>(to+1, c[5], c[2], c[6], manip);
#define PIXEL01_100   Interp10<PTYPES>(to+1, c[5], c[2], c[6], manip);
#define PIXEL10_0     StoreRGB<PTYPES>(to+dline_pixels, c[5], manip);
#define PIXEL10_10    Interp1<PTYPES>(to+dline_pixels, c[5], c[7], manip);
#define PIXEL10_11    Interp1<PTYPES>(to+dline_pixels, c[5], c[8], manip);
#define PIXEL10_12    Interp1<PTYPES>(to+dline_pixels, c[5], c[4], manip);
#define PIXEL10_20    Interp2<PTYPES>(to+dline_pixels, c[5], c[8], c[4], manip);
#define PIXEL10_21    Interp2<PTYPES>(to+dline_pixels, c[5], c[7], c[4], manip);
#define PIXEL10_22    Interp2<PTYPES>(to+dline_pixels, c[5], c[7], c[8], manip);
#define PIXEL10_60    Interp6<PTYPES>(to+dline_pixels, c[5], c[4], c[8], manip);
#define PIXEL10_61    Interp6<PTYPES>(to+dline_pixels, c[5], c[8], c[4], manip);
#define PIXEL10_70    Interp7<PTYPES>(to+dline_pixels, c[5], c[8], c[4], manip);
#define PIXEL10_90    Interp9<PTYPES>(to+dline_pixels, c[5], c[8], c[4], manip);
#define PIXEL10_100   Interp10<PTYPES>(to+dline_pixels, c[5], c[8], c[4], manip);
#define PIXEL11_0     StoreRGB<PTYPES>(to+dline_pixels+1, c[5], manip);
#define PIXEL11_10    Interp1<PTYPES>(to+dline_pixels+1, c[5], c[9], manip);
#define PIXEL11_11    Interp1<PTYPES>(to+dline_pixels+1, c[5], c[6], manip);
#define PIXEL11_12    Interp1<PTYPES>(to+dline_pixels+1, c[5], c[8], manip);
#define PIXEL11_20    Interp2<PTYPES>(to+dline_pixels+1, c[5], c[6], c[8], manip);
#define PIXEL11_21    Interp2<PTYPES>(to+dline_pixels+1, c[5], c[9], c[8], manip);
#define PIXEL11_22    Interp2<PTYPES>(to+dline_pixels+1, c[5], c[9], c[6], manip);
#define PIXEL11_60    Interp6<PTYPES>(to+dline_pixels+1, c[5], c[8], c[6], manip);
#define PIXEL11_61    Interp6<PTYPES>(to+dline_pixels+1, c[5], c[6], c[8], manip);
#define PIXEL11_70    Interp7<PTYPES>(to+dline_pixels+1, c[5], c[6], c[8], manip);
#define PIXEL11_90    Interp9<PTYPES>(to+dline_pixels+1, c[5], c[6], c[8], manip);
#define PIXEL11_100   Interp10<PTYPES>(to+dline_pixels+1, c[5], c[6], c[8], manip);

template <class Dest_pixel, class Manip_pixels>
void Scale_Hq2x
	(
	unsigned char *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	)
{
  int  i, j, k;
  int  prevline, nextline;
  int  w[10];
  int  c[10];
  int  yuv[10];
#if 0
srcx = srcy=0; srcw=sline_pixels; srch=sheight;//++++TESTING
#endif
  int stopy = srcy + srch, stopx = srcx + srcw;
  unsigned char *from = source + srcy*sline_pixels + srcx;
  Dest_pixel *to = dest + 2*srcy*dline_pixels + 2*srcx;

  if (stopx > sline_pixels)
	stopx = sline_pixels;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j=srcy; j<stopy; j++)
  {
    unsigned char *from0 = from;		// Save start of line.
    Dest_pixel *to0 = to;

    if (j>0)      prevline = -sline_pixels; else prevline = 0;
    if (j<sheight-1) nextline =  sline_pixels; else nextline = 0;

    for (i=srcx; i<stopx; i++)
    {
      int pattern = hqx_init(w, c, yuv, from, i, sline_pixels, 
					prevline, nextline, manip);
      switch (pattern)
      {
        case 0:
        case 1:
        case 4:
        case 32:
        case 128:
        case 5:
        case 132:
        case 160:
        case 33:
        case 129:
        case 36:
        case 133:
        case 164:
        case 161:
        case 37:
        case 165:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_22
          PIXEL01_21
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_20
          PIXEL01_22
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_21
          PIXEL11_22
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_21
          PIXEL01_20
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_11
          PIXEL01_21
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_22
          PIXEL01_12
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_20
          PIXEL01_11
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_20
          PIXEL01_22
          PIXEL10_20
          PIXEL11_12
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_21
          PIXEL11_11
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_12
          PIXEL11_22
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_21
          PIXEL01_20
          PIXEL10_11
          PIXEL11_20
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_12
          PIXEL01_20
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_20
          PIXEL01_22
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_21
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 66:
        {
          PIXEL00_22
          PIXEL01_21
          PIXEL10_21
          PIXEL11_22
          break;
        }
        case 24:
        {
          PIXEL00_21
          PIXEL01_22
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_20
          PIXEL01_11
          PIXEL10_20
          PIXEL11_12
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_12
          PIXEL01_20
          PIXEL10_11
          PIXEL11_20
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_20
          PIXEL01_22
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_21
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_11
            PIXEL01_10
          }
          else
          {
            PIXEL00_60
            PIXEL01_90
          }
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 146:
        case 178:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
            PIXEL11_12
          }
          else
          {
            PIXEL01_90
            PIXEL11_61
          }
          PIXEL10_20
          break;
        }
        case 84:
        case 85:
        {
          PIXEL00_20
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL01_11
            PIXEL11_10
          }
          else
          {
            PIXEL01_60
            PIXEL11_90
          }
          PIXEL10_21
          break;
        }
        case 112:
        case 113:
        {
          PIXEL00_20
          PIXEL01_22
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL10_12
            PIXEL11_10
          }
          else
          {
            PIXEL10_61
            PIXEL11_90
          }
          break;
        }
        case 200:
        case 204:
        {
          PIXEL00_21
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
            PIXEL11_11
          }
          else
          {
            PIXEL10_90
            PIXEL11_60
          }
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_12
            PIXEL10_10
          }
          else
          {
            PIXEL00_61
            PIXEL10_90
          }
          PIXEL01_20
          PIXEL11_22
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
            PIXEL10_11
          }
          else
          {
            PIXEL00_90
            PIXEL10_60
          }
          PIXEL01_21
          PIXEL11_20
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
            PIXEL01_12
          }
          else
          {
            PIXEL00_90
            PIXEL01_61
          }
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 67:
        {
          PIXEL00_11
          PIXEL01_21
          PIXEL10_21
          PIXEL11_22
          break;
        }
        case 70:
        {
          PIXEL00_22
          PIXEL01_12
          PIXEL10_21
          PIXEL11_22
          break;
        }
        case 28:
        {
          PIXEL00_21
          PIXEL01_11
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 152:
        {
          PIXEL00_21
          PIXEL01_22
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 194:
        {
          PIXEL00_22
          PIXEL01_21
          PIXEL10_21
          PIXEL11_11
          break;
        }
        case 98:
        {
          PIXEL00_22
          PIXEL01_21
          PIXEL10_12
          PIXEL11_22
          break;
        }
        case 56:
        {
          PIXEL00_21
          PIXEL01_22
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 25:
        {
          PIXEL00_12
          PIXEL01_22
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_21
          PIXEL01_22
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 27:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_10
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 86:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_21
          PIXEL11_10
          break;
        }
        case 216:
        {
          PIXEL00_21
          PIXEL01_22
          PIXEL10_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 106:
        {
          PIXEL00_10
          PIXEL01_21
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 30:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 210:
        {
          PIXEL00_22
          PIXEL01_10
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 120:
        {
          PIXEL00_21
          PIXEL01_22
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_10
          break;
        }
        case 75:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          PIXEL10_10
          PIXEL11_22
          break;
        }
        case 29:
        {
          PIXEL00_12
          PIXEL01_11
          PIXEL10_22
          PIXEL11_21
          break;
        }
        case 198:
        {
          PIXEL00_22
          PIXEL01_12
          PIXEL10_21
          PIXEL11_11
          break;
        }
        case 184:
        {
          PIXEL00_21
          PIXEL01_22
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 99:
        {
          PIXEL00_11
          PIXEL01_21
          PIXEL10_12
          PIXEL11_22
          break;
        }
        case 57:
        {
          PIXEL00_12
          PIXEL01_22
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 71:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_21
          PIXEL11_22
          break;
        }
        case 156:
        {
          PIXEL00_21
          PIXEL01_11
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 226:
        {
          PIXEL00_22
          PIXEL01_21
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 60:
        {
          PIXEL00_21
          PIXEL01_11
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 195:
        {
          PIXEL00_11
          PIXEL01_21
          PIXEL10_21
          PIXEL11_11
          break;
        }
        case 102:
        {
          PIXEL00_22
          PIXEL01_12
          PIXEL10_12
          PIXEL11_22
          break;
        }
        case 153:
        {
          PIXEL00_12
          PIXEL01_22
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 58:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 83:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 92:
        {
          PIXEL00_21
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 202:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          PIXEL01_21
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          PIXEL11_11
          break;
        }
        case 78:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          PIXEL11_22
          break;
        }
        case 154:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 114:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 89:
        {
          PIXEL00_12
          PIXEL01_22
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 90:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_11
            PIXEL01_0
          }
          else
          {
            PIXEL00_60
            PIXEL01_90
          }
          PIXEL10_20
          PIXEL11_21
          break;
        }
        case 182:
        case 150:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
            PIXEL11_12
          }
          else
          {
            PIXEL01_90
            PIXEL11_61
          }
          PIXEL10_20
          break;
        }
        case 213:
        case 212:
        {
          PIXEL00_20
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL01_11
            PIXEL11_0
          }
          else
          {
            PIXEL01_60
            PIXEL11_90
          }
          PIXEL10_21
          break;
        }
        case 241:
        case 240:
        {
          PIXEL00_20
          PIXEL01_22
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL10_12
            PIXEL11_0
          }
          else
          {
            PIXEL10_61
            PIXEL11_90
          }
          break;
        }
        case 236:
        case 232:
        {
          PIXEL00_21
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
            PIXEL11_11
          }
          else
          {
            PIXEL10_90
            PIXEL11_60
          }
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_12
            PIXEL10_0
          }
          else
          {
            PIXEL00_61
            PIXEL10_90
          }
          PIXEL01_20
          PIXEL11_22
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
            PIXEL10_11
          }
          else
          {
            PIXEL00_90
            PIXEL10_60
          }
          PIXEL01_21
          PIXEL11_20
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
            PIXEL01_12
          }
          else
          {
            PIXEL00_90
            PIXEL01_61
          }
          PIXEL10_22
          PIXEL11_20
          break;
        }
        case 124:
        {
          PIXEL00_21
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_10
          break;
        }
        case 203:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          PIXEL10_10
          PIXEL11_11
          break;
        }
        case 62:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 211:
        {
          PIXEL00_11
          PIXEL01_10
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 118:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_12
          PIXEL11_10
          break;
        }
        case 217:
        {
          PIXEL00_12
          PIXEL01_22
          PIXEL10_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 110:
        {
          PIXEL00_10
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 155:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_10
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 188:
        {
          PIXEL00_21
          PIXEL01_11
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 185:
        {
          PIXEL00_12
          PIXEL01_22
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 61:
        {
          PIXEL00_12
          PIXEL01_11
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 157:
        {
          PIXEL00_12
          PIXEL01_11
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 103:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_12
          PIXEL11_22
          break;
        }
        case 227:
        {
          PIXEL00_11
          PIXEL01_21
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 230:
        {
          PIXEL00_22
          PIXEL01_12
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 199:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_21
          PIXEL11_11
          break;
        }
        case 220:
        {
          PIXEL00_21
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 158:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 234:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          PIXEL01_21
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_11
          break;
        }
        case 242:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 59:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 121:
        {
          PIXEL00_12
          PIXEL01_22
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 87:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 79:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          PIXEL11_22
          break;
        }
        case 122:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 94:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 218:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 91:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 229:
        {
          PIXEL00_20
          PIXEL01_20
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 167:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_20
          PIXEL11_20
          break;
        }
        case 173:
        {
          PIXEL00_12
          PIXEL01_20
          PIXEL10_11
          PIXEL11_20
          break;
        }
        case 181:
        {
          PIXEL00_20
          PIXEL01_11
          PIXEL10_20
          PIXEL11_12
          break;
        }
        case 186:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 115:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 93:
        {
          PIXEL00_12
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 206:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          PIXEL11_11
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_12
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_10
          }
          else
          {
            PIXEL10_70
          }
          PIXEL11_11
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_10
          }
          else
          {
            PIXEL00_70
          }
          PIXEL01_12
          PIXEL10_11
          PIXEL11_20
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_10
          }
          else
          {
            PIXEL01_70
          }
          PIXEL10_20
          PIXEL11_12
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_20
          PIXEL01_11
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_10
          }
          else
          {
            PIXEL11_70
          }
          break;
        }
        case 189:
        {
          PIXEL00_12
          PIXEL01_11
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 231:
        {
          PIXEL00_11
          PIXEL01_12
          PIXEL10_12
          PIXEL11_11
          break;
        }
        case 126:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_10
          break;
        }
        case 219:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_10
          PIXEL10_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 125:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_12
            PIXEL10_0
          }
          else
          {
            PIXEL00_61
            PIXEL10_90
          }
          PIXEL01_11
          PIXEL11_10
          break;
        }
        case 221:
        {
          PIXEL00_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL01_11
            PIXEL11_0
          }
          else
          {
            PIXEL01_60
            PIXEL11_90
          }
          PIXEL10_10
          break;
        }
        case 207:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
            PIXEL01_12
          }
          else
          {
            PIXEL00_90
            PIXEL01_61
          }
          PIXEL10_10
          PIXEL11_11
          break;
        }
        case 238:
        {
          PIXEL00_10
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
            PIXEL11_11
          }
          else
          {
            PIXEL10_90
            PIXEL11_60
          }
          break;
        }
        case 190:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
            PIXEL11_12
          }
          else
          {
            PIXEL01_90
            PIXEL11_61
          }
          PIXEL10_11
          break;
        }
        case 187:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
            PIXEL10_11
          }
          else
          {
            PIXEL00_90
            PIXEL10_60
          }
          PIXEL01_10
          PIXEL11_12
          break;
        }
        case 243:
        {
          PIXEL00_11
          PIXEL01_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL10_12
            PIXEL11_0
          }
          else
          {
            PIXEL10_61
            PIXEL11_90
          }
          break;
        }
        case 119:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_11
            PIXEL01_0
          }
          else
          {
            PIXEL00_60
            PIXEL01_90
          }
          PIXEL10_12
          PIXEL11_10
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_12
          PIXEL01_20
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          PIXEL11_11
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          PIXEL01_12
          PIXEL10_11
          PIXEL11_20
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_20
          PIXEL11_12
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_20
          PIXEL01_11
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 250:
        {
          PIXEL00_10
          PIXEL01_10
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 123:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_10
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_10
          break;
        }
        case 95:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_10
          PIXEL11_10
          break;
        }
        case 222:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 252:
        {
          PIXEL00_21
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 249:
        {
          PIXEL00_12
          PIXEL01_22
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 235:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_21
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          PIXEL11_11
          break;
        }
        case 111:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_22
          break;
        }
        case 63:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_11
          PIXEL11_21
          break;
        }
        case 159:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_22
          PIXEL11_12
          break;
        }
        case 215:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_21
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 246:
        {
          PIXEL00_22
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 254:
        {
          PIXEL00_10
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 253:
        {
          PIXEL00_12
          PIXEL01_11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 251:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_10
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 239:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          PIXEL01_12
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          PIXEL11_11
          break;
        }
        case 127:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_20
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_20
          }
          PIXEL11_10
          break;
        }
        case 191:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_11
          PIXEL11_12
          break;
        }
        case 223:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_10
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_20
          }
          break;
        }
        case 247:
        {
          PIXEL00_11
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          PIXEL10_12
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
        case 255:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_100
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_0
          }
          else
          {
            PIXEL01_100
          }
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_0
          }
          else
          {
            PIXEL10_100
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL11_0
          }
          else
          {
            PIXEL11_100
          }
          break;
        }
      }
      from++;
      to += 2;
    }
    from = from0 + sline_pixels;
    to = to0 + 2*dline_pixels;
  }
}

/*
 *	3X scaler.
 */
#define PIXEL00_1M  Interp1<PTYPES>(to, c[5], c[1], manip);
#define PIXEL00_1U  Interp1<PTYPES>(to, c[5], c[2], manip);
#define PIXEL00_1L  Interp1<PTYPES>(to, c[5], c[4], manip);
#define PIXEL00_2   Interp2<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_4   Interp4<PTYPES>(to, c[5], c[4], c[2], manip);
#define PIXEL00_5   Interp5<PTYPES>(to, c[4], c[2], manip);
#define PIXEL00_C   StoreRGB<PTYPES>(to, c[5], manip);

#define PIXEL01_1   Interp1<PTYPES>(to+1, c[5], c[2], manip);
#define PIXEL01_3   Interp3<PTYPES>(to+1, c[5], c[2], manip);
#define PIXEL01_6   Interp1<PTYPES>(to+1, c[2], c[5], manip);
#define PIXEL01_C   StoreRGB<PTYPES>(to+1, c[5], manip);

#define PIXEL02_1M  Interp1<PTYPES>(to+2, c[5], c[3], manip);
#define PIXEL02_1U  Interp1<PTYPES>(to+2, c[5], c[2], manip);
#define PIXEL02_1R  Interp1<PTYPES>(to+2, c[5], c[6], manip);
#define PIXEL02_2   Interp2<PTYPES>(to+2, c[5], c[2], c[6], manip);
#define PIXEL02_4   Interp4<PTYPES>(to+2, c[5], c[2], c[6], manip);
#define PIXEL02_5   Interp5<PTYPES>(to+2, c[2], c[6], manip);
#define PIXEL02_C   StoreRGB<PTYPES>(to+2, c[5], manip);

#define PIXEL10_1   Interp1<PTYPES>(to+dline_pixels, c[5], c[4], manip);
#define PIXEL10_3   Interp3<PTYPES>(to+dline_pixels, c[5], c[4], manip);
#define PIXEL10_6   Interp1<PTYPES>(to+dline_pixels, c[4], c[5], manip);
#define PIXEL10_C   StoreRGB<PTYPES>(to+dline_pixels, c[5], manip);

#define PIXEL11     StoreRGB<PTYPES>(to+dline_pixels+1, c[5], manip);

#define PIXEL12_1   Interp1<PTYPES>(to+dline_pixels+2, c[5], c[6], manip);
#define PIXEL12_3   Interp3<PTYPES>(to+dline_pixels+2, c[5], c[6], manip);
#define PIXEL12_6   Interp1<PTYPES>(to+dline_pixels+2, c[6], c[5], manip);
#define PIXEL12_C   StoreRGB<PTYPES>(to+dline_pixels+2, c[5], manip);

#define PIXEL20_1M  Interp1<PTYPES>(to+2*dline_pixels, c[5], c[7], manip);
#define PIXEL20_1D  Interp1<PTYPES>(to+2*dline_pixels, c[5], c[8], manip);
#define PIXEL20_1L  Interp1<PTYPES>(to+2*dline_pixels, c[5], c[4], manip);
#define PIXEL20_2   Interp2<PTYPES>(to+2*dline_pixels, c[5], c[8], c[4],manip);
#define PIXEL20_4   Interp4<PTYPES>(to+2*dline_pixels, c[5], c[8], c[4],manip);
#define PIXEL20_5   Interp5<PTYPES>(to+2*dline_pixels, c[8], c[4], manip);
#define PIXEL20_C   StoreRGB<PTYPES>(to+2*dline_pixels, c[5], manip);

#define PIXEL21_1   Interp1<PTYPES>(to+2*dline_pixels+1, c[5], c[8], manip);
#define PIXEL21_3   Interp3<PTYPES>(to+2*dline_pixels+1, c[5], c[8], manip);
#define PIXEL21_6   Interp1<PTYPES>(to+2*dline_pixels+1, c[8], c[5], manip);
#define PIXEL21_C   StoreRGB<PTYPES>(to+2*dline_pixels+1, c[5], manip);

#define PIXEL22_1M  Interp1<PTYPES>(to+2*dline_pixels+2, c[5], c[9], manip);
#define PIXEL22_1D  Interp1<PTYPES>(to+2*dline_pixels+2, c[5], c[8], manip);
#define PIXEL22_1R  Interp1<PTYPES>(to+2*dline_pixels+2, c[5], c[6], manip);
#define PIXEL22_2   Interp2<PTYPES>(to+2*dline_pixels+2, c[5], c[6], c[8],manip);
#define PIXEL22_4   Interp4<PTYPES>(to+2*dline_pixels+2, c[5], c[6], c[8],manip);
#define PIXEL22_5   Interp5<PTYPES>(to+2*dline_pixels+2, c[6], c[8], manip);
#define PIXEL22_C   StoreRGB<PTYPES>(to+2*dline_pixels+2, c[5], manip);

template <class Dest_pixel, class Manip_pixels>
void Scale_Hq3x
	(
	unsigned char *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	)
{
  int  i, j, k;
  int  prevline, nextline;
  int  w[10];
  int  c[10];
  int  yuv[10];
#if 0
srcx = srcy=0; srcw=sline_pixels; srch=sheight;//++++TESTING
#endif
  int stopy = srcy + srch, stopx = srcx + srcw;
  unsigned char *from = source + srcy*sline_pixels + srcx;
  Dest_pixel *to = dest + 3*srcy*dline_pixels + 3*srcx;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j=srcy; j<stopy; j++)
  {
    unsigned char *from0 = from;		// Save start of line.
    Dest_pixel *to0 = to;

    if (j>0)      prevline = -sline_pixels; else prevline = 0;
    if (j<sheight-1) nextline =  sline_pixels; else nextline = 0;

    for (i=srcx; i<stopx; i++)
    {
      int pattern = hqx_init(w, c, yuv, from, i, sline_pixels, 
					prevline, nextline, manip);
      switch (pattern)
      {
        case 0:
        case 1:
        case 4:
        case 32:
        case 128:
        case 5:
        case 132:
        case 160:
        case 33:
        case 129:
        case 36:
        case 133:
        case 164:
        case 161:
        case 37:
        case 165:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 66:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 24:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 146:
        case 178:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_1M
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          break;
        }
        case 84:
        case 85:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 112:
        case 113:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_1M
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 200:
        case 204:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_1M
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          PIXEL22_1M
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 67:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 70:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 28:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 152:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 194:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 98:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 56:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 25:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 27:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 86:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 216:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 106:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 30:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 210:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 120:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 75:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 29:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 198:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 184:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 99:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 57:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 71:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 156:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 226:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 60:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 195:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 102:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 153:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 58:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 83:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 92:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 202:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 78:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 154:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 114:
        {
          PIXEL00_1M
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 89:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 90:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 182:
        case 150:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_2
          PIXEL21_1
          break;
        }
        case 213:
        case 212:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 241:
        case 240:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 236:
        case 232:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_2
          PIXEL11
          PIXEL12_1
          PIXEL22_1M
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 124:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 203:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 62:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 211:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 118:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 217:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 110:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 155:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 188:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 185:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 61:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 157:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 103:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 227:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 230:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 199:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 220:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 158:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 234:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1R
          break;
        }
        case 242:
        {
          PIXEL00_1M
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 59:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 121:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 87:
        {
          PIXEL00_1L
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 79:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 122:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 94:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 218:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 91:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 229:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_2
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 167:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_2
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 173:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 181:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 186:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 115:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 93:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 206:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_1M
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_1M
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_1M
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_1M
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 189:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 231:
        {
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_1
          PIXEL11
          PIXEL12_1
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 126:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 219:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
            PIXEL10_3
          }
          PIXEL02_1M
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 125:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL00_1U
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL00_2
            PIXEL10_6
            PIXEL20_5
            PIXEL21_1
          }
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          PIXEL22_1M
          break;
        }
        case 221:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL02_1U
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL02_2
            PIXEL12_6
            PIXEL21_1
            PIXEL22_5
          }
          PIXEL00_1U
          PIXEL01_1
          PIXEL10_C
          PIXEL11
          PIXEL20_1M
          break;
        }
        case 207:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL02_1R
            PIXEL10_C
          }
          else
          {
            PIXEL00_5
            PIXEL01_6
            PIXEL02_2
            PIXEL10_1
          }
          PIXEL11
          PIXEL12_1
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 238:
        {
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
            PIXEL22_1R
          }
          else
          {
            PIXEL10_1
            PIXEL20_5
            PIXEL21_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1R
          PIXEL11
          PIXEL12_1
          break;
        }
        case 190:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
            PIXEL22_1D
          }
          else
          {
            PIXEL01_1
            PIXEL02_5
            PIXEL12_6
            PIXEL22_2
          }
          PIXEL00_1M
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          break;
        }
        case 187:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
            PIXEL20_1D
          }
          else
          {
            PIXEL00_5
            PIXEL01_1
            PIXEL10_6
            PIXEL20_2
          }
          PIXEL02_1M
          PIXEL11
          PIXEL12_C
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 243:
        {
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL20_1L
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_1
            PIXEL20_2
            PIXEL21_6
            PIXEL22_5
          }
          PIXEL00_1L
          PIXEL01_C
          PIXEL02_1M
          PIXEL10_1
          PIXEL11
          break;
        }
        case 119:
        {
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL00_1L
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_6
            PIXEL02_5
            PIXEL12_1
          }
          PIXEL10_1
          PIXEL11
          PIXEL20_1L
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_2
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_2
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_2
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_2
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 250:
        {
          PIXEL00_1M
          PIXEL01_C
          PIXEL02_1M
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 123:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 95:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          PIXEL21_C
          PIXEL22_1M
          break;
        }
        case 222:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 252:
        {
          PIXEL00_1M
          PIXEL01_1
          PIXEL02_1U
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 249:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 235:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 111:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 63:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL10_C
          PIXEL11
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1M
          break;
        }
        case 159:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 215:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 246:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 254:
        {
          PIXEL00_1M
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_4
          }
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_4
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL21_3
            PIXEL22_2
          }
          break;
        }
        case 253:
        {
          PIXEL00_1U
          PIXEL01_1
          PIXEL02_1U
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 251:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
          }
          else
          {
            PIXEL00_4
            PIXEL01_3
          }
          PIXEL02_1M
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL10_C
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL10_3
            PIXEL20_2
            PIXEL21_3
          }
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL12_C
            PIXEL22_C
          }
          else
          {
            PIXEL12_3
            PIXEL22_4
          }
          break;
        }
        case 239:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          PIXEL02_1R
          PIXEL10_C
          PIXEL11
          PIXEL12_1
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          PIXEL22_1R
          break;
        }
        case 127:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL01_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_2
            PIXEL01_3
            PIXEL10_3
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL02_4
            PIXEL12_3
          }
          PIXEL11
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
            PIXEL21_C
          }
          else
          {
            PIXEL20_4
            PIXEL21_3
          }
          PIXEL22_1M
          break;
        }
        case 191:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          PIXEL20_1D
          PIXEL21_1
          PIXEL22_1D
          break;
        }
        case 223:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
            PIXEL10_C
          }
          else
          {
            PIXEL00_4
            PIXEL10_3
          }
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL01_C
            PIXEL02_C
            PIXEL12_C
          }
          else
          {
            PIXEL01_3
            PIXEL02_2
            PIXEL12_3
          }
          PIXEL11
          PIXEL20_1M
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL21_C
            PIXEL22_C
          }
          else
          {
            PIXEL21_3
            PIXEL22_4
          }
          break;
        }
        case 247:
        {
          PIXEL00_1L
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_1
          PIXEL11
          PIXEL12_C
          PIXEL20_1L
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
        case 255:
        {
          if (Diff(yuv[4], yuv[2]))
          {
            PIXEL00_C
          }
          else
          {
            PIXEL00_2
          }
          PIXEL01_C
          if (Diff(yuv[2], yuv[6]))
          {
            PIXEL02_C
          }
          else
          {
            PIXEL02_2
          }
          PIXEL10_C
          PIXEL11
          PIXEL12_C
          if (Diff(yuv[8], yuv[4]))
          {
            PIXEL20_C
          }
          else
          {
            PIXEL20_2
          }
          PIXEL21_C
          if (Diff(yuv[6], yuv[8]))
          {
            PIXEL22_C
          }
          else
          {
            PIXEL22_2
          }
          break;
        }
      }
      from++;
      to+=3;
    }
    from = from0 + sline_pixels;
    to = to0 + 3*dline_pixels;
  }
}

