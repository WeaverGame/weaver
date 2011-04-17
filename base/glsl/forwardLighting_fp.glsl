/*
===========================================================================
Copyright (C) 2007-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

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

/* forwardLighting_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;

#if defined(LIGHT_DIRECTIONAL)
uniform sampler2D	u_ShadowMap0;
uniform sampler2D	u_ShadowMap1;
uniform sampler2D	u_ShadowMap2;
uniform sampler2D	u_ShadowMap3;
uniform sampler2D	u_ShadowMap4;
#elif defined(LIGHT_PROJ)
uniform sampler2D	u_ShadowMap0;
#else
uniform samplerCube	u_ShadowMap;
#endif



uniform vec3		u_ViewOrigin;

#if defined(LIGHT_DIRECTIONAL)
uniform vec3		u_LightDir;
#else
uniform vec3		u_LightOrigin;
#endif
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform	float		u_LightWrapAround;

uniform mat4		u_ShadowMatrix[MAX_SHADOWMAPS];
uniform vec4		u_ShadowParallelSplitDistances;
uniform float       u_ShadowTexelSize;
uniform float       u_ShadowBlur;

uniform mat4		u_ViewMatrix;

uniform int         u_PortalClipping;
uniform vec4		u_PortalPlane;

uniform float		u_DepthScale;

varying vec3		var_Position;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
#if defined(USE_NORMAL_MAPPING)
varying vec2		var_TexSpecular;
#endif
varying vec4		var_TexAttenuation;
#if defined(USE_NORMAL_MAPPING)
varying vec4		var_Tangent;
varying vec4		var_Binormal;
#endif
varying vec4		var_Normal;
//varying vec4		var_Color;



/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors(const vec3 forward, inout vec3 right, inout vec3 up)
{
	// this rotate and negate guarantees a vector
	// not colinear with the original
	right.y = -forward.x;
	right.z = forward.y;
	right.x = forward.z;

	float d = dot(right, forward);
	right += forward * -d;
	normalize(right);
	up = cross(right, forward);	// GLSL cross product is the same as in Q3A
}


#if defined(LIGHT_DIRECTIONAL)
// TODO PCF for sun shadowing
#elif defined(LIGHT_PROJ)
vec4 PCF(vec4 shadowVert, float filterWidth, float samples)
{
	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;
	
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);
	for(float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for(float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			// moments += texture2DProj(u_ShadowMap0, vec3(shadowVert.xy + vec2(i, j), shadowVert.w));
			moments += texture2D(u_ShadowMap0, shadowVert.xy / shadowVert.w + vec2(i, j));
		}
	}
	
	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#else
vec4 PCF(vec3 I, float filterWidth, float samples)
{
	vec3 forward, right, up;
	
	forward = normalize(I);
	MakeNormalVectors(forward, right, up);

	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;
	
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);
	for(float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for(float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			moments += textureCube(u_ShadowMap, I + right * i + up * j);
		}
	}
	
	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#endif


#if defined(PCSS)
float SumBlocker(vec4 shadowVert, float vertexDistance, float filterWidth, float samples)
{
	float stepSize = 2.0 * filterWidth / samples;
	
	float blockerCount = 0.0;
    float blockerSum = 0.0;
    
	for(float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for(float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			float shadowDistance = texture2DProj(u_ShadowMap, vec3(shadowVert.xy + vec2(i, j), shadowVert.w)).x;
			// float shadowDistance = texture2D(u_ShadowMap, shadowVert.xy / shadowVert.w + vec2(i, j)).x;
			
			// FIXME VSM_CLAMP
			
			if(vertexDistance > shadowDistance)
			{
				blockerCount += 1.0;
				blockerSum += shadowDistance;
			}
		}
	}
	
	float result;
	if(blockerCount > 0.0)
		result = blockerSum / blockerCount;
	else
		result = 0.0;
	
	return result;
}

float EstimatePenumbra(float vertexDistance, float blocker)
{
	float penumbra;
	
	if(blocker == 0.0)
		penumbra = 0.0;
	else
		penumbra = ((vertexDistance - blocker) * u_LightRadius) / blocker;
	
	return penumbra;
}
#endif




void	main()
{
#if defined(USE_PORTAL_CLIPPING)
	{
		float dist = dot(var_Position.xyz, u_PortalPlane.xyz) - u_PortalPlane.w;
		if(dist < 0.0)
		{
			discard;
			return;
		}
	}
#endif

	float shadow = 1.0;
#if defined(USE_SHADOWING)

#if defined(LIGHT_DIRECTIONAL)

	vec4 shadowVert;
	vec4 shadowMoments;
	
	// transform to camera space
	vec4 Pcam = u_ViewMatrix * vec4(var_Position.xyz, 1.0);
	float vertexDistanceToCamera = -Pcam.z;
	
#if defined(r_ParallelShadowSplits_1)
	if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		#endif
	}
	else
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[1] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
		#endif
	}
#elif defined(r_ParallelShadowSplits_2)
	if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		#endif
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[1] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
		#endif
		
	}
	else
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[2] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
		#endif
	}
#elif defined(r_ParallelShadowSplits_3)
	if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		#endif
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[1] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
		#endif
		
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[2] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
		#endif
	}
	else
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[3] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
		#endif
	}
#elif defined(r_ParallelShadowSplits_4)
	if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		#endif
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[1] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
		#endif
		
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[2] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
		#endif
	}
	else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.w)
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[3] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
		#endif
	}
	else
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[4] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap4, shadowVert.xyw);
		#endif
	}
#else
	{
		#if defined(r_ShowParallelShadowSplits)
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
		#else
		shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
		shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
		#endif
	}
#endif

#elif defined(LIGHT_PROJ)

	vec4 shadowVert = u_ShadowMatrix[0] * vec4(var_Position.xyz, 1.0);
	
	// compute incident ray
	vec3 I = var_Position.xyz - u_LightOrigin;
	
	const float	SHADOW_BIAS = 0.001;
	float vertexDistance = length(I) / u_LightRadius - SHADOW_BIAS;
	
	#if defined(PCF_2X2)
	vec4 shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, 2.0);
	#elif defined(PCF_3X3)
	vec4 shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, 3.0);
	#elif defined(PCF_4X4)
	vec4 shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, 4.0);
	#elif defined(PCF_5X5)
	vec4 shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, 5.0);
	#elif defined(PCF_6X6)
	vec4 shadowMoments = PCF(shadowVert, u_ShadowTexelSize * u_ShadowBlur, 6.0);
	
	/*
	#elif defined(PCSS)
	
	// step 1: find blocker estimate
	
	float blockerSearchWidth = u_ShadowTexelSize * u_LightRadius / vertexDistance;
	float blockerSamples = 6.0; // how many samples to use for blocker search
	float blocker = SumBlocker(shadowVert, vertexDistance, blockerSearchWidth, blockerSamples);
	
	#if 0
	// uncomment to visualize blockers
	gl_FragColor = vec4(blocker * 0.3, 0.0, 0.0, 1.0);
	return;
	#endif
	
	// step 2: estimate penumbra using parallel planes approximation
	float penumbra = EstimatePenumbra(vertexDistance, blocker);
	
	#if 0
	// uncomment to visualize penumbrae
	gl_FragColor = vec4(0.0, 0.0, penumbra, 1.0);
	return;
	#endif
	
	// step 3: compute percentage-closer filter
	vec4 shadowMoments;
	if(penumbra > 0.0)
	{
		const float PCFsamples = 4.0;
		
		
		//float maxpen = PCFsamples * (1.0 / u_ShadowTexelSize);
		//if(penumbra > maxpen)
		//	penumbra = maxpen;
		//
	
		shadowMoments = PCF(shadowVert, penumbra, PCFsamples);
	}
	else
	{
		shadowMoments = texture2DProj(u_ShadowMap, shadowVert.xyw);
	}
	*/
	#else
	
	// no filter
	vec4 shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
	#endif

