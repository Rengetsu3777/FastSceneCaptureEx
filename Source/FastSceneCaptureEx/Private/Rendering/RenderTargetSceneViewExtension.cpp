// Fill out your copyright notice in the Description page of Project Settings.


#include "Rendering/RenderTargetSceneViewExtension.h"

#include "PixelShaderUtils.h"
#include "RenderGraphEvent.h"
#include "SceneTexturesConfig.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ShaderPasses/InvertColourComputePass.h"
#include "ShaderPasses/InvertColourRenderPass.h"

//コンストラクタ
FRenderTargetSceneViewExtension::FRenderTargetSceneViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister)
{
}

//
void FRenderTargetSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	FSceneViewExtensionBase::PrePostProcessPass_RenderThread(GraphBuilder, View, Inputs);



	//レンダーターゲットが適用されているかをチェック。セットされていない場合は、早期に終了。
	if(RenderTargetSource == nullptr)
	{
		return;
	}
	
	//有効なプールレンダーターゲットがあるか確認。無い場合は作成する。
	if(!PooledRenderTarget.IsValid())
	{
		// Only needs to be done once
		// However, if you modify the render target asset, eg: change the resolution or pixel format, you may need to recreate the PooledRenderTarget object
		//CreatePooledRenderTarget_RenderThread(RenderTargetSource, PooledRenderTarget);//これはこのファイルの後ろの方に実装されている。
		CreatePooledRenderTarget_RenderThread();
	}
	


	//レンダーターゲットを作成した後、ビューポートのグローバルシェーダーマップとSceneColorテクスチャを取得します。
	checkSlow(View.bIsViewInfo);
	const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;
	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	const FScreenPassTexture SceneColourTexture((*Inputs.SceneTextures)->SceneColorTexture, Viewport);


	// Needs to be registered every frame
	//レンダリングターゲットリソースにアクセスするには、Pレンダーターゲットを渡して、外部テクスチャを登録し、名前を付ける必要がある。
	FRDGTextureRef RenderTargetTexture = GraphBuilder.RegisterExternalTexture(PooledRenderTarget, TEXT("Bound Render Target"));



	// Since we're rendering to the render target, we're going to use the full size of the render target rather than the screen
	// フルスクリーンレンダリングではなく、レンダリングターゲットにレンダリングするるため、レンダリングターゲットテクスチャの寸法を使用する。これらの値は、アセット自体に設定されている。
	const FIntRect RenderViewport = FIntRect(0, 0, RenderTargetTexture->Desc.Extent.X, RenderTargetTexture->Desc.Extent.Y);
	
	// True for pixel shader, false for compute shader
#if true

	//SceneColor版=======================
	
	//パラメーター構造体を設定し、グローバルシェーダーマップからピクセルシェーダーを取得し、フルスクリーンパスをレンダリンググラフに追加する。
	FInvertColourPS::FParameters* Parameters = GraphBuilder.AllocParameters<FInvertColourPS::FParameters>();
	Parameters->TextureSize = RenderTargetTexture->Desc.Extent;
	Parameters->SceneColorSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->SceneColorTexture = SceneColourTexture.Texture;
	// We're going to also clear the render target
	Parameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::EClear);
	
	TShaderMapRef<FInvertColourPS> PixelShader(GlobalShaderMap);
	FPixelShaderUtils::AddFullscreenPass(GraphBuilder, GlobalShaderMap, FRDGEventName(TEXT("Render Target Pass")), PixelShader, Parameters, RenderViewport);


#else
	FRDGTextureRef TempTexture = GraphBuilder.CreateTexture(RenderTargetTexture->Desc, TEXT("Temp Texture"));
	FRDGTextureUAVDesc TempUAVDesc = FRDGTextureUAVDesc(TempTexture);
	FRDGTextureUAVRef TempUAV = GraphBuilder.CreateUAV(TempUAVDesc);

	const FIntPoint ThreadCount = RenderViewport.Size();
	const FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(ThreadCount, FIntPoint(16, 16));
	
	FInvertColourCS::FParameters* Parameters = GraphBuilder.AllocParameters<FInvertColourCS::FParameters>();
	Parameters->TextureSize = RenderTargetTexture->Desc.Extent;
	Parameters->SceneColorSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->SceneColorTexture = SceneColourTexture.Texture;
	Parameters->OutputTexture = TempUAV;
	
	TShaderMapRef<FInvertColourCS> ComputeShader(GlobalShaderMap);
	FComputeShaderUtils::AddPass(GraphBuilder, FRDGEventName(TEXT("Render Target Compute Pass")), ERDGPassFlags::Compute, ComputeShader, Parameters, GroupCount);
	
	AddCopyTexturePass(GraphBuilder, TempTexture, RenderTargetTexture);
#endif
}

void FRenderTargetSceneViewExtension::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	RenderTargetSource = RenderTarget;
}



//新しいプールレンダリングターゲットを作成する。
//void FRenderTargetSceneViewExtension::CreatePooledRenderTarget_RenderThread(TObjectPtr<UTextureRenderTarget2D> RenderTargetSourceInput, TRefCountPtr<IPooledRenderTarget> PooledRenderTargetInput)
void FRenderTargetSceneViewExtension::CreatePooledRenderTarget_RenderThread()
{
	//SceneColor用========
	//レンダリングスレッド内にあるかどうかを確認する。そうでない場合は、これらの一部でエラーが発生する。
	checkf(IsInRenderingThread() || IsInRHIThread(), TEXT("Cannot create from outside the rendering thread"));
	
	// Render target resources require the render thread
	//リソースの作成
	//const FTextureRenderTargetResource* RenderTargetResource = RenderTargetSourceInput->GetRenderTargetResource();
	const FTextureRenderTargetResource* RenderTargetResource = RenderTargetSource->GetRenderTargetResource();

	if(RenderTargetResource == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Render Target Resource is null"));
	}
	
	//RHI参照を取得。
	const FTexture2DRHIRef RenderTargetRHI = RenderTargetResource->GetRenderTargetTexture();
	if(RenderTargetRHI.GetReference() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Render Target RHI is null"));
	}


	//構造体を作成
	FSceneRenderTargetItem RenderTargetItem;
	RenderTargetItem.TargetableTexture = RenderTargetRHI;
	RenderTargetItem.ShaderResourceTexture = RenderTargetRHI;

	// Flags allow it to be used as a render target, shader resource, UAV 
	//レンダリングターゲットの説明(desc)が必要。テクスチャの作成またはバッファーのdescと同じように機能する。サイズとテクスチャが形式があります。どちらもリソースとRHI、そしてクリアカラーと作成フラグがある。
	FPooledRenderTargetDesc RenderTargetDesc = FPooledRenderTargetDesc::Create2DDesc(RenderTargetResource->GetSizeXY(), RenderTargetRHI->GetDesc().Format, FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV, TexCreate_None, false);
	
	//追跡されていない要素を作成し、descriptorを渡す。（プールされたレンダーターゲットは参照型です。）第二引数は関数の出力。第三引数は先ほど作成したアイテムストロークは、後で自分で行うこともできる。
	//GRenderTargetPool.CreateUntrackedElement(RenderTargetDesc, PooledRenderTargetInput, RenderTargetItem);
	GRenderTargetPool.CreateUntrackedElement(RenderTargetDesc, PooledRenderTarget, RenderTargetItem);

	UE_LOG(LogTemp,Warning, TEXT("Created untracked Pooled Render Target resource"));
}




//====================

//Depth用=============