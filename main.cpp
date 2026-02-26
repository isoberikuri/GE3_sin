#include <Windows.h>
#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <format>
#include <string>
#include<fstream>
#include<sstream>
#include <dinput.h>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include"StringUtility.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "MyMath.h"
#include "TextureManager.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

struct VertexData
{
	MyMath::Vector4 position;
	MyMath::Vector2 texcoord;
};

struct MaterialData
{
	std::string textureFilePath;
};


struct ModelData
{
	std::vector<VertexData>vertices;
	MaterialData material;

};

struct Material
{
	MyMath::Vector4 color;
	int32_t enableLighting;
	float padding[3];
	MyMath::Matrix4x4 uvTransform;
	float shininess;
};

struct TransformationMatrix
{
	MyMath::Matrix4x4 WVP;
	MyMath::Matrix4x4 World;
};

//MaterialData読み込み関数
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	//１．中で必要となる変数の宣言
	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ１行を格納するもの

	//２．ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	//３．実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}
	//４．MaterialDataを返す
	return materialData;
}


//OBj読み込み関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	//１、中で必要となる変数の宣言
	ModelData modelData;//構築するModelData
	std::vector<MyMath::Vector4>positions;//位置
	std::vector<MyMath::Vector3>normals;//法線
	std::vector<MyMath::Vector2>texcoords;//テクスチャ座標
	std::string line;//ファイルから読んで１行を格納するもの

	//２，ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	//３，実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "v") {
			MyMath::Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			MyMath::Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			MyMath::Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してindexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を所得して、頂点を構築する
				MyMath::Vector4 position = positions[elementIndices[0] - 1];
				position.x *= -1.0f;
				MyMath::Vector2 texcoord = texcoords[elementIndices[1] - 1];
				texcoord.y = 1.0f - texcoord.y;

				MyMath::Vector3 normal = normals[elementIndices[2] - 1];
				//normal.x *= -1.0f;
				//VertexData vertex = { position,texcoord};
				//VertexData vertex = { position,texcoord,normal };
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord };
				//triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib")
		{
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、デイレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);

		}

	}

	//４，ModelDataを返す
	return modelData;
}

void Log(const std::string& message)
{
	OutputDebugStringA(message.c_str());
}