#else
	// compute incident ray
	vec3 I = var_Position.xyz - u_LightOrigin;

	#if defined(PCF_2X2)
	vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 2.0);
	#elif defined(PCF_3X3)
	vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 3.0);
	#elif defined(PCF_4X4)
	vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 4.0);
	#elif defined(PCF_5X5)
	vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 5.0);
	#elif defined(PCF_6X6)
	vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 6.0);
	#else
	vec4 shadowMoments = textureCube(u_ShadowMap, I);
	#endif
#endif


#if defined(VSM)
	{
		#if defined(VSM_CLAMP)
		// convert to [-1, 1] vector space
		shadowMoments = 2.0 * (shadowMoments - 0.5);
		#endif
	
		float shadowDistance = shadowMoments.r;
		float shadowDistanceSquared = shadowMoments.a;
		
		const float	SHADOW_BIAS = 0.001;

#if defined(LIGHT_DIRECTIONAL)
		float vertexDistance = shadowVert.z - SHADOW_BIAS;
#else
		float vertexDistance = length(I) / u_LightRadius - SHADOW_BIAS;
#endif
	
		// standard shadow map comparison
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistanceSquared;
		float Ex_2 = shadowDistance * shadowDistance;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		float variance = max(E_x2 - Ex_2, VSM_EPSILON);
		// float variance = smoothstep(VSM_EPSILON, 1.0, max(E_x2 - Ex_2, 0.0));
	
		// compute probabilistic upper bound
		float mD = shadowDistance - vertexDistance;
		float mD_2 = mD * mD;
		float p = variance / (variance + mD_2);
		
		#if defined(r_LightBleedReduction)
		p = smoothstep(r_LightBleedReduction, 1.0, p);
		#endif
	
		#if defined(DEBUG_VSM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_VSM & 1) != 0 ? variance : 0.0;
		gl_FragColor.g = (DEBUG_VSM & 2) != 0 ? mD_2 : 0.0;
		gl_FragColor.b = (DEBUG_VSM & 4) != 0 ? p : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#else
		shadow = max(shadow, p);
		#endif
	}
