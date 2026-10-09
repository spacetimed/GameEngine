#pragma once

#include <DirectXMath.h>
#include <vector>
#include <string>

namespace Sei::HUD
{
	struct DataItem
	{
		std::string key;
		const std::string* value = nullptr;
	};

	struct Section
	{
		DirectX::XMFLOAT2 textPosition = { 0.0f, 0.0f };
		DirectX::XMFLOAT4 textColor = { 1.0f, 1.0f, 1.0f, 0.5f };
		float fontSize = 12.0f; // px
		std::vector<DataItem> items;
	};
}
