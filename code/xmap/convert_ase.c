/*
===========================================================================
Copyright (C) 1999-2006 Id Software, Inc.
Copyright (C) 2001-2006 Randy Reddig (Q3Map2)
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "qbsp.h"



/*
ConvertSurface()
converts a bsp drawsurface to an ase chunk
*/
static void ConvertSurface(FILE * f, dmodel_t * model, int modelNum, dsurface_t * ds, int surfaceNum, vec3_t origin)
{
	int             i, v, face, a, b, c;
	drawVert_t     *dv;
	vec3_t          normal;
	char            name[1024];

	//Sys_Printf("ConvertSurface( %i , %i )\n", modelNum, surfaceNum);

	// ignore patches for now
	if(ds->surfaceType != MST_PLANAR && ds->surfaceType != MST_TRIANGLE_SOUP)
		return;

	// print object header for each dsurf
	sprintf(name, "mat%dmodel%dsurf%d", ds->shaderNum, modelNum, surfaceNum);
	fprintf(f, "*GEOMOBJECT\t{\r\n");
	fprintf(f, "\t*NODE_NAME\t\"%s\"\r\n", name);
	fprintf(f, "\t*NODE_TM\t{\r\n");
	fprintf(f, "\t\t*NODE_NAME\t\"%s\"\r\n", name);
	fprintf(f, "\t\t*INHERIT_POS\t0\t0\t0\r\n");
	fprintf(f, "\t\t*INHERIT_ROT\t0\t0\t0\r\n");
	fprintf(f, "\t\t*INHERIT_SCL\t0\t0\t0\r\n");
	fprintf(f, "\t\t*TM_ROW0\t1.0\t0\t0\r\n");
	fprintf(f, "\t\t*TM_ROW1\t0\t1.0\t0\r\n");
	fprintf(f, "\t\t*TM_ROW2\t0\t0\t1.0\r\n");
	fprintf(f, "\t\t*TM_ROW3\t0\t0\t0\r\n");
	fprintf(f, "\t\t*TM_POS\t%f\t%f\t%f\r\n", origin[0], origin[1], origin[2]);
	fprintf(f, "\t}\r\n");

	// print mesh header
	fprintf(f, "\t*MESH\t{\r\n");
	fprintf(f, "\t\t*TIMEVALUE\t0\r\n");
	fprintf(f, "\t\t*MESH_NUMVERTEX\t%d\r\n", ds->numVerts);
	fprintf(f, "\t\t*MESH_NUMFACES\t%d\r\n", ds->numIndexes / 3);
	switch (ds->surfaceType)
	{
		case MST_PLANAR:
			fprintf(f, "\t\t*COMMENT\t\"SURFACETYPE\tMST_PLANAR\"\r\n");
			break;
		case MST_TRIANGLE_SOUP:
			fprintf(f, "\t\t*COMMENT\t\"SURFACETYPE\tMST_TRIANGLE_SOUP\"\r\n");
			break;
	}

	// export vertex xyz
	fprintf(f, "\t\t*MESH_VERTEX_LIST\t{\r\n");
	for(i = 0; i < ds->numVerts; i++)
	{
		v = i + ds->firstVert;
		dv = &drawVerts[v];
		fprintf(f, "\t\t\t*MESH_VERTEX\t%d\t%f\t%f\t%f\r\n", i, dv->xyz[0], dv->xyz[1], dv->xyz[2]);
	}
	fprintf(f, "\t\t}\r\n");

	// export vertex normals
	fprintf(f, "\t\t*MESH_NORMALS\t{\r\n");
	for(i = 0; i < ds->numIndexes; i += 3)
	{
		face = (i / 3);
		a = drawIndexes[i + ds->firstIndex];
		b = drawIndexes[i + ds->firstIndex + 1];
		c = drawIndexes[i + ds->firstIndex + 2];
		VectorCopy(drawVerts[a].normal, normal);
		VectorAdd(normal, drawVerts[b].normal, normal);
		VectorAdd(normal, drawVerts[c].normal, normal);
		if(VectorNormalize2(normal, normal))
			fprintf(f, "\t\t\t*MESH_FACENORMAL\t%d\t%f\t%f\t%f\r\n", face, normal[0], normal[1], normal[2]);
	}
	for(i = 0; i < ds->numVerts; i++)
	{
		v = i + ds->firstVert;
		dv = &drawVerts[v];
		fprintf(f, "\t\t\t*MESH_VERTEXNORMAL\t%d\t%f\t%f\t%f\r\n", i, dv->normal[0], dv->normal[1], dv->normal[2]);
	}
	fprintf(f, "\t\t}\r\n");

	// export faces
	fprintf(f, "\t\t*MESH_FACE_LIST\t{\r\n");
	for(i = 0; i < ds->numIndexes; i += 3)
	{
		face = (i / 3);
		a = drawIndexes[i + ds->firstIndex];
		c = drawIndexes[i + ds->firstIndex + 1];
		b = drawIndexes[i + ds->firstIndex + 2];
		fprintf(f, "\t\t\t*MESH_FACE\t%d\tA:\t%d\tB:\t%d\tC:\t%d\tAB:\t1\tBC:\t1\tCA:\t1\t*MESH_SMOOTHING\t0\t*MESH_MTLID\t0\r\n",
				face, a, b, c);
	}
	fprintf(f, "\t\t}\r\n");

	// export vertex st
	fprintf(f, "\t\t*MESH_NUMTVERTEX\t%d\r\n", ds->numVerts);
	fprintf(f, "\t\t*MESH_TVERTLIST\t{\r\n");
	for(i = 0; i < ds->numVerts; i++)
	{
		v = i + ds->firstVert;
		dv = &drawVerts[v];
		fprintf(f, "\t\t\t*MESH_TVERT\t%d\t%f\t%f\t%f\r\n", i, dv->st[0], (1.0 - dv->st[1]), 1.0f);
	}
	fprintf(f, "\t\t}\r\n");

	// export texture faces
	fprintf(f, "\t\t*MESH_NUMTVFACES\t%d\r\n", ds->numIndexes / 3);
	fprintf(f, "\t\t*MESH_TFACELIST\t{\r\n");
	for(i = 0; i < ds->numIndexes; i += 3)
	{
		face = (i / 3);
		a = drawIndexes[i + ds->firstIndex];
		c = drawIndexes[i + ds->firstIndex + 1];
		b = drawIndexes[i + ds->firstIndex + 2];
		fprintf(f, "\t\t\t*MESH_TFACE\t%d\t%d\t%d\t%d\r\n", face, a, b, c);
	}
	fprintf(f, "\t\t}\r\n");

	// print mesh footer
	fprintf(f, "\t}\r\n");

	// print object footer
	fprintf(f, "\t*PROP_MOTIONBLUR\t0\r\n");
	fprintf(f, "\t*PROP_CASTSHADOW\t1\r\n");
	fprintf(f, "\t*PROP_RECVSHADOW\t1\r\n");
	fprintf(f, "\t*MATERIAL_REF\t%d\r\n", ds->shaderNum);
	fprintf(f, "}\r\n");
}


