// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderTargetSubsystem.h"

#include "EngineUtils.h"
#include "SceneViewExtension.h"
#include "Engine/TextureRenderTarget2D.h"

void URenderTargetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CustomSceneViewExtension = FSceneViewExtensions::NewExtension<FRenderTargetSceneViewExtension>();
	//CustomDepthViewExtension = FSceneViewExtensions::NewExtension<FRenderTargetDepthViewExtension>();//Depthが使えないため、一旦消す

	//if(UTextureRenderTarget2D* RenderTarget = LoadObject<UTextureRenderTarget2D>(nullptr, TEXT("/Script/Engine.TextureRenderTarget2D'/FastSceneCaptureEx/RT_Target.RT_Target'"))) {
	//レンダーターゲット自体を手動でロードし、シーンビュー拡張機能自体のSetterを使用して設定する。
	if(UTextureRenderTarget2D* RenderTarget = LoadObject<UTextureRenderTarget2D>(nullptr, TEXT("/Script/Engine.TextureRenderTarget2D'/FastSceneCaptureEx/RT_SceneCaptureTarget.RT_SceneCaptureTarget'"))) {
		CustomSceneViewExtension->SetRenderTarget(RenderTarget);
	}

	//追加 2025/09/97//Depthが使えないため、一旦消す
	/*
	if (UTextureRenderTarget2D* RenderTargetDepth = LoadObject<UTextureRenderTarget2D>(nullptr, TEXT("/Script/Engine.TextureRenderTarget2D'/FastSceneCaptureEx/RT_DepthCaptureTarget.RT_DepthCaptureTarget'"))) {
		CustomDepthViewExtension->SetRenderTargetDepth(RenderTargetDepth);
	}
	*/
}

void URenderTargetSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	CustomSceneViewExtension.Reset();
	CustomSceneViewExtension = nullptr;
}
