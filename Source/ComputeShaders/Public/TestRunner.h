#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WhiteNoise.h"
#include "TestRunner.generated.h"


UCLASS()
class ATestRunner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestRunner();
	WhiteNoise noise;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		class UTextureRenderTarget2D* RenderTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	uint32 TimeStamp;
	float TranscurredTime;
	bool Initialized;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
};