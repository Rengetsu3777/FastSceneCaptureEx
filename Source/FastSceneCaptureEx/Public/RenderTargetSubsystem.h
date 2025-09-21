// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Rendering/RenderTargetSceneViewExtension.h"
#include "Rendering/RenderTargetDepthViewExtension.h"//追加
#include "RenderTargetSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FASTSCENECAPTUREEX_API URenderTargetSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

private:
	TSharedPtr< class FRenderTargetSceneViewExtension, ESPMode::ThreadSafe > CustomSceneViewExtension;
	//TSharedPtr< class FRenderTargetDepthViewExtension, ESPMode::ThreadSafe > CustomDepthViewExtension;//追加

	// The source render target texture asset
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetSource = nullptr;

	/*//Depthが使えないため、一旦消す
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTargetDepthSource = nullptr;
	*/

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
