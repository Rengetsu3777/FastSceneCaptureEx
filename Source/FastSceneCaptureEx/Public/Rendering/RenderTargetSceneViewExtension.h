// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "PostProcess/PostProcessing.h"
#include "UObject/Object.h"

/**
 * Turtorial: https://www.youtube.com/watch?v=aGycp6t0OEQ&t=232s
 */

//シーンビュー拡張
class FASTSCENECAPTUREEX_API FRenderTargetSceneViewExtension : public FSceneViewExtensionBase
{
private:
	TObjectPtr<UTextureRenderTarget2D> RenderTargetSource = nullptr;//テクスチャレンダーターゲット2Dポインター。これはサブシステムで以前に設定されている。

	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;//プールされたレンダリングターゲット参照カウントのポインター。これはレンダリンググラフ側の処理と全体的な関数を実行するために使用します。そこから先は全てほぼ同じです。

public:
	FRenderTargetSceneViewExtension(const FAutoRegister& AutoRegister);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
	
	virtual void PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override {};
	virtual void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {};
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override {};
	virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override {};
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	virtual void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {};

	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);//レンダーターゲットを設定するセッター関数

private:
	//void CreatePooledRenderTarget_RenderThread(TObjectPtr<UTextureRenderTarget2D> RenderTargetSourceInput, TRefCountPtr<IPooledRenderTarget> PooledRenderTargetInput);//Pレンダーターゲットを作成する関数
	void CreatePooledRenderTarget_RenderThread();//Pレンダーターゲットを作成する関数

	//↑の関数は、レンダリングスレッドの実で実行できるため、同じ命名規則にしたがい、最後に「_RunThread（_RenderThread？）」を追加した。
};


