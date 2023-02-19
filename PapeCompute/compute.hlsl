RWTexture2D<float4> target : register(u0);

static const float3 gradientTop = float3( 1.0, 0.0, 0.0 );
static const float3 gradientBottom = float3( 0.0, 0.0, 0.0 );
static const int maxDepth = 3;

struct Material
{
	bool isReflective;
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float exponent;

	float3 evaluate(float3 eye, float3 light, float3 normal)
	{
		return ambient
			+ diffuse * dot(-light, normal)
			+ specular * pow(
				clamp(
					dot(
						reflect(light, normal),
						eye
					),
					0.0, 1.0),
				exponent);
	}
};

struct Sphere
{
	float3 position;
	float radius;
	Material material;

	float sdf(float3 samplepoint)
	{
		return length(samplepoint - position) - radius;
	}

	float3 normal(float3 samplePoint)
	{
		return normalize(samplePoint - position);
	}
};

float3 getDivergentColor(float3 rayDir)
{
	float mixValue = (rayDir.y + 1.0) / 2.0;
	return lerp(gradientTop, gradientBottom, mixValue);
}


//float3 normalAtPoint(float3 p)
//{
//	float epsilon = 0.001;
//	float dx = sdf(p + float3(epsilon, 0.0, 0.0)) - sdf(p);
//	float dy = sdf(p + float3(0.0, epsilon, 0.0)) - sdf(p);
//	float dz = sdf(p + float3(0.0, 0.0, epsilon)) - sdf(p);
//
//	return normalize(float3(dx, dy, dz));
//}

//float4 colorAtPoint(float3 p)
//{
//	float3 eyeDir = float3(0.0, 0.0, -1.0);
//	return dot(normalAtPoint(p), eyeDir);
//}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	Sphere sphere;
	sphere.radius = 0.3;
	sphere.position = float3(0.0, 0.0, 1.0);
	sphere.material.ambient = float3(0.1, 0.0, 0.0);
	sphere.material.diffuse = float3(0.8, 0.2, 0.2);
	sphere.material.specular = float3(1.0, 1.0, 1.0);
	sphere.material.exponent = 16.0;
	sphere.material.isReflective = true;

	uint width, height;
	target.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	uv.x *= (float)width/(float)height;

	float3 outColor = float3(0.1, 0.2, 0.2);

	float3 rayDir = float3(0.0, 0.0, 1.0);
	float3 eyePoint = float3(0.0, 0.0, 0.0);
	float3 startPoint = eyePoint + float3(float2(-0.5, -0.5).xy + uv.xy, 0.0);
	float3 samplePoint = startPoint;
	float3 localLightPosition = float3(0.0, 1.0, 0.5);
	float step = 0.01;


	for (int depth = 0; depth < maxDepth; ++depth)
	{
		bool isDivergent = true;
		for (int i = 0; i < 100; ++i)
		{
			if (sphere.sdf(samplePoint) < 0.0)
			{
				isDivergent = false;

				if (sphere.material.isReflective)
				{
					// Set up for next ray march iteration
					startPoint = samplePoint;
					rayDir = sphere.normal(samplePoint);
					samplePoint += rayDir * step * 2.0;
				}
				else
				{
					// Compute local lighting model
					outColor = sphere.material.evaluate(
						normalize(eyePoint - sphere.position),
						normalize(samplePoint - localLightPosition),
						sphere.normal(samplePoint) );
				}

				break;
			}
			else
			{
				samplePoint += rayDir * step;
			}
		}

		if (isDivergent)
		{
			outColor = getDivergentColor(rayDir);
			break;
		}
	}

	target[DTid.xy] = float4(outColor, 1.0);
}