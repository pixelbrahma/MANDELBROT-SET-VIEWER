#version 410

out vec4 colorOut;
uniform double screen_ratio;
uniform dvec2 screen_size;
uniform dvec2 center;
uniform double zoom;
uniform int iterations;

vec4 Map_To_Color(float t)
{
	float r = 9.0 * (1.0 - t) * t * t * t;
	float g = 15.0 * (1.0 - t) * (1.0 - t) * t * t;
	float b = 8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t;

	return vec4(r, g, b, 1.0);
}

void main()
{
	dvec2 z, c;
	c.x = screen_ratio * (gl_FragCoord.x / screen_size.x - 0.5);
	c.y = (gl_FragCoord.y / screen_size.y - 0.5);
	c.x /= zoom;
	z.y /= zoom;
	c.x += center.x;
	c.y += center.y;

	int i;
	for(i=0;i<iterations;i++)
	{
		double x = ((z.x * z.x - z.y * z.y) + c.x) * 1.4;
		double y = ((z.x * z.y + z.y * z.x) + c.y) * 1.4;

		if((x + x + y + y) > 4.0)
		break;
		z.x = x;
		z.y = y;
	}

	double t = double(i) / double(iterations);
	colorOut = Map_To_Color(float(t));
}