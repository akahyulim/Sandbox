#pragma once
#include <yaml-cpp/yaml.h>
#include <DirectXMath.h>

namespace YAML
{
	template<>
	struct convert<DirectX::XMFLOAT2>
	{
		static Node encode(const DirectX::XMFLOAT2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
	
			return true;
		}
	};

	template<>
	struct convert<DirectX::XMFLOAT3>
	{
		static Node encode(const DirectX::XMFLOAT3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();

			return true;
		}
	};

	template<>
	struct convert<DirectX::XMFLOAT4>
	{
		static Node encode(const DirectX::XMFLOAT4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}
	};

	template<>
	struct convert<DirectX::XMFLOAT4X4>
	{
		static Node encode(const DirectX::XMFLOAT4X4& rhs)
		{
			Node node;
			node.push_back(rhs._11);	node.push_back(rhs._12);	node.push_back(rhs._13);	node.push_back(rhs._14);
			node.push_back(rhs._21);	node.push_back(rhs._22);	node.push_back(rhs._23);	node.push_back(rhs._24);
			node.push_back(rhs._31);	node.push_back(rhs._32);	node.push_back(rhs._33);	node.push_back(rhs._34);
			node.push_back(rhs._41);	node.push_back(rhs._42);	node.push_back(rhs._43);	node.push_back(rhs._44);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT4X4& rhs)
		{
			if (!node.IsSequence() || node.size() != 16)
				return false;

			rhs._11 = node[0].as<float>();	rhs._12 = node[1].as<float>();	rhs._13 = node[2].as<float>();	rhs._14 = node[3].as<float>();
			rhs._21 = node[4].as<float>();	rhs._22 = node[5].as<float>();	rhs._23 = node[6].as<float>();	rhs._24 = node[7].as<float>();
			rhs._31 = node[8].as<float>();	rhs._32 = node[9].as<float>();	rhs._33 = node[10].as<float>();	rhs._34 = node[11].as<float>();
			rhs._41 = node[12].as<float>();	rhs._42 = node[13].as<float>();	rhs._43 = node[14].as<float>();	rhs._44 = node[15].as<float>();

			return true;
		}
	};
}

namespace Dive
{
	static YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT2& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT3& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4& quat)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << quat.x << quat.y << quat.z << quat.w << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4X4& mat)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq
			<< mat._11 << mat._12 << mat._13 << mat._14
			<< mat._21 << mat._22 << mat._23 << mat._24
			<< mat._31 << mat._32 << mat._33 << mat._34
			<< mat._41 << mat._42 << mat._43 << mat._44 << YAML::EndSeq;
		return out;
	}
}
