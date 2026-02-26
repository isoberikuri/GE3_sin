#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include "TextureManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	// 引数で受け取ってメンバ変数にする
	this->spriteCommon_ = spriteCommon;
	DirectXCommon* dxCommon = spriteCommon_->GetDxCommon();

	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	//頂点バッファビューを作成する
	vertexBufferViewSprite.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.SizeInBytes = UINT(sizeof(VertexData) * 6);//使用するリソースのサイズは頂点のサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);//１頂点あたりのサイズ

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得

	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	vertexData[0].texcoord = { 0.0f,1.0f };

	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexData[1].texcoord = { 0.0f,0.0f };

	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexData[2].texcoord = { 1.0f,1.0f };

	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexData[3].texcoord = { 1.0f,0.0f };

	indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	materialData->color = MyMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);


	transformationMatrixResourceSprite = dxCommon->CreateBufferResource(/*device,*/ sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transforMatrixData));
	// 単位行列を書き込んでおく
	//*wvpData = MakeIdentity4x4();
	transforMatrixData->WVP = MyMath::MakeIdentity4x4();
	transforMatrixData->World = MyMath::MakeIdentity4x4();

	transform = {
		  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	MyMath::Transform cameraTransform{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };

}

void Sprite::Update()
{
	//Sprite用のWorldViewProjectionMatrixを作る
	MyMath::Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	MyMath::Matrix4x4 viewMatrix = MyMath::MakeIdentity4x4();
	MyMath::Matrix4x4 projectionMatrix = MyMath::MakeOrthorgraphicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	//Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transforMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transforMatrixData->World = worldMatrix;

	// メンバ変数の値を見た目に反映する処理
	transform.translate = { position.x, position.y, 0.0f };
	transform.rotate = { 0.0f, 0.0f, rotation };

	// 頂点リソースにデータを書きこむ
	// 左下
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	transform.scale = { size.x, size.y, 1.0f };

}

void Sprite::Draw()
{
	DirectXCommon* dxCommon = spriteCommon_->GetDxCommon();
	spriteCommon_->SetCommonDrawSettings(dxCommon->GetCommandList());

	//Spriteの描画。変更が必要なものだけ変更する
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//VBVを設定
	dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//IBNを設定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
	D3D12_GPU_DESCRIPTOR_HANDLE textureHanlde = TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex);
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureHanlde);
	//描画!(DrawCall/ドローコル）６個のインデックスを使用し１つのインスタンスを描画。その他は当面０で良い
	dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}
