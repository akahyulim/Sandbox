#pragma once
#include "Texture.h"

namespace Dive
{
	class Texture2D : public Texture
	{
	public:
		Texture2D() = default;
		virtual ~Texture2D() override;

		virtual bool Deserialize(const std::vector<uint8_t>& fileBuffer, std::wstring_view extension) override;

		virtual bool Create(Graphics* graphics) override;

	private:
	private:
	};
}