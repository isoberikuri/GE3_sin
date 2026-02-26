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

	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
	//DirectX::ScratchImage mipImages = spriteCommon_->GetDxCommon()->LoadTexture(modelData.material.textureFilePath);

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource = dxCommon->CreateTextureResource(metadata);
	dxCommon->UploadTextureData(textureResource, mipImages);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//２Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	////Sparite用のTransformationMatrix用のリソースを作る。Matrix4x4 １つ分のサイズを用意する
	//transformationMatrixResourceSprite = dxCommon->CreateBufferResource(/*device,*/ sizeof(MyMath::Matrix4x4));
	////書き込むためのアドレスを取得
	//transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transforMatrixData));
	////単位行列を書きこんでおく
	//transforMatrixData->WVP = MyMath::MakeIdentity4x4();
	//transforMatrixData->World = MyMath::MakeIdentity4x4();

	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

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
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	//描画!(DrawCall/ドローコル）６個のインデックスを使用し１つのインスタンスを描画。その他は当面０で良い
	dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}
