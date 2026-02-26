#pragma once
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include "MyMath.h"

class SpriteCommon;



//スプライト
class Sprite
{
public:

	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

	// getter
	const MyMath::Vector2& GetPosition() const { return position; }
	// setter
	void SetPosition(const MyMath::Vector2& position) { this->position = position; }
	
	// 回転
	float GetRotation() const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	// 色
	const MyMath::Vector4& GetColor() const { return materialData->color; }
	void SetColor(const MyMath::Vector4& color) { materialData->color = color; }

	// サイズ
	const MyMath::Vector2& GetSize() const { return size; }
	void SetSize(const MyMath::Vector2& size) { this->size = size; }

private:
	SpriteCommon* spriteCommon_ = nullptr;
	MyMath::Transform transform;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

	//頂点データ
	struct VertexData
	{
		MyMath::Vector4 position;
		MyMath::Vector2 texcoord;
		MyMath::Vector3 normal;
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
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		MyMath::Matrix4x4 uvTransform;
		float shininess;
	};
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//バッファリソース内のデータを示すポインタ
	Material* materialData = nullptr;

	//座標返還行列データ
	struct TransformationMatrix
	{
		MyMath::Matrix4x4 WVP;
		MyMath::Matrix4x4 World;
	};
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;
	//バッファリソース内のデータを示すポインタ
	TransformationMatrix* transforMatrixData = nullptr;

	//座標
	MyMath::Vector2 position = { 0.0f, 0.0f };
	float rotation = 0.0f;
	// サイズ
	MyMath::Vector2 size = { 100.0f, 100.0f };

};