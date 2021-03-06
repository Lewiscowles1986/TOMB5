#include "LIBGPU.H"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#if __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <LIBETC.H>

#include "EMULATOR.H"
#include "EMULATOR_GLOBALS.H"

unsigned short vram[1024 * 512];
DISPENV word_33BC;
DRAWENV activeDrawEnv;
DRAWENV byte_9CCA4;
int dword_3410 = 0;
char byte_3352 = 0;
unsigned long* terminator;
void(*drawsync_callback)(void) = NULL;

void* off_3348[]=
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

int ClearImage(RECT16* rect, u_char r, u_char g, u_char b)
{
	Emulator_CheckTextureIntersection(rect);

	for (int y = rect->y; y < 512; y++)
	{
		for (int x = rect->x; x < 1024; x++)
		{
			unsigned short* pixel = vram + (y * 1024 + x);

			if (x >= rect->x && x < rect->x + rect->w && 
				y >= rect->y && y < rect->y + rect->h)
			{
				pixel[0] = 1 << 15 | ((r >> 3) << 10) | ((g >> 3) << 5) | ((b >> 3));
			}
		}
	}

	return 0;
}

int DrawSync(int mode)
{
	if (drawsync_callback != NULL)
	{
		drawsync_callback();
	}
	return 0;
}

int LoadImagePSX(RECT16* rect, u_long* p)
{
	Emulator_CheckTextureIntersection(rect);

	unsigned short* dst = (unsigned short*)p;

	for (int y = rect->y; y < 512; y++)
	{
		for (int x = rect->x; x < 1024; x++)
		{
			unsigned short* src = vram + (y * 1024 + x);

			if (x >= rect->x && x < rect->x + rect->w &&
				y >= rect->y && y < rect->y + rect->h)
			{
				src[0] = *dst++;
			}
		}
	}

#if _DEBUG
	FILE* f = fopen("VRAM.BIN", "wb");
	fwrite(&vram[0], 1, 1024 * 512 * 2, f);
	fclose(f);
#endif

	return 0;
}

int MoveImage(RECT16* rect, int x, int y)
{
#if 0//TODO
	for (int sy = rect->y; sy < 512; sy++)
	{
		for (int sx = rect->x; sx < 1024; sx++)
		{
			unsigned short* src = vram + (sy * 1024 + sx);
			unsigned short* dst = vram + (y * 1024 + x);

			if (sx >= rect->x && sx < rect->x + rect->w &&
				sy >= rect->y && sy < rect->y + rect->h)
			{
				*dst++ = *src++;
			}
		}
	}
#endif
	return 0;
}

int ResetGraph(int mode)
{
	UNIMPLEMENTED();
	return 0;
}

int SetGraphDebug(int level)
{
	UNIMPLEMENTED();
	return 0;
}

int StoreImage(RECT16 * RECT16, u_long * p)
{
	UNIMPLEMENTED();
	return 0;
}

u_long* ClearOTag(u_long* ot, int n)
{
	//Nothing to do here.
	if (n == 0)
		return NULL;

	//Last is special terminator
	ot[n-1] = (unsigned long)terminator;

	if (n > 1)
	{
		for (int i = n-2; i > -1; i--)
		{
			ot[i] = (unsigned long)&ot[i+1];
		}
	}
	return NULL;
}

u_long* ClearOTagR(u_long* ot, int n)
{
	//Nothing to do here.
	if (n == 0)
		return NULL;

	//First is special terminator
	ot[0] = (unsigned long)terminator;

	if (n > 1)
	{
		for (int i = 1; i < n; i++)
		{
			ot[i] = (unsigned long)&ot[i - 1];
		}
	}
	return NULL;
}

void SetDispMask(int mask)
{
	UNIMPLEMENTED();
}

DISPENV* GetDispEnv(DISPENV* env)//(F)
{
	memcpy(env, &word_33BC, sizeof(DISPENV));
	return env;
}

DISPENV* PutDispEnv(DISPENV* env)//To Finish
{
	memcpy((char*)&word_33BC, env, sizeof(DISPENV));
	return 0;
}

DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h)//(F)
{
	env->disp.x = x;
	env->disp.y = y;
	env->disp.w = w;
	env->screen.x = 0;
	env->screen.y = 0;
	env->screen.w = 0;
	env->screen.h = 0;
	env->isrgb24 = 0;
	env->isinter = 0;
	env->pad1 = 0;
	env->pad0 = 0;
	env->disp.h = h;
	return 0;
}

DRAWENV* PutDrawEnv(DRAWENV* env)//Guessed
{
	memcpy((char*)&activeDrawEnv, env, sizeof(DRAWENV));
	return 0;
}

DRAWENV* SetDefDrawEnv(DRAWENV* env, int x, int y, int w, int h)//(F)
{
	env->clip.x = x;
	env->clip.y = y;
	env->clip.w = w;
	env->tw.x = 0;
	env->tw.y = 0;
	env->tw.w = 0;
	env->tw.h = 0;
	env->r0 = 0;
	env->g0 = 0;
	env->b0 = 0;
	env->dtd = 1;
	env->clip.h = h;

	if (GetVideoMode() == 0)
	{
		env->dfe = h < 0x121 ? 1 : 0;
	}
	else
	{
		env->dfe = h < 0x101 ? 1 : 0;
	}

	env->ofs[0] = x;
	env->ofs[1] = y;

	env->tpage = 10;
	env->isbg = 0;

	return env;
}

void SetDrawEnv(DR_ENV* dr_env, DRAWENV* env)
{

}

u_long DrawSyncCallback(void(*func)(void))
{
	drawsync_callback = func;
	return 0;
}

