#version 460
#extension GL_EXT_ray_tracing : enable


struct Payload
{
	vec3 light;
	vec3 throughput;
	uint recursionDepth;
	uint randomState;
};


layout(location = 0) rayPayloadInEXT Payload payload;


void main()
{
	if(payload.recursionDepth > 0)
		payload.light += payload.throughput * vec3(10.0f);
	else
		payload.light += vec3(0.428f, 1.5f, 1000.0f);
}
