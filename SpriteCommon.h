#pragma once
#include "DirectXCommon.h"

//スプライト共通部
class SpriteCommon
{
public:
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	void Initialize(DirectXCommon* dxCommon);

	//ルートシグネチャの作成
	void CreateRootSignature();
	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();
	//共通描画設定
	void SetCommonDrawSettings(ID3D12GraphicsCommandList* commandList);

private:
	DirectXCommon* dxCommon_;

	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	//グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;


};