/*
ConvertShader()
exports a bsp shader to an ase chunk
*/

/*
	*MATERIAL 0 {
		*MATERIAL_NAME "models/test/rock16l"
		*MATERIAL_CLASS "Standard"
		*MATERIAL_AMBIENT 0.5882	0.5882	0.5882
		*MATERIAL_DIFFUSE 0.5882	0.5882	0.5882
		*MATERIAL_SPECULAR 0.5882	0.5882	0.5882
		*MATERIAL_SHINE 0.0000
		*MATERIAL_SHINESTRENGTH 0.0000
		*MATERIAL_TRANSPARENCY 0.0000
		*MATERIAL_WIRESIZE 1.0000
		*MATERIAL_SHADING Phong
		*MATERIAL_XP_FALLOFF 0.0000
		*MATERIAL_SELFILLUM 0.0000
		*MATERIAL_FALLOFF In
		*MATERIAL_XP_TYPE Filter
		*MAP_DIFFUSE {
			*MAP_NAME "Map #2"
			*MAP_CLASS "Bitmap"
			*MAP_SUBNO 1
			*MAP_AMOUNT 1.0000
			*BITMAP "models/test/rock16l"
			*MAP_TYPE Screen
			*UVW_U_OFFSET 0.0000
			*UVW_V_OFFSET 0.0000
			*UVW_U_TILING 1.0000
			*UVW_V_TILING 1.0000
			*UVW_ANGLE 0.0000
			*UVW_BLUR 1.0000
			*UVW_BLUR_OFFSET 0.0000
			*UVW_NOUSE_AMT 1.0000
			*UVW_NOISE_SIZE 1.0000
			*UVW_NOISE_LEVEL 1
			*UVW_NOISE_PHASE 0.0000
			*BITMAP_FILTER Pyramidal
		}
	}
*/

