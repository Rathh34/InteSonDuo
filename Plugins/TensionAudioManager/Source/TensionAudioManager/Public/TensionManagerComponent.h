#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "TensionManagerComponent.generated.h"

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TENSIONAUDIOMANAGER_API UTensionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTensionManagerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION(BlueprintCallable, Category="Tension")
	void SetTensionSmooth(float NewTension, float TimeSeconds = 2.0f);

	UFUNCTION(BlueprintPure, Category="Tension")
	float GetTension() const { return CurrentTension; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tension", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CurrentTension = 0.0f;

	float TargetTension = 0.0f;

	// Seconds to go from current to target (via FInterpTo)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tension")
	float DefaultTransitionTime = 2.0f;

	float InterpSpeed = 1.0f;

	// Horror ambience layers
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio|Ambience")
	USoundCue* CalmCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio|Ambience")
	USoundCue* UneasyCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio|Ambience")
	USoundCue* PanicCue = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* CalmAC = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* UneasyAC = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* PanicAC = nullptr;

	void UpdateTension(float DeltaTime);
	void UpdateAmbienceVolumes();
};