#elif defined(ESM)
	{		
		const float	SHADOW_BIAS = 0.001;
		
#if defined(LIGHT_DIRECTIONAL)
		float vertexDistance = shadowVert.z - SHADOW_BIAS;
#else
		float vertexDistance = (length(I) / u_LightRadius) * r_ShadowMapDepthScale;// - SHADOW_BIAS;
#endif
		
		float shadowDistance = shadowMoments.a;
		
		// exponential shadow mapping
		// shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
		shadow = clamp(exp(r_OverDarkeningFactor * (shadowDistance - vertexDistance)), 0.0, 1.0);
		// shadow = smoothstep(0.0, 1.0, shadow);
		
		#if defined(DEBUG_ESM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_ESM & 1) != 0 ? shadowDistance : 0.0;
		gl_FragColor.g = (DEBUG_ESM & 2) != 0 ? -(shadowDistance - vertexDistance) : 0.0;
		gl_FragColor.b = (DEBUG_ESM & 4) != 0 ? shadow : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#endif
	}
#endif // ESM

	if(shadow <= 0.0)
	{
		discard;
		return;
	}

#endif // USE_SHADOWING
	
	// compute light direction in world space
#if defined(LIGHT_DIRECTIONAL)
	vec3 L = u_LightDir;
#else
	vec3 L = normalize(u_LightOrigin - var_Position);
#endif

	vec2 texDiffuse = var_TexDiffuse.st;
	
#if defined(USE_NORMAL_MAPPING)

	// invert tangent space for twosided surfaces
	mat3 tangentToWorldMatrix;
#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		tangentToWorldMatrix = mat3(-var_Tangent.xyz, -var_Binormal.xyz, -var_Normal.xyz);
	}
	else
#endif
	{
		tangentToWorldMatrix = mat3(var_Tangent.xyz, var_Binormal.xyz, var_Normal.xyz);
	}


	vec2 texNormal = var_TexNormal.st;
	vec2 texSpecular = var_TexSpecular.st;
	
	// compute view direction in world space
	vec3 V = normalize(u_ViewOrigin - var_Position.xyz);
	
