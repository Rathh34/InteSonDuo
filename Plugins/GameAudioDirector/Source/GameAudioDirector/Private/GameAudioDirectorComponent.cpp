#include "GameAudioDirectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UGameAudioDirectorComponent::UGameAudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentTension = 0.0f;
    TargetTension = 0.0f;
    DefaultTransitionTime = 2.0f;
    InterpSpeed = (DefaultTransitionTime > 0.01f)
        ? (1.0f / DefaultTransitionTime)
        : 1000.0f;

    TimeSinceLastStinger = 0.0f;
    NextStingerDelay = 0.0f;
}

void UGameAudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();

    auto SpawnLoop = [this](USoundCue* Cue) -> UAudioComponent*
    {
        if (!Cue) return nullptr;

        return UGameplayStatics::SpawnSound2D(
            this,
            Cue,
            0.0f,
            1.0f,
            0.0f,
            nullptr,
            true,
            false
        );
    };

    CalmAC   = SpawnLoop(CalmCue);
    UneasyAC = SpawnLoop(UneasyCue);
    PanicAC  = SpawnLoop(PanicCue);

    TimeSinceLastStinger = 0.0f;
    NextStingerDelay = MinStingerInterval_LowTension;
}

void UGameAudioDirectorComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTension(DeltaTime);
    UpdateAmbienceVolumes();
    UpdateRandomStingers(DeltaTime);
}

// ========= TENSION =========

void UGameAudioDirectorComponent::SetTensionSmooth(float NewTension, float TimeSeconds)
{
    TargetTension = FMath::Clamp(NewTension, 0.0f, 1.0f);

    const float Time = (TimeSeconds > 0.0f) ? TimeSeconds : DefaultTransitionTime;
    InterpSpeed = (Time > 0.01f) ? (1.0f / Time) : 1000.0f;
}

void UGameAudioDirectorComponent::UpdateTension(float DeltaTime)
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

void UGameAudioDirectorComponent::UpdateAmbienceVolumes()
{
    const float T = FMath::Clamp(CurrentTension, 0.0f, 1.0f);

    float CalmVol   = 0.0f;
    float UneasyVol = 0.0f;
    float PanicVol  = 0.0f;

    if (T <= 0.4f)
    {
        const float Alpha = (T / 0.4f);
        CalmVol   = 1.0f - Alpha;
        UneasyVol = Alpha;
        PanicVol  = 0.0f;
    }
    else
    {
        const float Alpha = (T - 0.4f) / 0.6f;
        CalmVol   = 0.0f;
        UneasyVol = 1.0f - Alpha;
        PanicVol  = Alpha;
    }

    if (CalmAC)   CalmAC->SetVolumeMultiplier(CalmVol);
    if (UneasyAC) UneasyAC->SetVolumeMultiplier(UneasyVol);
    if (PanicAC)  PanicAC->SetVolumeMultiplier(PanicVol);
}

// ========= GENERAL SOUND HELPERS =========

