#pragma once
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class SpriteCommon;



//スプライト
class Sprite
{
public:

	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;
	};
	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	struct Vector2
	{
		float x;
		float y;
	};

	struct Matrix4x4
	{
		float m[4][4];
	};

	struct Transform
	{
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	// 単位行列
	Matrix4x4 MakeIdentity4x4() {
		Matrix4x4 identity;
		identity.m[0][0] = 1.0f;
		identity.m[0][1] = 0.0f;
		identity.m[0][2] = 0.0f;
		identity.m[0][3] = 0.0f;
		identity.m[1][0] = 0.0f;
		identity.m[1][1] = 1.0f;
		identity.m[1][2] = 0.0f;
		identity.m[1][3] = 0.0f;
		identity.m[2][0] = 0.0f;
		identity.m[2][1] = 0.0f;
		identity.m[2][2] = 1.0f;
		identity.m[2][3] = 0.0f;
		identity.m[3][0] = 0.0f;
		identity.m[3][1] = 0.0f;
		identity.m[3][2] = 0.0f;
		identity.m[3][3] = 1.0f;
		return identity;
	}

	// 4x4の掛け算
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result;
		result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] +
			m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
		result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] +
			m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
		result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] +
			m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
		result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] +
			m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

		result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] +
			m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
		result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] +
			m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
		result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] +
			m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
		result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] +
			m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

		result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] +
			m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
		result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] +
			m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
		result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] +
			m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
		result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] +
			m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

		result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] +
			m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
		result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] +
			m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
		result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] +
			m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
		result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] +
			m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

		return result;
	}

	// X軸で回転
	Matrix4x4 MakeRotateXMatrix(float radian) {
		float cosTheta = std::cos(radian);
		float sinTheta = std::sin(radian);
		return { 1.0f, 0.0f,      0.0f,     0.0f, 0.0f, cosTheta, sinTheta, 0.0f,
				0.0f, -sinTheta, cosTheta, 0.0f, 0.0f, 0.0f,     0.0f,     1.0f };
	}

	// Y軸で回転
	Matrix4x4 MakeRotateYMatrix(float radian) {
		float cosTheta = std::cos(radian);
		float sinTheta = std::sin(radian);
		return { cosTheta, 0.0f, -sinTheta, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				sinTheta, 0.0f, cosTheta,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	}

	// Z軸で回転
	Matrix4x4 MakeRotateZMatrix(float radian) {
		float cosTheta = std::cos(radian);
		float sinTheta = std::sin(radian);
		return { cosTheta, sinTheta, 0.0f, 0.0f, -sinTheta, cosTheta, 0.0f, 0.0f,
				0.0f,     0.0f,     1.0f, 0.0f, 0.0f,      0.0f,     0.0f, 1.0f };
	}

	// Affine変換
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate,
		const Vector3& translate) {
		Matrix4x4 result = Multiply(
			Multiply(MakeRotateXMatrix(rotate.x), MakeRotateYMatrix(rotate.y)),
			MakeRotateZMatrix(rotate.z));
		result.m[0][0] *= scale.x;
		result.m[0][1] *= scale.x;
		result.m[0][2] *= scale.x;

		result.m[1][0] *= scale.y;
		result.m[1][1] *= scale.y;
		result.m[1][2] *= scale.y;

		result.m[2][0] *= scale.z;
		result.m[2][1] *= scale.z;
		result.m[2][2] *= scale.z;

		result.m[3][0] = translate.x;
		result.m[3][1] = translate.y;
		result.m[3][2] = translate.z;
		return result;
	}

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
		return
		{
		2.0f / (right - left),0.0f,0.0f,0.f,
		0.0f,2.0f / (top - bottom),0.0f,0.0f,
		0.0f,0.0f,1.0f / (farClip - nearClip),0.0f,
		(left + right) / (left - right),(top + bottom) / (bottom - top),(nearClip - farClip),1.0f
		};
	}

private:
	SpriteCommon* spriteCommon_ = nullptr;

	//頂点データ
	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite;
	//バッファリソース内のデータを示すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};

	//マテリアルデータ
	struct Material
	{
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		float shininess;
	};
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//バッファリソース内のデータを示すポインタ
	Material* materialData = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	//座標返還行列データ
	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;
	//バッファリソース内のデータを示すポインタ
	TransformationMatrix* transforMatrixData = nullptr;

	Transform transform;

};