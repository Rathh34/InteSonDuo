#include "TensionManagerComponent.h"
#include "Kismet/GameplayStatics.h"

UTensionManagerComponent::UTensionManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentTension = 0.0f;
    TargetTension = 0.0f;
    DefaultTransitionTime = 2.0f;
    InterpSpeed = (DefaultTransitionTime > 0.01f) ? (1.0f / DefaultTransitionTime) : 1000.0f;
}

void UTensionManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    auto SpawnLoop = [this](USoundCue* Cue) -> UAudioComponent*
    {
        if (!Cue) return nullptr;

        return UGameplayStatics::SpawnSound2D(
            this,
            Cue,
            0.0f,   // start silent
            1.0f,
            0.0f,
            nullptr,
            true,   // persist
            false
        );
    };

    CalmAC   = SpawnLoop(CalmCue);
    UneasyAC = SpawnLoop(UneasyCue);
    PanicAC  = SpawnLoop(PanicCue);
}

void UTensionManagerComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTension(DeltaTime);
    UpdateAmbienceVolumes();
}

void UTensionManagerComponent::SetTensionSmooth(float NewTension, float TimeSeconds)
{
    TargetTension = FMath::Clamp(NewTension, 0.0f, 1.0f);

    float Time = (TimeSeconds > 0.0f) ? TimeSeconds : DefaultTransitionTime;
    InterpSpeed = (Time > 0.01f) ? (1.0f / Time) : 1000.0f;
}

void UTensionManagerComponent::UpdateTension(float DeltaTime)
{
    if (FMath::IsNearlyEqual(CurrentTension, TargetTension, 0.001f))
    {
        CurrentTension = TargetTension;
        return;
    }

    CurrentTension = FMath::FInterpTo(
        CurrentTension,
        TargetTension,
        DeltaTime,
        InterpSpeed
    );
}

void UTensionManagerComponent::UpdateAmbienceVolumes()
{
    const float T = FMath::Clamp(CurrentTension, 0.0f, 1.0f);

    float CalmVol   = 0.0f;
    float UneasyVol = 0.0f;
    float PanicVol  = 0.0f;

    // 0.0–0.4: crossfade calm → uneasy
    // 0.4–1.0: crossfade uneasy → panic
    if (T <= 0.4f)
    {
        const float Alpha = (T / 0.4f); // 0 → 1
        CalmVol   = 1.0f - Alpha;
        UneasyVol = Alpha;
        PanicVol  = 0.0f;
    }
    else
    {
        const float Alpha = (T - 0.4f) / 0.6f; // 0 → 1 over 0.4..1.0
        CalmVol   = 0.0f;
        UneasyVol = 1.0f - Alpha;
        PanicVol  = Alpha;
    }

    if (CalmAC)   CalmAC->SetVolumeMultiplier(CalmVol);
    if (UneasyAC) UneasyAC->SetVolumeMultiplier(UneasyVol);
    if (PanicAC)  PanicAC->SetVolumeMultiplier(PanicVol);
}
