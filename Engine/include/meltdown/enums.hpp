#pragma once

namespace mtd
{
	/*
	* @brief Identifier for the pipeline shader stage.
	*/
	enum class ShaderStage
	{
		Vertex,
		Fragment,
		VertexAndFragment
	};

	/*
	* @brief Types of descriptors supported by the engine.
	*/
	enum class DescriptorType
	{
		UniformBuffer
	};

	/*
	* @brief Types of mesh supported by the engine. Each mesh type has a specific set of
	* properties that can be accessed in the shaders.
	*/
	enum class MeshType
	{
		Default3D,
		MultiMaterial3D,
		Billboard
	};

	/*
	* @brief Types of material attributes stored as a float or a vector of floats.
	*/
	enum class MaterialFloatDataType
	{
		DiffuseColor
	};

	/*
	* @brief Types of material attributes stored as a texture.
	*/
	enum class MaterialTextureType
	{
		DiffuseMap
	};

	/*
	* @brief Enumeration describing how vertices will be assembled to create points, lines or triangles.
	*/
	enum class ShaderPrimitiveTopology
	{
		PointList,
		LineList,
		LineStrip,
		LineListAdjacent,
		LineStripAdjacent,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		TriangleListAdjacent,
		TriangleStripAdjacent
	};

	/*
	* @brief Method used for face culling during rasterization.
	* The orientation defines the front face of the triangles, while the backface will be culled.
	*/
	enum class ShaderFaceCulling
	{
		Clockwise,
		Counterclockwise,
		None
	};

	/*
	* @brief Allowed configuration of attachments for a framebuffer.
	*/
	enum class FramebufferAttachments
	{
		SingleColor,
		SingleColorWithDepth,
		TwoColors,
		TwoColorsWithDepth,
		ThreeColors,
		ThreeColorsWithDepth,
		FourColors,
		FourColorsWithDepth
	};

	/*
	* @brief Filtering type used for sampling textures.
	*/
	enum class TextureSamplingFilterType
	{
		Nearest,
		Linear
	};

	/*
	* @brief Enumeration of the physical keys that can be pressed in the keyboard.
	*/
	enum class KeyCode
	{
		Space = 0x20,
		Apostrophe = 0x27,
		Comma = 0x2C,
		Minus = 0x2D,
		Period = 0x2E,
		Slash = 0x2F,
		Num0 = 0x30,
		Num1 = 0x31,
		Num2 = 0x32,
		Num3 = 0x33,
		Num4 = 0x34,
		Num5 = 0x35,
		Num6 = 0x36,
		Num7 = 0x37,
		Num8 = 0x38,
		Num9 = 0x39,
		Semicolon = 0x3B,
		Equal = 0x3D,
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,
		OpenBracket = 0x5B,
		Backslash = 0x5C,
		CloseBracket = 0x5D,
		GraveAccent = 0x60,
		World1 = 0xA1,
		World2 = 0xA2,
		Escape = 0x100,
		Enter = 0x101,
		Tab = 0x102,
		Backspace = 0x103,
		Insert = 0x104,
		Delete = 0x105,
		RightArrow = 0x106,
		LeftArrow = 0x107,
		DownArrow = 0x108,
		UpArrow = 0x109,
		PageUp = 0x10A,
		PageDown = 0x10B,
		Home = 0x10C,
		End = 0x10D,
		CapsLock = 0x118,
		ScrollLock = 0x119,
		NumLock = 0x11A,
		PrintScreen = 0x11B,
		Pause = 0x11C,
		F1 = 0x122,
		F2 = 0x123,
		F3 = 0x124,
		F4 = 0x125,
		F5 = 0x126,
		F6 = 0x127,
		F7 = 0x128,
		F8 = 0x129,
		F9 = 0x12A,
		F10 = 0x12B,
		F11 = 0x12C,
		F12 = 0x12D,
		F13 = 0x12E,
		F14 = 0x12F,
		F15 = 0x130,
		F16 = 0x131,
		F17 = 0x132,
		F18 = 0x133,
		F19 = 0x134,
		F20 = 0x135,
		F21 = 0x136,
		F22 = 0x137,
		F23 = 0x138,
		F24 = 0x139,
		F25 = 0x13A,
		NumPad0 = 0x140,
		NumPad1 = 0x141,
		NumPad2 = 0x142,
		NumPad3 = 0x143,
		NumPad4 = 0x144,
		NumPad5 = 0x145,
		NumPad6 = 0x146,
		NumPad7 = 0x147,
		NumPad8 = 0x148,
		NumPad9 = 0x149,
		NumPadDecimal = 0x14A,
		NumPadDivide = 0x14B,
		NumPadMultiply = 0x14C,
		NumPadSubtract = 0x14D,
		NumPadAdd = 0x14E,
		NumPadEnter = 0x14F,
		NumPadEqual = 0x150,
		LeftShift = 0x154,
		LeftControl = 0x155,
		LeftAlt = 0x156,
		LeftSuper = 0x157,
		RightShift = 0x158,
		RightControl = 0x159,
		RightAlt = 0x15A,
		RightSuper = 0x15B,
		Menu = 0x15C
	};
}