static void ConvertShader(FILE * f, dshader_t * shader, int shaderNum)
{
	shaderInfo_t   *si;
	char           *c, filename[1024];


	// get shader
	si = ShaderInfoForShader(shader->shader);
	if(si == NULL)
	{
		Sys_Printf("WARNING: NULL shader in BSP\n");
		return;
	}

	// set bitmap filename
	if(si->shader[0] != '*')
		strcpy(filename, si->shader);
	else
		sprintf(filename, "%s.tga", si->shader);

#if 0
	for(c = filename; *c != '\0'; c++)
		if(*c == '/')
			*c = '\\';
#endif

	// print shader info
	fprintf(f, "\t*MATERIAL\t%d\t{\r\n", shaderNum);
	fprintf(f, "\t\t*MATERIAL_NAME\t\"%s\"\r\n", shader->shader);
	fprintf(f, "\t\t*MATERIAL_CLASS\t\"Standard\"\r\n");
	fprintf(f, "\t\t*MATERIAL_DIFFUSE\t%f\t%f\t%f\r\n", si->color[0], si->color[1], si->color[2]);
	fprintf(f, "\t\t*MATERIAL_SHADING Phong\r\n");

	// print map info
	fprintf(f, "\t\t*MAP_DIFFUSE\t{\r\n");
	fprintf(f, "\t\t\t*MAP_NAME\t\"%s\"\r\n", shader->shader);
	fprintf(f, "\t\t\t*MAP_CLASS\t\"Bitmap\"\r\n");
	fprintf(f, "\t\t\t*MAP_SUBNO\t1\r\n");
	fprintf(f, "\t\t\t*MAP_AMOUNT\t1.0\r\n");
	fprintf(f, "\t\t\t*MAP_TYPE\tScreen\r\n");
	fprintf(f, "\t\t\t*BITMAP\t\"%s\"\r\n", filename);
	fprintf(f, "\t\t\t*BITMAP_FILTER\tPyramidal\r\n");
	fprintf(f, "\t\t}\r\n");

	fprintf(f, "\t}\r\n");
}



