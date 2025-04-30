#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec4 payload;

void main()
{
	payload = vec4(1.0f, 0.5f, 0.0f, 1.0f);
}
