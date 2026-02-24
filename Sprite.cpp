#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	// 引数で受け取ってメンバ変数にする
	this->spriteCommon_ = spriteCommon;

	DirectXCommon* dxCommon = spriteCommon_->GetDxCommon();

	vertexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 6);
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	//vertexData[0].texcoord = { 0.0f,1.0f };
	//vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	//vertexData[1].texcoord = { 0.0f,0.0f };
	//vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	//vertexData[2].texcoord = { 1.0f,1.0f };
	//vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	//vertexData[3].texcoord = { 1.0f,0.0f };

	indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	//indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	//indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	//indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;


	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);//１頂点あたりのサイズ

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//マテリアル用のリソースを作る。今回はcolor１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	Vector4* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// wvp用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transfomationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	//Matrix4x4* wvpData = nullptr;
	TransformationMatrix* transfomrationMatrixData = nullptr;
	// 書き込むためのアドレスを取得
	transfomationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transfomrationMatrixData));
	// 単位行列を書き込んでおく
	//*wvpData = MakeIdentity4x4();
	transfomrationMatrixData->WVP = MakeIdentity4x4();
	transfomrationMatrixData->World = MakeIdentity4x4();

	Transform transform{
	  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform cameraTransform{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };

	//Textureを読んで転送する
	//DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
	DirectX::ScratchImage mipImages = spriteCommon_->GetDxCommon()->LoadTexture(modelData.material.textureFilePath);

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResouce = dxCommon->CreateTextureResource(metadata);
	dxCommon->UploadTextureData(textureResouce, mipImages);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//２Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResouce.Get(), &srvDesc, textureSrvHandleCPU);



	////Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点６つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//１頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	////１枚目の三角形

	//vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	//vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	//vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	//vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	////２枚目の三角形
	//vertexDataSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };//右上
	//vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	//vertexDataSprite[4].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	//vertexDataSprite[4].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	//vertexDataSprite[5].texcoord = { 1.0f,1.0f };

	//vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	//vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	//vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	//vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	//vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	//vertexDataSprite[3].texcoord = { 1.0f,0.0f };

	//Sparite用のTransformationMatrix用のリソースを作る。Matrix4x4 １つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(/*device,*/ sizeof(Matrix4x4));
	//データを書き込む
	Matrix4x4* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書きこんでおく
	*transformationMatrixDataSprite = MakeIdentity4x4();

	//CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f,1.0f,1.0f},{ 0.0f,0.0f,0.0f },{0.0f,0.0f,0.0f} };

	//頂点インデックス
	Microsoft::WRL::ComPtr<ID3D12Resource>  indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};

	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	/*indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;*/




}

void Sprite::Update()
{


}

void Sprite::Draw()
{


}
