// Bias/scale helper functions
float2 undoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 doVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

float2 encodeVelocity(float4 position, float4 prevPosition)
{
	float2 qx = ((position.xy / position.w)
		- (prevPosition.xy / prevPosition.w))
		* uniforms.halfExposureFramerate;
	float lengthQX = length(qx);

	float weight = max(0.5, min(lengthQX, uniforms.maxMotionBlurRadius)) / (lengthQX + 0.001);

	return doVelocityBiasScale(qx * weight);
}