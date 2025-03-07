#version 460
#extension GL_EXT_ray_tracing : enable

layout(set = 1, binding = 0, rgba8) uniform image2D storageImage;

void main()
{
	ivec2 pixelCoord = ivec2(gl_LaunchIDEXT.xy);

	imageStore(storageImage, pixelCoord, vec4(1.0f, 0.0f, 1.0f, 1.0f));
}