//ウィンメイン
//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{


	//ポインタ
	WinApp* winApp = nullptr;
	Input* input = nullptr;
	DirectXCommon* dxCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;

	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//入力の初期化
	input = new Input();
	input->Initialize(winApp);

	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	// テキスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	std::vector<std::string>textures =
	{
		"resources/uvChecker.png",
		"resources/monsterball.png"
	};
	for (const std::string& texPath : textures)
	{
		TextureManager::GetInstance()->LoadTexture(texPath);
	}

	Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", L"abc")));

	//モデル読み込み
	//ModelData modelData = LoadObjFile("resources", "plane.obj");
	ModelData modelData = LoadObjFile("resources", "axis.obj");

	////頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource =
		dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	
	////頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();//リソースの先頭のアドレスから使う
	//vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());//使用するリソースのサイズは頂点のサイズ
	//vertexBufferView.StrideInBytes = sizeof(VertexData);//１頂点あたりのサイズ

	////頂点リソースにデータを書き込む
	////VertexData* vertexData = nullptr;
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));//書き込むためのアドレスを取得
	//std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	
	//マテリアル用のリソースを作る。今回はcolor１つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	MyMath::Vector4* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	*materialData = MyMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// wvp用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transfomationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	//Matrix4x4* wvpData = nullptr;
	TransformationMatrix* transfomrationMatrixData = nullptr;
	// 書き込むためのアドレスを取得
	transfomationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transfomrationMatrixData));
	// 単位行列を書き込んでおく
	//*wvpData = MakeIdentity4x4();
	transfomrationMatrixData->WVP = MyMath::MakeIdentity4x4();
	transfomrationMatrixData->World = MyMath::MakeIdentity4x4();

	MyMath::Transform transform{
	  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	MyMath::Transform cameraTransform{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };

	//Textureを読んで転送する
	//DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
	//DirectX::ScratchImage mipImages = dxCommon->LoadTexture(modelData.material.textureFilePath);

	//const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResouce = dxCommon->CreateTextureResource(metadata);
	////dxCommon->UploadTextureData(textureResouce, mipImages);

	////metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//２Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	////SRVを作成するDescriptorHeapの場所を決める
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);
	////先頭はImGuiが使っているのでその次を使う
	///*textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);*/
	////SRVの生成
	//dxCommon->GetDevice()->CreateShaderResourceView(textureResouce.Get(), &srvDesc, textureSrvHandleCPU);


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
	
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左下
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };

	//Sparite用のTransformationMatrix用のリソースを作る。Matrix4x4 １つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(/*device,*/ sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書きこんでおく
	transformationMatrixDataSprite->World = MyMath::MakeIdentity4x4();
	transformationMatrixDataSprite->WVP = MyMath::MakeIdentity4x4();
	////CPUで動かす用のTransformを作る
	MyMath::Transform transformSprite{ {1.0f,1.0f,1.0f},{ 0.0f,0.0f,0.0f },{0.0f,0.0f,0.0f} };

	////頂点インデックス
	//Microsoft::WRL::ComPtr<ID3D12Resource>  indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//
	////リソースの先頭のアドレスから使う
	//indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	////使用するリソースのサイズはインデックス６つ分のサイズ
	//indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	////インデックスはuint32_tとする
	//indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	////インデックスリソースにデータを書き込む
	//uint32_t* indexDataSprite = nullptr;
	//indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	/*indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;*/

	BYTE key[256]{};
	BYTE prekey[256]{};

	LPDIRECTINPUT8 directInput = nullptr;
	LPDIRECTINPUTDEVICE8 keyboard = nullptr;

	/*std::vector<std::string> texFiles =
	{
		"resources/uvChecker.png",
		"resources/monsterBall.png",
		"resources/uvChecker.png",
		"resources/monsterBall.png",
		"resources/uvChecker.png"
	};*/

	//スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	//スプライトの初期化
	/*Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, texFiles[]);*/

	std::vector<Sprite*> sprites;
	for (uint32_t i = 0; i < 5; ++i)
	{
		Sprite* sprite = new Sprite();
		std::string& textureFile = textures[i % 2];
		sprite->Initialize(spriteCommon, textureFile);
		float x_position = 100.0f + 150.0f * i;
		sprite->SetPosition({ x_position, 100.0f });
		sprites.push_back(sprite);
	}

	//------------------------------------------------------------------------------------------------------------------------------

	//ウィンドウの×ボタンが押されるまでループ
	while (true)
	{
		//入力の更新
		input->Update();
		
		for (Sprite* sprite : sprites)
		{
			sprite->Update();

			// 現在の座標を変数で受ける
			MyMath::Vector2 position = sprite->GetPosition();
			// 座標を変更する
			//position += MyMath::Vector2{ 0.1f,0.1f };
			position.x += 0.1f;
			position.y += 0.1f;

			// 変更を反映する
			sprite->SetPosition(position);

			//// 角度で変化させるテスト
			//float rotation = sprite->GetRotation();
			//rotation += 5.0f;
			//sprite->SetRotation(rotation);

			//// 色を変化させるテスト
			//MyMath::Vector4 color = sprite->GetColor();
			//color.x += 0.01f;
			//if (color.x > 1.0f)
			//{
			//	color.x -= 1.0f;
			//}
			//sprite->SetColor(color);

			// サイズを変化させるテスト
			MyMath::Vector2 size = sprite->GetSize();
			size.x += 0.01f;
			size.y += 0.01f;
			sprite->SetSize(size);
		}


		////windowsのメッセージ処理
		if (winApp->ProcessMessage())
		{
			break;
		}
			
			//数字の0キーが押されていたら
			if (input->PushKey(DIK_0))
			{
				OutputDebugStringA("Hit 0\n");
			}

			//ゲームの処理

			//Sprite用のWorldViewProjectionMatrixを作る
			MyMath::Matrix4x4 worldMatrixSprite = MyMath::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			MyMath::Matrix4x4 viewMatrixSprite = MyMath::MakeIdentity4x4();
			MyMath::Matrix4x4 projectionMatrixSprite = MyMath::MakeOrthorgraphicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
			MyMath::Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			//*transformationMatrixDataSprite = worldViewProjectionMatrixSprite;

			//フレームが始まる旨を告げる
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();




			MyMath::Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			MyMath::Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			MyMath::Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			MyMath::Matrix4x4 projectionMatrix = MyMath::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
			MyMath::Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			transfomrationMatrixData->WVP = worldViewProjectionMatrix;
			transfomrationMatrixData->World = worldMatrix;

			//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			//ImGui::ShowDemoWindow();

			ImGui::Begin("Settings");
			ImGui::ColorEdit4("material", &materialData->x, ImGuiColorEditFlags_AlphaPreview);//RGBWの指定
			ImGui::DragFloat3("rotate", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("scale", &transform.scale.x, 0.1f);
			ImGui::DragFloat3("translate", &transform.translate.x, 0.1f);
			ImGui::Separator();
			ImGui::End();


			//transform.rotate.y += 0.03f;

			//ImGuiの内部コマンドを生成する
			ImGui::Render();
			
			//描画前処理
			dxCommon->PreDraw();

			for (Sprite* sprite : sprites)
			{
				sprite->Draw();
			}



			//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transfomationMatrixResource->GetGPUVirtualAddress());
			////SRVのDescriptorTableの先頭を設定。２はrootParameter[2]である。
			//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			////モデル描画
			//dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

			//--------------------------------------

			//Spriteの描画。変更が必要なものだけ変更する
			//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//VBVを設定
			//dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//IBNを設定
			//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

			//描画!(DrawCall/ドローコル）６個のインデックスを使用し１つのインスタンスを描画。その他は当面０で良い
			//dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

			//実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());


			//描画後処理
		dxCommon->PostDraw();


	}

	//CloseHandle(fenceEvent);



	//ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	//こういうもんである。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	

	//解放処理

	//入力解放
	delete input;

	//WindowsAPIの終了処理
	winApp->Finalize();

	// テキスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();

	//WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	//DirectX解放
	delete dxCommon;
	delete spriteCommon;
	for (Sprite* sprite : sprites)
	{
		if (sprite)
		{
			delete sprite;
		}
	}

	return 0;
}