int WriteASEFile(char *filename)
{
	int             i, j, s, modelNum;
	FILE           *f;
	dshader_t      *shader;
	dmodel_t       *dm;
	drawSurface_t  *ds;
	entity_t       *e;
	vec3_t          origin;
	const char     *key;
	char            name[1024], base[1024];

	Sys_Printf("writing %s\n", filename);
	f = fopen(filename, "wb");
	if(!f)
		Error("Can't write %s\b", filename);

	// print header
	fprintf(f, "*3DSMAX_ASCIIEXPORT\t200\r\n");
	fprintf(f, "*COMMENT\t\"Generated by XMap (XreaL) -bsp2ase\"\r\n");
	fprintf(f, "*SCENE\t{\r\n");
	fprintf(f, "\t*SCENE_FILENAME\t\"%s\"\r\n", base);
	fprintf(f, "\t*SCENE_FIRSTFRAME\t0\r\n");
	fprintf(f, "\t*SCENE_LASTFRAME\t100\r\n");
	fprintf(f, "\t*SCENE_FRAMESPEED\t30\r\n");
	fprintf(f, "\t*SCENE_TICKSPERFRAME\t160\r\n");
	fprintf(f, "\t*SCENE_BACKGROUND_STATIC\t0.0000\t0.0000\t0.0000\r\n");
	fprintf(f, "\t*SCENE_AMBIENT_STATIC\t0.0000\t0.0000\t0.0000\r\n");
	fprintf(f, "}\r\n");

	// print materials
	fprintf(f, "*MATERIAL_LIST\t{\r\n");
	fprintf(f, "\t*MATERIAL_COUNT\t%d\r\n", numShaders);
	for(i = 0; i < numShaders; i++)
	{
		shader = &dshaders[i];
		ConvertShader(f, shader, i);
	}
	fprintf(f, "}\r\n");

	// walk entity list
	for(i = 0; i < numEntities; i++)
	{
		// get entity and model
		e = &entities[i];
		if(i == 0)
		{
			modelNum = 0;
		}
		else
		{
			key = ValueForKey(e, "model");
			if(key[0] != '*')
			{
				continue;
			}
			modelNum = atoi(key + 1);
		}
		dm = &dmodels[modelNum];

		// get entity origin
		key = ValueForKey(e, "origin");
		if(key[0] == '\0')
			VectorClear(origin);
		else
			GetVectorForKey(e, "origin", origin);

		// convert model
		for(j = 0; j < dm->numSurfaces; j++)
		{
			s = j + dm->firstSurface;
			ds = &drawSurfaces[s];

			ConvertSurface(f, dm, modelNum, ds, s, origin);
		}
	}

	// close the file
	fclose(f);

	// return to sender
	return 0;
}


/*
ConvertBspToASE()
exports an 3d studio ase file from the bsp
*/
int ConvertBspToASE(int argc, char **argv)
{
	int             i;
	double          start, end;
	char            source[1024];
	char            dest[1024];

	Sys_Printf("---- convert map to ase ----\n");

	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "-threads"))
		{
			numthreads = atoi(argv[i + 1]);
			i++;
		}
		else if(!strcmp(argv[i], "-v"))
		{
			Sys_Printf("verbose = true\n");
			verbose = qtrue;
		}
		else if(!strcmp(argv[i], "-connect"))
		{
			Broadcast_Setup(argv[++i]);
		}
		else if(argv[i][0] == '-')
			Error("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if(i != argc - 1)
	{
		Error("usage: xmap -bsp2ase [-<switch> [-<switch> ...]] <mapname.bsp>\n"
			  "\n" "Switches:\n" "   v              = verbose output\n");
		//"   quake1       = convert from QuakeWorld to XreaL\n"
		//"   quake2       = convert from Quake2 to XreaL\n"
		//"   quake3         = convert from Quake3 to XreaL\n"
		//"   quake4         = convert from Quake4 to XreaL\n");
	}

	start = I_FloatTime();

	ThreadSetDefault();

	SetQdirFromPath(argv[i]);

	strcpy(source, ExpandArg(argv[i]));
	StripExtension(source);
	DefaultExtension(source, ".bsp");

	// start from scratch
	LoadShaderInfo();

	Sys_Printf("reading %s\n", source);
	LoadBSPFile(source);

	ParseEntities();

	//
	strcpy(dest, ExpandArg(argv[i]));
	StripExtension(dest);
	strcat(dest, "_converted");
	DefaultExtension(dest, ".ase");

	WriteASEFile(dest);

	end = I_FloatTime();
	Sys_Printf("%5.0f seconds elapsed\n", end - start);

	// shut down connection
	Broadcast_Shutdown();


	return 0;
}
