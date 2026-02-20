#pragma once
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>

class SpriteCommon;

//struct Vector4
//{
//	float x;
//	float y;
//	float z;
//	float w;
//};
//struct Vector3
//{
//	float x;
//	float y;
//	float z;
//};
//
//struct Vector2
//{
//	float x;
//	float y;
//};

//スプライト
class Sprite
{
public:

	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

private:
	SpriteCommon* spriteCommon = nullptr;

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

	////マテリアルデータ
	//struct Material
	//{
	//	Vector4 color;
	//	int32_t enableLighting;
	//	float padding[3];
	//	Matrix4x4 uvTransform;
	//	float shininess;
	//};
	////バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	////バッファリソース内のデータを示すポインタ
	//Material* materialData = nullptr;

	////座標返還行列データ
	//struct TransformationMatrix
	//{
	//	Matrix4x4 WVP;
	//	Matrix4x4 World;
	//};
	////バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;
	////バッファリソース内のデータを示すポインタ
	//TransformationMatrix* transforMatrixData = nullptr;

};