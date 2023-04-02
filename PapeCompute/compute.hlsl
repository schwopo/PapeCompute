RWTexture2D<float4> target : register(u0);

struct RenderParams
{
	float3 eye;
	float3 target;
};

RenderParams renderParams : register(b0);

static const float3 gradientTop = float3( 1.0, 0.0, 0.0 );
static const float3 gradientBottom = float3( 0.0, 0.0, 0.0 );
static const int maxDepth = 3;

float noise(float r)
{
	return frac(19999.0 * sin(74742.0 * (r + 3883.03838)));
}

float noise(float3 v)
{
	float r = dot(v, float3(0.33332134, 0.9986777, 0.748882));
	return noise(r);
}


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

float3 getDivergentColor(float3 rayDir)
{
	float mixValue = (rayDir.y + 1.0) / 2.0;
	return lerp(gradientTop, gradientBottom, mixValue);
}

float invSqDst(float3 v, float3 w)
{
	return 1.0 / sqrt(
		pow(v.x - w.x, 2)
		+ pow(v.y - w.y, 2)
		+ pow(v.z - w.z, 2)
	);
}

float sdf(float3 samplePoint)
{
	float radius = 0.3;
	float threshold = 0.1;
	float3 position = float3(0.0, 0.0, 1.0);
	float3 position2 = float3(1.2, 0.0, 1.0);

	float pointBrightness = 0.0;
	pointBrightness += invSqDst(samplePoint, position);
	pointBrightness += invSqDst(samplePoint, position2);

	return 3.0- (pointBrightness);
}


float3 normalAtPoint(float3 p)
{
	float epsilon = 0.001;
	float dx = sdf(p + float3(epsilon, 0.0, 0.0)) - sdf(p);
	float dy = sdf(p + float3(0.0, epsilon, 0.0)) - sdf(p);
	float dz = sdf(p + float3(0.0, 0.0, epsilon)) - sdf(p);

	return normalize(float3(dx, dy, dz));
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	Material material;

	material.ambient = float3(0.1, 0.0, 0.0);
	material.diffuse = float3(0.8, 0.2, 0.2);
	material.specular = float3(1.0, 1.0, 1.0);
	material.exponent = 16.0;
	material.isReflective = true;

	uint width, height;
	target.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	uv.x *= (float)width/(float)height;

	float3 outColor = float3(0.1, 0.2, 0.2);

	float3 eyePoint = renderParams.eye;
	float3 targetPoint = renderParams.target;
	float3 up = float3(0.0, 1.0, 0.0);
	float3 lookDir = targetPoint - eyePoint; // this will lead to orthographic projection
	float3 rayDir = lookDir;

	float3 right = cross(up, lookDir); // might be inverted
	float3 upward = cross(lookDir, right);

	float2 offset = float2(-0.5, -0.5) + uv.xy;

	float3 startPoint = eyePoint + offset.x * right + offset.y * upward;
	float3 samplePoint = startPoint;
	float3 localLightPosition = float3(0.0, 1.0, 0.5);
	float step = 0.01;


	for (int depth = 0; depth < maxDepth; ++depth)
	{
		bool isDivergent = true;
		for (int i = 0; i < 100; ++i)
		{
			if (sdf(samplePoint) < 0.0)
			{
				isDivergent = false;

				if (material.isReflective)
				{
					// Set up for next ray march iteration
					startPoint = samplePoint;
					rayDir = normalAtPoint(samplePoint);
					samplePoint += rayDir * step * 2.0;
				}
				else
				{
					// Compute local lighting model
					outColor = material.evaluate(
						normalize(eyePoint - samplePoint),
						normalize(samplePoint - localLightPosition),
						normalAtPoint(samplePoint) );
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