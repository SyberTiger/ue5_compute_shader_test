#include "TestRunner.h"


// Sets default values
ATestRunner::ATestRunner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestRunner::BeginPlay()
{
	Super::BeginPlay();
	noise = WhiteNoise();
	Initialized = false;

}

// Called every frame
void ATestRunner::Tick(float DeltaTime)
{
	TranscurredTime += DeltaTime;
	Super::Tick(DeltaTime);

	if (TranscurredTime > 1.0f)
	{
		if (!Initialized)
		{
			noise.BeginRendering();
			Initialized = true;
		}

		FWhiteNoiseCSParameters parameters(RenderTarget);
		TimeStamp++;
		parameters.TimeStamp = TimeStamp;
		noise.UpdateParameters(parameters);
	}


}
void ATestRunner::BeginDestroy()
{
	Super::BeginDestroy();
	noise.EndRendering();


}