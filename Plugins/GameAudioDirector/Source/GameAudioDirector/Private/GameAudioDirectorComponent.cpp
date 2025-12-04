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
}

void UGameAudioDirectorComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTension(DeltaTime);
    UpdateAmbienceVolumes();
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

    if (CalmAC)   CalmAC->SetVolumeMultiplier(CalmVol * MasterVolume);
    if (UneasyAC) UneasyAC->SetVolumeMultiplier(UneasyVol * MasterVolume);
    if (PanicAC)  PanicAC->SetVolumeMultiplier(PanicVol * MasterVolume);
}

// ========= VOLUME CONTROL =========

void UGameAudioDirectorComponent::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetSFXVolume(float NewVolume)
{
    SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetUIVolume(float NewVolume)
{
    UIVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

// ========= GENERAL SOUND HELPERS =========

void UGameAudioDirectorComponent::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;
    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundAtLocation(USoundBase* Sound, FVector Location,
                                                      float Volume, float Pitch)
{
    if (!Sound) return;
    UWorld* World = GetWorld();
    if (!World) return;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
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
    const FVector WorldOffset  = CamRotation.RotateVector(Offset);
    const FVector SpawnLocation = CamLocation + WorldOffset;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Sound, SpawnLocation, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayUISound(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;
    const float FinalVolume = Volume * MasterVolume * UIVolume;
    UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
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

    if (CalmAC)   CalmAC->SetVolumeMultiplier(CalmVol * MasterVolume);
    if (UneasyAC) UneasyAC->SetVolumeMultiplier(UneasyVol * MasterVolume);
    if (PanicAC)  PanicAC->SetVolumeMultiplier(PanicVol * MasterVolume);

    PlaySoundRelativeToPlayer(Stinger, FVector(100.0f, 0.0f, 0.0f), 1.0f, 1.0f);
}

// ========= LOOPING SFX =========

void UGameAudioDirectorComponent::StartLoopingSFX2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;

    StopLoopingSFX2D();

    const float FinalVolume = Volume * MasterVolume * SFXVolume;

    LoopingSFX2D = UGameplayStatics::SpawnSound2D(
        this,
        Sound,
        FinalVolume,
        Pitch,
        0.0f,
        nullptr,
        true,
        false
    );
}

void UGameAudioDirectorComponent::StopLoopingSFX2D()
{
    if (LoopingSFX2D)
    {
        LoopingSFX2D->Stop();
        LoopingSFX2D = nullptr;
    }
}

void UGameAudioDirectorComponent::StartLoopingSFXAtLocation(USoundBase* Sound, FVector Location,
                                                            float Volume, float Pitch)
{
    if (!Sound) return;

    StopLoopingSFX3D();

    UWorld* World = GetWorld();
    if (!World) return;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;

    LoopingSFX3D = UGameplayStatics::SpawnSoundAtLocation(
        World,
        Sound,
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        Pitch,
        0.0f,
        nullptr,
        nullptr,
        true
    );
}

void UGameAudioDirectorComponent::StopLoopingSFX3D()
{
    if (LoopingSFX3D)
    {
        LoopingSFX3D->Stop();
        LoopingSFX3D = nullptr;
    }
}

// ========= FOOTSTEPS =========

USoundBase* UGameAudioDirectorComponent::ChooseRandomFromArray(const TArray<USoundBase*>& Array) const
{
    if (Array.Num() == 0)
        return nullptr;

    const int32 Index = FMath::RandRange(0, Array.Num() - 1);
    return Array[Index];
}

void UGameAudioDirectorComponent::PlayFootstepAtLocation(FVector Location, EFootstepSurface Surface)
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const FFootstepSoundList* ListPtr = Footsteps_BySurface.Find(Surface);
    if (!ListPtr)
    {
        ListPtr = Footsteps_BySurface.Find(EFootstepSurface::Default);
    }
    if (!ListPtr)
        return;

    USoundBase* Chosen = ChooseRandomFromArray(ListPtr->Sounds);
    if (!Chosen)
        return;

    const float FinalVolume = MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location, FinalVolume);
}
