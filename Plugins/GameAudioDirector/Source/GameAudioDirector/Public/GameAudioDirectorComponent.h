#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "GameAudioDirectorComponent.generated.h"

UENUM(BlueprintType)
enum class EFootstepSurface : uint8
{
    Default UMETA(DisplayName="Default"),
    Wood    UMETA(DisplayName="Wood"),
    Stone   UMETA(DisplayName="Stone"),
    Metal   UMETA(DisplayName="Metal"),
    Carpet  UMETA(DisplayName="Carpet")
};

USTRUCT(BlueprintType)
struct FFootstepSoundList
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<USoundBase*> Sounds;
};

/**
 * Simple game audio director component: tension, ambience, SFX, footsteps, jumpscares,
 * and basic controllable looping SFX.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent), Blueprintable)
class GAMEAUDIODIRECTOR_API UGameAudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGameAudioDirectorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    // ========= TENSION CONTROL =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Tension")
    void SetTensionSmooth(float NewTension, float TimeSeconds = 2.0f);

    UFUNCTION(BlueprintPure, Category="Audio Director|Tension")
    float GetTension() const { return CurrentTension; }

    // ========= VOLUME CONTROL =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume")
    void SetSFXVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume")
    void SetUIVolume(float NewVolume);

    // ========= GENERAL SOUND HELPERS =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D")
    void PlaySound2D(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D")
    void PlaySoundAtLocation(USoundBase* Sound, FVector Location,
                             float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play Relative To Player")
    void PlaySoundRelativeToPlayer(USoundBase* Sound, FVector Offset,
                                   float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play UI")
    void PlayUISound(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Jumpscare")
    void PlayJumpscare(USoundBase* Stinger, float AmbienceDuck = 0.2f);

    // ========= LOOPING SFX (start/stop) =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping")
    void StartLoopingSFX2D(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping")
    void StopLoopingSFX2D();

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping")
    void StartLoopingSFXAtLocation(USoundBase* Sound, FVector Location,
                                   float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping")
    void StopLoopingSFX3D();

    // ========= FOOTSTEPS =========

    /** Play a footstep at a world location, using the given surface. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Footsteps")
    void PlayFootstepAtLocation(FVector Location, EFootstepSurface Surface);

protected:
    // Tension
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Tension",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float CurrentTension = 0.0f;

    float TargetTension = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Tension",
              meta=(ClampMin="0.01"))
    float DefaultTransitionTime = 2.0f;

    float InterpSpeed = 1.0f;

    // Volume
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float UIVolume = 1.0f;

    // Ambience
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience")
    USoundCue* CalmCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience")
    USoundCue* UneasyCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience")
    USoundCue* PanicCue = nullptr;

    UPROPERTY(Transient)
    UAudioComponent* CalmAC = nullptr;

    UPROPERTY(Transient)
    UAudioComponent* UneasyAC = nullptr;

    UPROPERTY(Transient)
    UAudioComponent* PanicAC = nullptr;

    // Looping SFX
    UPROPERTY(Transient)
    UAudioComponent* LoopingSFX2D = nullptr;

    UPROPERTY(Transient)
    UAudioComponent* LoopingSFX3D = nullptr;

    // Footsteps: one map per surface
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps")
    TMap<EFootstepSurface, FFootstepSoundList> Footsteps_BySurface;

    // Internal helpers
    void UpdateTension(float DeltaTime);
    void UpdateAmbienceVolumes();

    USoundBase* ChooseRandomFromArray(const TArray<USoundBase*>& Array) const;
};