void DrawOTagEnv(u_long* p, DRAWENV* env)//
{
#if 0
	//if (byte_3352[0] > 1)
	{
		GPU_printf("DrawOTagEnv(%08x,&08x)...\n", p, env);
	}//loc_EF8

	//s0 = &env->dr_env

	//sub_17C0(&env->dr_env, env);

	env->dr_env.tag = env->dr_env.tag & 0xFF000000 | (ptrdiff_t)p & 0xFFFFFF;
	//a0 = off_3348[6];
	//v0 = off_3348[2];
	///jalr off_3348[2](off_3348[6]);
	memcpy(&byte_9CCA4, &env, sizeof(DRAWENV));

#else
	PutDrawEnv(env);

	if (env->isbg)
	{
		ClearImage(&env->clip, env->b0, env->g0, env->r0);
	}

	GLuint fbo = 0;
	
	if (p != NULL)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, 1024, 0, 512, -1, 1);
		glViewport(0, 0, 1024, 512);

		glEnable(GL_TEXTURE_2D);
		Emulator_GenerateFrameBuffer(fbo);
		Emulator_GenerateFrameBufferTexture();

		P_TAG* pTag = (P_TAG*)p;

		do
		{
			int textured = (pTag->code & 4) != 0;
			int blend_mode = 0;
			int semi_transparent = (pTag->code & 2) != 0;

			if (textured)
			{
				if ((pTag->code & 1) != 0)
				{
					blend_mode = 2;
				}
				else
				{
					blend_mode = 1;
				}
			}
			else
			{
				blend_mode = 0;
			}

			glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);

			if (semi_transparent)
			{
				Emulator_SetBlendMode(blend_mode);
			}

			switch (pTag->code & ~3)
			{
			case 0x00: // null poly
				break;
			case 0x20: // POLY_F3
			{
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);
				POLY_F3* poly = (POLY_F3*)pTag;

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x2, NULL);
				char* texCoordPointer = Emulator_GenerateTexcoordArrayQuad(NULL, NULL, NULL, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, NULL, NULL, NULL, false);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glColorPointer(3, GL_FLOAT, sizeof(Vertex), colourPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texCoordPointer);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x24: // POLY_FT3
			{
				
				POLY_FT3* poly = (POLY_FT3*)pTag;

				Emulator_GenerateAndBindTpage(poly->tpage, poly->clut, semi_transparent);
				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x2, NULL);
				char* texCoordPointer = Emulator_GenerateTexcoordArrayQuad(NULL, NULL, NULL, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, NULL, NULL, NULL, true);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glColorPointer(3, GL_FLOAT, sizeof(Vertex), colourPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texCoordPointer);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x28: // POLY_F4
			{
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);
				POLY_F4* poly = (POLY_F4*)pTag;

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x3, &poly->x2);
				char* texCoordPointer = Emulator_GenerateTexcoordArrayQuad(NULL, NULL, NULL, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, NULL, NULL, NULL, false);
				
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glColorPointer(3, GL_FLOAT, sizeof(Vertex), colourPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texCoordPointer);
				glDrawArrays(GL_QUADS, 0, 4);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x2C: // POLY_FT4
			{

				POLY_FT4* poly = (POLY_FT4*)pTag;

				Emulator_GenerateAndBindTpage(poly->tpage, poly->clut, semi_transparent);

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x3, &poly->x2);
				char* texcoordPointer = Emulator_GenerateTexcoordArrayQuad(&poly->u0, &poly->u1, &poly->u3, &poly->u2);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);

				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texcoordPointer);
				glDrawArrays(GL_QUADS, 0, 4);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x30: // POLY_G3
			{
				POLY_G3* poly = (POLY_G3*)pTag;

				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x2, NULL);
				char* texcoordPointer = Emulator_GenerateTexcoordArrayQuad(NULL, NULL, NULL, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, &poly->r1, &poly->r2, NULL, false);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texcoordPointer);
				glColorPointer(4, GL_FLOAT, sizeof(Vertex), colourPointer);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x34: // POLY_GT3
			{
				POLY_GT3* poly = (POLY_GT3*)pTag;

				Emulator_GenerateAndBindTpage(poly->tpage, poly->clut, semi_transparent);

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x2, NULL);
				char* texcoordPointer = Emulator_GenerateTexcoordArrayQuad(&poly->u0, &poly->u1, &poly->u2, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, &poly->r1, &poly->r2, NULL, true);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texcoordPointer);
				glColorPointer(4, GL_FLOAT, sizeof(Vertex), colourPointer);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x38: // POLY_G4
			{
				POLY_G4* poly = (POLY_G4*)pTag;

				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x3, &poly->x2);
				char* texcoordPointer = Emulator_GenerateTexcoordArrayQuad(NULL, NULL, NULL, NULL);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, &poly->r1, &poly->r3, &poly->r2, false);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texcoordPointer);
				glColorPointer(4, GL_FLOAT, sizeof(Vertex), colourPointer);
				glDrawArrays(GL_QUADS, 0, 4);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			}
			case 0x3C: // POLY_GT4
			{				
				POLY_GT4* poly = (POLY_GT4*)pTag;
				
				Emulator_GenerateAndBindTpage(poly->tpage, poly->clut, semi_transparent);

				char* vertexPointer = Emulator_GenerateVertexArrayQuad(&poly->x0, &poly->x1, &poly->x3, &poly->x2);
				char* texcoordPointer = Emulator_GenerateTexcoordArrayQuad(&poly->u0, &poly->u1, &poly->u3, &poly->u2);
				char* colourPointer = Emulator_GenerateColourArrayQuad(&poly->r0, &poly->r1, &poly->r2, &poly->r3, true);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				
				glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertexPointer);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), texcoordPointer);
				glColorPointer(4, GL_FLOAT, sizeof(Vertex), colourPointer);
				glDrawArrays(GL_QUADS, 0, 4);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);
				break;
			}
			case 0x40: // LINE_F2
			{
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				LINE_F2* poly = (LINE_F2*)pTag;
				glLineWidth(1);
				glBegin(GL_LINES);

				glColor3ubv(&poly->r0);
				glVertex2f(poly->x0, poly->y0);

				glVertex2f(poly->x1, poly->y1);

				glEnd();
				break;
			}
			case 0x50: // LINE_G2
			{
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				LINE_G2* poly = (LINE_G2*)pTag;
				glLineWidth(1);
				glBegin(GL_LINES);
				glColor3ubv(&poly->r0);
				glVertex2f(poly->x0, poly->y0);
				glColor3ubv(&poly->r1);
				glVertex2f(poly->x1, poly->y1);
				glEnd();
				
				if (getlen(pTag) == 9)
				{
					poly++;
					//poly = (LINE_G2*)((uintptr_t)poly - 4);
					glBegin(GL_LINES);
					glColor3ubv(&poly->r0);
					glVertex2f(poly->x0, poly->y0);
					glColor3ubv(&poly->r1);
					glVertex2f(poly->x1, poly->y1);
					glEnd();
				}
				break;
			}
			case 0x60: // TILE
			{
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				TILE* poly = (TILE*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(poly->x0 + poly->w, poly->y0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(poly->x0 + poly->w, poly->y0 + poly->h);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(poly->x0, poly->y0 + poly->h);

				glEnd();

				break;
			}
			case 0x64: // SPRT
			{
				SPRT* poly = (SPRT*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(poly->u0 / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f((poly->u0 + poly->w) / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0 + poly->w, poly->y0);

				glTexCoord2f(poly->u0 / 256.0f, (poly->v0 + poly->h) / 256.0f);
				glVertex2f(poly->x0 + poly->w, poly->y0 + poly->h);

				glTexCoord2f((poly->u0 + poly->w) / 256.0f, (poly->v0 + poly->h) / 256.0f);
				glVertex2f(poly->x0, poly->y0 + poly->h);

				glEnd();

				break;
			}
			case 0x68: // TILE_1
			{
				const int width = 1;

				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				TILE_1* poly = (TILE_1*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(poly->x0 + width, poly->y0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(poly->x0 + width, poly->y0 + width);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(poly->x0, poly->y0 + width);

				glEnd();

				break;
			}
			case 0x70: // TILE_8
			{
				const int width = 8;
				
				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				TILE_1* poly = (TILE_1*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(poly->x0 + width, poly->y0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(poly->x0 + width, poly->y0 + width);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(poly->x0, poly->y0 + width);

				glEnd();

				break;
			}
			case 0x74: // SPRT_8
			{
				SPRT_8* poly = (SPRT_8*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(poly->u0 / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f((poly->u0 + 8) / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0 + 8, poly->y0);

				glTexCoord2f(poly->u0 / 256.0f, (poly->v0 + 8) / 256.0f);
				glVertex2f(poly->x0 + 8, poly->y0 + 8);

				glTexCoord2f((poly->u0 + 8) / 256.0f, (poly->v0 + 8) / 256.0f);
				glVertex2f(poly->x0, poly->y0 + 8);

				glEnd();

				break;
			}
			case 0x78: // TILE_16
			{
				const int width = 16;

				glBindTexture(GL_TEXTURE_2D, nullWhiteTexture);

				TILE_1* poly = (TILE_1*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(poly->x0 + width, poly->y0);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(poly->x0 + width, poly->y0 + width);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(poly->x0, poly->y0 + width);

				glEnd();

				break;
			}
			case 0x7C: // SPRT_16
			{
				SPRT_16* poly = (SPRT_16*)pTag;

				glBegin(GL_QUADS);

				glColor3ubv(&poly->r0);

				glTexCoord2f(poly->u0 / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0, poly->y0);

				glTexCoord2f((poly->u0 + 16) / 256.0f, poly->v0 / 256.0f);
				glVertex2f(poly->x0 + 16, poly->y0);

				glTexCoord2f(poly->u0 / 256.0f, (poly->v0 + 16) / 256.0f);
				glVertex2f(poly->x0 + 16, poly->y0 + 16);

				glTexCoord2f((poly->u0 + 16) / 256.0f, (poly->v0 + 16) / 256.0f);
				glVertex2f(poly->x0, poly->y0 + 16);

				glEnd();

				break;
			}
			case 0xE1: // TPAGE
			{
				unsigned short tpage = ((unsigned short*)pTag)[2];
				Emulator_GenerateAndBindTpage(tpage, 0, semi_transparent);
				break;
			}
			default:
				eprinterr("Unhandled primitive type: %02X\n", pTag->code);
				//Unhandled poly
				break;
			}

			pTag = (P_TAG*)(unsigned int)pTag->addr;

			//Reset for vertex colours
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		}while ((unsigned long)pTag != (unsigned long)terminator && (unsigned long)pTag != 0xFFFFFF);

		Emulator_DestroyLastVRAMTexture();
		Emulator_DeleteFrameBufferTexture();

		glViewport(0, 0, windowWidth, windowHeight);
		glPopMatrix();

		Emulator_DestroyFrameBuffer(fbo);
	}

	Emulator_CheckTextureIntersection(&env->clip);

#endif
}