#if defined(USE_PARALLAX_MAPPING)
	
	// ray intersect in view direction
	
	mat3 worldToTangentMatrix;
	#if defined(GLHW_ATI) || defined(GLHW_ATI_DX10)
	worldToTangentMatrix = mat3(tangentToWorldMatrix[0][0], tangentToWorldMatrix[1][0], tangentToWorldMatrix[2][0],
								tangentToWorldMatrix[0][1], tangentToWorldMatrix[1][1], tangentToWorldMatrix[2][1], 
								tangentToWorldMatrix[0][2], tangentToWorldMatrix[1][2], tangentToWorldMatrix[2][2]);
	#else
	worldToTangentMatrix = transpose(tangentToWorldMatrix);
	#endif

	// compute view direction in tangent space
	vec3 Vts = worldToTangentMatrix * V;
	Vts = normalize(Vts);
	
	// size and start position of search in texture space
	vec2 S = Vts.xy * -u_DepthScale / Vts.z;
		
	float depth = RayIntersectDisplaceMap(texNormal, S, u_NormalMap);
	
	// compute texcoords offset
	vec2 texOffset = S * depth;
	
	texDiffuse.st += texOffset;
	texNormal.st += texOffset;
	texSpecular.st += texOffset;
#endif // USE_PARALLAX_MAPPING

	// compute half angle in world space
	vec3 H = normalize(L + V);

	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, texNormal.st).xyz - 0.5);
	#if defined(r_NormalScale)
	N.z *= r_NormalScale;
	normalize(N);
	#endif

	// transform normal into world space
	N = tangentToWorldMatrix * N;

	
#else // USE_NORMAL_MAPPING

	vec3 N;
#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		N = -normalize(var_Normal.xyz);
	}
	else
#endif
	{
		N = normalize(var_Normal.xyz);
	}
		
#endif // USE_NORMAL_MAPPING

	// compute the light term
#if defined(r_WrapAroundLighting)
	float NL = clamp(dot(N, L) + u_LightWrapAround, 0.0, 1.0) / clamp(1.0 + u_LightWrapAround, 0.0, 1.0);
#else
	float NL = clamp(dot(N, L), 0.0, 1.0);
#endif

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse.st);
	diffuse.rgb *= u_LightColor * NL;

#if defined(USE_NORMAL_MAPPING)
	// compute the specular term
	vec3 specular = texture2D(u_SpecularMap, texSpecular).rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#endif



	// compute light attenuation
#if defined(LIGHT_PROJ)
	vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, var_TexAttenuation.xyw).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(var_TexAttenuation.z + 0.5, 0.0)).rgb; // FIXME
	
#elif defined(LIGHT_DIRECTIONAL)
	vec3 attenuationXY = vec3(1.0);
	vec3 attenuationZ  = vec3(1.0);
	
#else
	vec3 attenuationXY = texture2D(u_AttenuationMapXY, var_TexAttenuation.xy).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(var_TexAttenuation.z, 0)).rgb;
#endif
				
	// compute final color
	vec4 color = diffuse;
	
#if defined(USE_NORMAL_MAPPING)
	color.rgb += specular;
#endif

#if !defined(LIGHT_DIRECTIONAL)
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
#endif
	
	color.rgb *= u_LightScale;
	color.rgb *= shadow;

	color.r *= var_TexDiffuse.p;
	color.gb *= var_TexNormal.pq;

	gl_FragColor = color;
	
#if 0
#if defined(USE_PARALLAX_MAPPING)
	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), diffuse.a);
#elif defined(USE_NORMAL_MAPPING)
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), diffuse.a);
#else
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), diffuse.a);
#endif
#endif

#if 0
#if defined(USE_VERTEX_SKINNING)
	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), diffuse.a);
#elif defined(USE_VERTEX_ANIMATION)
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), diffuse.a);
#else
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), diffuse.a);
#endif
#endif
}
