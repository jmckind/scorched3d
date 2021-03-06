// -*- mode: C; -*-

// it seems we can define at max. 8 varying parameters...
varying vec3 viewerdir;
varying vec3 lightdir;
varying vec3 normal;
//varying vec2 foamtexcoord; // coords for the actual tiled foam texture
varying vec3 aoftexcoord; // coords for the aof texture for open sea
varying vec4 reflectiontexcoord;	// x,y,w
varying vec4 noise_texc;

uniform vec3 landfoam;
uniform vec3 viewpos;
uniform vec3 upwelltop;
uniform vec3 upwellbot;
uniform vec3 upwelltopbot;
uniform vec3 noise_xform_0;
uniform vec3 noise_xform_1;

const float virtualplane_height = 12.0;

void main()
{
	// shadow coords
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[2] = gl_TextureMatrix[2] * vertex;

	// normalize vertex normal
	vec3 N = normalize(gl_Normal);
	normal = N;

	// compute upwelling color (slope dependent)
	// that is (inputpos.z + viewpos.z + 3) / 9 + (normal.z - 0.8);
	// or (inputpos.z + viewpos.z) / 9 + normal.z + 1/3 - 0.8
	// or (inputpos.z + viewpos.z) * 1/9 + normal.z - 7/15
	// 1/9 = 0.1111111 , 7/15 = 0.466667
	gl_FrontColor.xyz = upwelltopbot *
		clamp((gl_Vertex.z + viewpos.z) * 0.1111111 + N.z - 0.4666667, 0.0, 1.0) + upwellbot;
		
	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	viewerdir = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));

	// transform vertex to projection space (clip coordinates)
	gl_Position = ftransform();	
	gl_FogFragCoord = max(gl_Position.z - 350.0, 0.0);

	// transform light pos to object space. we assume mvinv has no projection coefficients.
	// light is directional, so lightpos.w = 0
	lightdir = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));

	// transform noise coordinates
	noise_texc.xy = vec2(gl_Vertex) / 2.0 * noise_xform_0.z + noise_xform_0.xy;
	noise_texc.zw = vec2(gl_Vertex) / 2.0 * noise_xform_1.z + noise_xform_1.xy;

	// transform inputpos.xy with texture matrix to get texture coodinates
	//fixme: use uniforms here as well, no tex matrix.	
	aoftexcoord = vec3(gl_Vertex.x + landfoam.x, gl_Vertex.y + landfoam.y, landfoam.z);	
	//foamtexcoord = (gl_TextureMatrix[0] * gl_Vertex).xy;

	// compute reflection texture coordinates
	// formula to compute them from inputpos (coord):
	// vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
	// texc.z -= VIRTUAL_PLANE_HEIGHT;
	// uv1[ptr] = texc;
	// after that mulitply with texture matrix!
	vec3 texc = N * (virtualplane_height * N.z) + vec3(gl_Vertex);
	texc.z -= virtualplane_height;
	reflectiontexcoord = gl_TextureMatrix[1] * vec4(texc, 1.0);
}