void UGameAudioDirectorComponent::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;

    UGameplayStatics::PlaySound2D(this, Sound, Volume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundAtLocation(USoundBase* Sound, FVector Location,
                                                      float Volume, float Pitch)
{
    if (!Sound) return;

    UWorld* World = GetWorld();
    if (!World) return;

    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Volume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundRelativeToPlayer(USoundBase* Sound, FVector Offset,
                                                            float Volume, float Pitch)
{
    if (!Sound) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
    if (!CamMgr) return;

    const FVector CamLocation = CamMgr->GetCameraLocation();
    const FRotator CamRotation = CamMgr->GetCameraRotation();

    const FVector WorldOffset = CamRotation.RotateVector(Offset);
    const FVector SpawnLocation = CamLocation + WorldOffset;

    UGameplayStatics::PlaySoundAtLocation(World, Sound, SpawnLocation, Volume, Pitch);
}

void UGameAudioDirectorComponent::PlayJumpscare(USoundBase* Stinger, float AmbienceDuck)
{
    if (!Stinger)
        return;

    AmbienceDuck = FMath::Clamp(AmbienceDuck, 0.0f, 1.0f);

    const float T = FMath::Clamp(CurrentTension, 0.0f, 1.0f);

    float CalmVol   = 0.0f;
    float UneasyVol = 0.0f;
    float PanicVol  = 0.0f;

    if (T <= 0.4f)
    {
        const float Alpha = (T / 0.4f);
        CalmVol   = 1.0f - Alpha;
        UneasyVol = Alpha;
        PanicVol  = 0.0f;
    }
    else
    {
        const float Alpha = (T - 0.4f) / 0.6f;
        CalmVol   = 0.0f;
        UneasyVol = 1.0f - Alpha;
        PanicVol  = Alpha;
    }

    CalmVol   *= AmbienceDuck;
    UneasyVol *= AmbienceDuck;
    PanicVol  *= AmbienceDuck;

    if (CalmAC)   CalmAC->SetVolumeMultiplier(CalmVol);
    if (UneasyAC) UneasyAC->SetVolumeMultiplier(UneasyVol);
    if (PanicAC)  PanicAC->SetVolumeMultiplier(PanicVol);

    PlaySoundRelativeToPlayer(Stinger, FVector(100.0f, 0.0f, 0.0f), 1.0f, 1.0f);
}

// ========= FOOTSTEPS =========

USoundBase* UGameAudioDirectorComponent::ChooseRandomFromArray(const TArray<USoundBase*>& Array) const
{
    if (Array.Num() == 0)
        return nullptr;

    const int32 Index = FMath::RandRange(0, Array.Num() - 1);
    return Array[Index];
}

void UGameAudioDirectorComponent::PlayFootstepAtLocation(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const bool bPanicMood = (CurrentTension >= 0.6f);

    USoundBase* Chosen = nullptr;

    if (bPanicMood)
        Chosen = ChooseRandomFromArray(Footsteps_Panic);
    else
        Chosen = ChooseRandomFromArray(Footsteps_Normal);

    if (!Chosen)
        return;

    UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location);
}

// ========= RANDOM STINGERS =========

USoundBase* UGameAudioDirectorComponent::ChooseRandomStinger() const
{
    const float T = FMath::Clamp(CurrentTension, 0.0f, 1.0f);
    const bool bHighTension = (T >= 0.6f);

    if (bHighTension && Stingers_HighTension.Num() > 0)
        return ChooseRandomFromArray(Stingers_HighTension);

    if (Stingers_LowTension.Num() > 0)
        return ChooseRandomFromArray(Stingers_LowTension);

    return nullptr;
}

void UGameAudioDirectorComponent::UpdateRandomStingers(float DeltaTime)
{
    if (!bEnableRandomStingers || StingerPlayChance <= 0.0f)
        return;

    TimeSinceLastStinger += DeltaTime;

    if (TimeSinceLastStinger < NextStingerDelay)
        return;

    if (FMath::FRand() <= StingerPlayChance)
    {
        USoundBase* Stinger = ChooseRandomStinger();
        if (Stinger)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                APlayerController* PC = World->GetFirstPlayerController();
                if (PC)
                {
                    APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
                    if (CamMgr)
                    {
                        const FVector CamLocation = CamMgr->GetCameraLocation();

                        const float Distance = FMath::FRandRange(200.0f, 500.0f);
                        const float AngleDeg = FMath::FRandRange(0.0f, 360.0f);
                        const float Height   = FMath::FRandRange(-50.0f, 100.0f);

                        const float Rad = FMath::DegreesToRadians(AngleDeg);
                        const FVector Dir(FMath::Cos(Rad), FMath::Sin(Rad), 0.0f);

                        const FVector SpawnLocation = CamLocation + Dir * Distance + FVector(0.0f, 0.0f, Height);

                        UGameplayStatics::PlaySoundAtLocation(World, Stinger, SpawnLocation);
                    }
                }
            }
        }
    }

    const float T = FMath::Clamp(CurrentTension, 0.0f, 1.0f);
    const float BaseInterval = FMath::Lerp(MinStingerInterval_LowTension,
                                           MinStingerInterval_HighTension,
                                           T);
    const float RandomFactor = FMath::FRandRange(0.7f, 1.3f);
    NextStingerDelay = BaseInterval * RandomFactor;
    TimeSinceLastStinger = 0.0f;
}
