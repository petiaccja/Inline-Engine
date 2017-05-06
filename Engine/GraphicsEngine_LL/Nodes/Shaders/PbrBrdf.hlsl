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