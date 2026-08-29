#pragma once

namespace Dive
{
	struct StaticGeometryData;

	enum class ePresetType
	{
		Triangle,
		Quad,
		Plane,
		Cube,
		Sphere,
		Capsule,
		None
	};

	class Preset
	{
	public:
		static StaticGeometryData GenerateTriangle();
		static StaticGeometryData GenerateQuad();
		static StaticGeometryData GeneratePlane();
		static StaticGeometryData GenerateCube();
		static StaticGeometryData GenerateSphere();
		static StaticGeometryData GenerateCapsule();
	};
}