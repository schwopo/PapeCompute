RWTexture2D<float4> target : register(u0);

float4 gradientTop = float4( 1.0, 1.0, 1.0, 1.0 );
float4 gradientBottom = float4( 1.0, 0.0, 1.0, 1.0 );

float sdf(float3 samplepoint)
{
	float sphereRadius = 0.3;
	float3 spherePosition = float3(0.0, 0.0, 1.0);
	return length(samplepoint - spherePosition) - sphereRadius;
}

float3 normalAtPoint(float3 p)
{
	float epsilon = 0.001;
	float dx = sdf(p + float3(epsilon, 0.0, 0.0)) - sdf(p);
	float dy = sdf(p + float3(0.0, epsilon, 0.0)) - sdf(p);
	float dz = sdf(p + float3(0.0, 0.0, epsilon)) - sdf(p);

	return normalize(float3(dx, dy, dz));
}

float4 colorAtPoint(float3 p)
{
	float3 eyeDir = float3(0.0, 0.0, -1.0);
	return dot(normalAtPoint(p), eyeDir);
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint width, height;
	target.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float4 outColor = float4(0.1, 0.2, 0.2, 1.0);

	float3 rayDir = float3(0.0, 0.0, 1.0);
	const float3 startPoint = float3(float2(-0.5, -0.5).xy + uv.xy, 0.0);
	float3 samplePoint = startPoint;
	float step = 0.01;


	for (int i = 0; i < 100; ++i)
	{
		if (sdf(samplePoint) < 0.0)
		{
			outColor = colorAtPoint(samplePoint);
			break;
		}
		else
		{
			samplePoint += rayDir * step;
		}
	}

	target[DTid.xy] = outColor;
}