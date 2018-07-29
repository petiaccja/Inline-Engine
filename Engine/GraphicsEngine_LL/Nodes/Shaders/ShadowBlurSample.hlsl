#if QUALITY == MEDIUM
static const int numSteps = 5-1;

static const float weights[5] =
{
  0.294118,
  0.117647,
  0.235294,
  0.117647,
  0.235294
};

static const float offsets[4] = 
{
  2.0,
  1.0,
  -2.0,
  -1.0
};
#endif

#if QUALITY == HIGH
static const int numSteps = 11-1;

static const float weights[11] =
{
  0.209857,
  0.00556439,
  0.0222576,
  0.0612083,
  0.122417,
  0.183625,
  0.00556439,
  0.0222576,
  0.0612083,
  0.122417,
  0.183625
};

static const float offsets[10] = 
{
  5.0,
  4.0,
  3.0,
  2.0,
  1.0,
  -5.0,
  -4.0, 
  -3.0,
  -2.0,
  -1.0
};
#endif

#if QUALITY == ULTRA
static const int numSteps = 23-1;

static const float weights[23] =
{
  0.154981,
  4.84288e-006,
  3.87431e-005,
  0.000222773,
  0.000980199,
  0.0034307,
  0.00980199,
  0.0232797,
  0.0465595,
  0.0791511,
  0.115129,
  0.143911,
  4.84288e-006,
  3.87431e-005,
  0.000222773,
  0.000980199,
  0.0034307,
  0.00980199,
  0.0232797,
  0.0465595,
  0.0791511,
  0.115129,
  0.143911
};

static const float offsets[22] = 
{
  11.0,
  10.0,
  9.0,
  8.0,
  7.0,
  6.0,
  5.0,
  4.0,
  3.0,
  2.0,
  1.0,
  -11.0,
  -10.0,
  -9.0,
  -8.0,
  -7.0,
  -6.0,
  -5.0,
  -4.0,
  -3.0,
  -2.0,
  -1.0
};
#endif

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;// / far;
};

float2 GetStepSize( float2 direction, float3 normal, float depth, float threshold )
{
  return direction 
         //* light_size * light_size //included in the penumbra
         * sqrt( max( dot( float3( 0.0, 0.0, 1.0 ), normal ), threshold ) )
         * (1 / (depth/* * depth * 100*/));
}

float GetShadow( float2 texCoord, const int layer )
{
	float4 shadow = inputTex4.Sample(samp1, texCoord);
	
	if(layer == 0)
	{
		return shadow.x;
	}
	else if(layer == 1)
	{
		return shadow.y;
	}
	else if(layer == 2)
	{
		return shadow.z;
	}
	else
	{
		return shadow.w;
	}
}

float Blur( float2 stepSize, float2 texCoord, float shadow, float depth, float penumbra, const int layer)
{
	if(penumbra <= 0.001)
	{
		return shadow;
	}
	
	const float maxDepthDiff = 0.01;
	
	float color = shadow * weights[0];
	
	float sumWeights = weights[0];
	
	for(int c = 0; c < numSteps; ++c)
	{
		float2 sampleLoc = texCoord + stepSize * (offsets[c] * penumbra);
		float depthAtSample = inputTex0.Sample(samp0, sampleLoc).x;
		
		if( abs( depth - depthAtSample ) < maxDepthDiff )
		{
			color += GetShadow( sampleLoc, layer ) * weights[c + 1];
			sumWeights += weights[c + 1];
		}
	}
	
	return color / sumWeights;	
}