#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "GameAudioDirectorComponent.generated.h"

/**
 * Simple game audio director component.
 *
 * Attach this to your Player (recommended) or a central AudioManager actor.
 * From any Blueprint that knows that actor:
 *
 * - Call SetTensionSmooth to drive ambience intensity (0 = safe, 1 = panic).
 * - Use PlaySound2D / PlaySoundAtLocation / PlaySoundRelativeToPlayer for SFX.
 * - Call PlayJumpscare for stingers with ambience ducking.
 * - Call PlayFootstepAtLocation from movement/anim notifies.
 *
 * Random stingers (creaks, whispers) are handled automatically based on tension.
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

    /** Smoothly move tension towards NewTension over TimeSeconds. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Tension")
    void SetTensionSmooth(
        UPARAM(DisplayName="New Tension (0-1)") float NewTension,
        UPARAM(DisplayName="Transition Time (seconds)") float TimeSeconds = 2.0f
    );

    /** Current normalized tension value (0 = calm, 1 = maximum). */
    UFUNCTION(BlueprintPure, Category="Audio Director|Tension")
    float GetTension() const { return CurrentTension; }

    // ========= GENERAL SOUND HELPERS =========

    /** Play a non-spatial (2D) sound (UI, inventory, menus, global stingers). */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D")
    void PlaySound2D(
        UPARAM(DisplayName="Sound") USoundBase* Sound,
        UPARAM(DisplayName="Volume Multiplier") float Volume = 1.0f,
        UPARAM(DisplayName="Pitch Multiplier") float Pitch = 1.0f
    );

    /** Play a 3D sound at a world location (doors, objects, distant sounds). */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D")
    void PlaySoundAtLocation(
        UPARAM(DisplayName="Sound") USoundBase* Sound,
        UPARAM(DisplayName="World Location") FVector Location,
        UPARAM(DisplayName="Volume Multiplier") float Volume = 1.0f,
        UPARAM(DisplayName="Pitch Multiplier") float Pitch = 1.0f
    );

    /**
     * Play a 3D sound relative to the player's camera.
     * Offset: local camera space (X=forward, Y=right, Z=up).
     */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Play Relative To Player")
    void PlaySoundRelativeToPlayer(
        UPARAM(DisplayName="Sound") USoundBase* Sound,
        UPARAM(DisplayName="Offset From Camera (Local)") FVector Offset,
        UPARAM(DisplayName="Volume Multiplier") float Volume = 1.0f,
        UPARAM(DisplayName="Pitch Multiplier") float Pitch = 1.0f
    );

    /** Play a jumpscare stinger near the player's camera and duck ambience. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Jumpscare")
    void PlayJumpscare(
        UPARAM(DisplayName="Stinger Sound") USoundBase* Stinger,
        UPARAM(DisplayName="Ambience Duck Factor (0-1)") float AmbienceDuck = 0.2f
    );

    // ========= FOOTSTEPS =========

    /** Play a footstep at a world location; chooses “normal” or “panic” based on tension. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Footsteps")
    void PlayFootstepAtLocation(
        UPARAM(DisplayName="Footstep Location") FVector Location
    );

protected:
    // ========= TENSION STATE =========

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Tension",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float CurrentTension = 0.0f;

    float TargetTension = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Tension",
              meta=(ClampMin="0.01"))
    float DefaultTransitionTime = 2.0f;

    float InterpSpeed = 1.0f;

    // ========= AMBIENCE LAYERS =========

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

    // ========= FOOTSTEPS =========

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps")
    TArray<USoundBase*> Footsteps_Normal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps")
    TArray<USoundBase*> Footsteps_Panic;

    // ========= RANDOM STINGERS =========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio Director|Stingers")
    bool bEnableRandomStingers = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Stingers")
    TArray<USoundBase*> Stingers_LowTension;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Stingers")
    TArray<USoundBase*> Stingers_HighTension;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Stingers",
              meta=(ClampMin="0.1"))
    float MinStingerInterval_LowTension = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Stingers",
              meta=(ClampMin="0.1"))
    float MinStingerInterval_HighTension = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Stingers",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float StingerPlayChance = 0.7f;

    float TimeSinceLastStinger = 0.0f;
    float NextStingerDelay = 0.0f;

    // ========= INTERNAL HELPERS =========

    void UpdateTension(float DeltaTime);
    void UpdateAmbienceVolumes();
    void UpdateRandomStingers(float DeltaTime);

    USoundBase* ChooseRandomFromArray(const TArray<USoundBase*>& Array) const;
    USoundBase* ChooseRandomStinger() const;
};
