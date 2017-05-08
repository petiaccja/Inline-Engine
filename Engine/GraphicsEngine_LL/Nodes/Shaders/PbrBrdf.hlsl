//based on: https://learnopengl.com/#!PBR/Theory

//Trowbridge-Reitz GGX normal distribution function
//approximates the amount the surface's microfacets are aligned to the halfway vector 
//influenced by the roughness of the surface; this is the primary function approximating the microfacets
float DistributionGGX(float3 N,	//surface normal
					  float3 H,	//halfway vector between surface normal and light direction
					  float a	//roughness
					 )
{
	const float PI = 3.14159265;
	a = a * a; //the lighting looks more correct squaring the roughness
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

//GGX and Schlick-Beckmann approximation known as Schlick-GGX
//describes the self - shadowing property of the microfacets.When a surface is relatively rough 
//the surface's microfacets can overshadow other microfacets thereby reducing the light the surface reflects
float GeometrySchlickGGX(float NdotV, float k)
{
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

//k is a remapping of roughness for direct lighting
float kDirect(float a)
{
	float tmp = (a + 1);
	return tmp * tmp / 8;
}

//k is a remapping of roughness for IBL lighting
float kIBL(float a)
{
	return a * a / 2;
}

float GeometrySmith(float3 N,	//surface normal
					float3 V,	//view direciton
					float3 L,	//ligth direction
					float k		//remapped roughness (see above)
					)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, k); //view direction: geometry obstruction
	float ggx2 = GeometrySchlickGGX(NdotL, k); //light direction: geometry shadowing

	return ggx1 * ggx2;
}

//By pre-computing F0 for both dielectrics and conductors we can use the same Fresnel-Schlick approximation 
//for both types of surfaces, but we do have to tint the base reflectivity if we have a metallic surface
float3 getF0(float3 surfaceColor, 
			 float metalness
			)
{
	float3 F0 = float3(0.04, 0.04, 0.04); //base reflectivity that is approximated for most dielectric surfaces
	F0 = lerp(F0, surfaceColor, metalness);
	return F0;
}

//Fresnel Schlick approximation
//The Fresnel equation describes the ratio of surface reflection at different surface angles
float3 FresnelSchlick(float cosTheta,	//dot product of surface normal and view direction
					  float3 F0			//precomputed surface's response at normal incidence
					)
{
	float tmp = 1.0 - cosTheta;
	float tmp2 = tmp * tmp;
	return F0 + (1.0 - F0) * (tmp2 * tmp2 * tmp);
}

float3 getCookTorranceBRDF(float3 albedo,
						   float3 normal,		//normalize(Normal)
						   float3 viewDir,		//normalize(camPos - WorldPos)
						   float3 lightDir,		//normalize(lightPositions[i] - WorldPos)
						   float3 radiance,		//lightColors[i] * attenuation;
						   float roughness,
						   float metalness
)
{
	//halfway vector between surface normal and light direction
	float3 halfwayVec = normalize(viewDir + lightDir);

	const float PI = 3.14159265;

	float NdotV = max(dot(normal, viewDir), 0.0);
	float HdotV = max(dot(halfwayVec, viewDir), 0.0);
	float NdotL = max(dot(normal, lightDir), 0.0);

	float  D = DistributionGGX(normal, halfwayVec, roughness);
	//NdotV for IBL
	//HdotV for direct
	float3 F = FresnelSchlick(HdotV, getF0(albedo, metalness));
	float  G = GeometrySmith(normal, viewDir, lightDir, kDirect(roughness));

	float3 kS = F;
	float3 kD = float3(1, 1, 1) - kS;
	kD *= 1.0 - metalness;

	float3 c = albedo;
	float3 Li = radiance;

	float3 Lo = (kD * c / PI) + (F * Li) * (D * G * NdotL / (4 * NdotV * NdotL + 0.0001));
	return Lo;
}










// -------------------------------------------------------------------------------------------------------------------------------------------------
// - Based on http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr.pdf + additional research by Richard Nemeth -
// -------------------------------------------------------------------------------------------------------------------------------------------------

// From http://research.tri-ace.com/Data/DesignReflectanceModel_notes.pdf
float3 Fresnel(float3 F0, float VoH)
{
	float3 minus = sqrt(F0) - 1;
	float3 plus = sqrt(F0) + 1;

	float3 alpha = sqrt(1 - (minus * minus * (1 - VoH * VoH)) / (plus * plus));

	float3 plusVoH = plus * VoH;
	float3 minusVoH = minus * VoH;
	float3 plusAlpha = plus * alpha;
	float3 minusAlpha = minus * alpha;

	float3 left = (plusVoH + minusAlpha) / (plusVoH - minusAlpha);
	float3 right = (minusVoH + plusAlpha) / (minusVoH - plusAlpha);

	return 0.5f * (left * left + right * right);
}

// Fresnel approximation (if F0 is larger than 0.4, then use accurate Fresnel(..) function instead
float3 FresnelSchlick(in float3 F0, in float VoH)
{
	return F0 + (1 - F0) * pow(1.f - VoH, 5.f);
}

float Vis_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	// Original formulation of G_SmithGGX Correlated
	// lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
	// lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

	// This is the optimize version
	float alphaG2 = alphaG * alphaG;

	// Caution : the " NdotL *" and " NdotV *" are explicitely inversed , this is not a mistake .
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
	return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float GGX(float NdotH, float m)
{
	float m2 = m * m;
	float f = (NdotH * m2 - NdotH) * NdotH + 1;
	return m2 / (f * f) / 3.14159265358979;
}

// Only used for DiffuseBurleyBRDF
float3 FresnelSchlick(float3 F0, float F90, float VoH)
{
	return F0 + (F90 - F0) * pow(1.f - VoH, 5.f);
}

// Disney Diffuse (Burley)
float3 DiffuseBurleyBRDF(float NdotV, float NdotL, float LdotH, float roughness, float3 F0)
{
	float linearRoughness = roughness; // TODO

	// Burley
	float energyBias = lerp(0, 0.5, linearRoughness);
	float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
	float lightScatter = FresnelSchlick(1.0, fd90, NdotL).r;
	float viewScatter = FresnelSchlick(1.0, fd90, NdotV).r;

	// Richard's research on how the diffuse will decrease based on surface F0, output is [0,1]
	float3 diffuseF0Loss = 1 - F0 * (0.530548 * F0 + 0.469452);

	return lightScatter * viewScatter * energyFactor * diffuseF0Loss / 3.14159265358979;
}

// Traditional specular microfacet brdf extended with indirect micro bounces
float3 SpecularMicrofacetBRDF(float3 F0, float NoV, float NoL, float NoH, float VoH, float roughness)
{
	float3 F = Fresnel(F0, VoH);
	float Vis = Vis_SmithGGXCorrelated(NoV, NoL, roughness);
	float D = GGX(NoH, roughness);

	float3 indirectMicroSpecular = 0.0f;// ... Read from 3DLUT(F0, NoL, roughness);

	return D * (F * Vis + indirectMicroSpecular);
}
// Standard brdf -> specular + diffuse
float3 StandardBRDF(float3 F0, float NoV, float NoL, float NoH, float VoH, float roughness)
{
	float specular = SpecularMicrofacetBRDF(F0, NoV, NoL, NoH, VoH, roughness);
	float diffuse = DiffuseBurleyBRDF(NoV, NoL, VoH, roughness, F0);

	return specular + diffuse;
}