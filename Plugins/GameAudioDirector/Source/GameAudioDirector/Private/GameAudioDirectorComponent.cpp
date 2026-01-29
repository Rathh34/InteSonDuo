#include "GameAudioDirectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

UGameAudioDirectorComponent::UGameAudioDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentAmbienceState = EAudioAmbienceState::Idle;
	CurrentAmbienceCustomName = NAME_None;
}

void UGameAudioDirectorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	AmbienceSpatialFlags.Empty();
	
	// Check for enhanced ambience config first
	if (const FAmbienceStateConfig* Config = EnhancedAmbienceCues.Find(CurrentAmbienceState))
	{
		if (Config->Cues.Num() > 0)
		{
			// Spawn all cues simultaneously
			for (const FAmbienceCueConfig& CueConfig : Config->Cues)
			{
				if (CueConfig.Cue)
				{
					UAudioComponent* AC = nullptr;
					
					if (CueConfig.bUseRandomSpatialization)
					{
						// Spawn behind player with left/right variation
						FVector SpawnLocation = GetRandomLocationBehindPlayer();
						
						AC = UGameplayStatics::SpawnSoundAtLocation(
							World,
							CueConfig.Cue,
							SpawnLocation,
							FRotator::ZeroRotator,
							1.0f,
							1.0f,
							0.0f,
							nullptr,
							nullptr,
							false
						);
						
						if (AC)
						{
							AC->bAllowSpatialization = true;
							AC->bIsUISound = false;
							AmbienceSpatialFlags.Add(AC, true);  // Mark as spatial
							
							// Bind delegate to reposition on loop
							AC->OnAudioFinished.AddDynamic(this, &UGameAudioDirectorComponent::OnAmbienceFinishedPlaying);
						}
					}
					else
					{
						// Spawn as 2D
						AC = UGameplayStatics::SpawnSound2D(
							World,
							CueConfig.Cue,
							1.0f,
							1.0f,
							0.0f,
							nullptr,
							true,
							false
						);
						
						if (AC)
						{
							AmbienceSpatialFlags.Add(AC, false);  // Mark as non-spatial
						}
					}
					
					if (AC)
					{
						AmbienceAudioComponents.Add(AC);
					}
				}
			}
		}
	}
	else
	{
		// Fallback to single cue (legacy)
		USoundCue* InitialCue = nullptr;
		
		if (!CurrentAmbienceCustomName.IsNone())
		{
			InitialCue = GetCustomAmbienceCue(CurrentAmbienceCustomName);
		}
		
		if (!InitialCue)
		{
			InitialCue = GetDefaultAmbienceCue(CurrentAmbienceState);
		}
		
		if (InitialCue)
		{
			UAudioComponent* AC = UGameplayStatics::SpawnSound2D(
				World,
				InitialCue,
				1.0f,
				1.0f,
				0.0f,
				nullptr,
				true,
				false
			);
			
			if (AC)
			{
				AmbienceAudioComponents.Add(AC);
				AmbienceSpatialFlags.Add(AC, false);
			}
		}
	}
	
	AmbienceTargetVolume = 1.0f;
	AmbienceFadeSpeed = 1.0f;
}

void UGameAudioDirectorComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAmbience(DeltaTime);
}

void UGameAudioDirectorComponent::OnAmbienceFinishedPlaying()
{
	// Reposition all spatial ambience components
	for (UAudioComponent* AC : AmbienceAudioComponents)
	{
		if (AC && AC->IsPlaying())
		{
			// Check if this component is marked as spatial
			if (bool* bIsSpatial = AmbienceSpatialFlags.Find(AC))
			{
				if (*bIsSpatial)
				{
					// Reposition to new random location behind player
					FVector NewLocation = GetRandomLocationBehindPlayer();
					AC->SetWorldLocation(NewLocation);
				}
			}
		}
	}
}



// ========= AMBIENCE HELPERS =========
USoundCue* UGameAudioDirectorComponent::GetDefaultAmbienceCue(EAudioAmbienceState State) const
{
	if (USoundCue** Found = const_cast<TMap<EAudioAmbienceState, USoundCue*>&>(DefaultAmbienceCues).Find(State))
	{
		return *Found;
	}
	return nullptr;
}

float UGameAudioDirectorComponent::GetDefaultAmbienceVolumeScale(EAudioAmbienceState State) const
{
	if (float* Found = const_cast<TMap<EAudioAmbienceState, float>&>(DefaultAmbienceVolumeScale).Find(State))
	{
		return *Found;
	}
	return 1.0f;
}

USoundCue* UGameAudioDirectorComponent::GetCustomAmbienceCue(FName StateName) const
{
	if (StateName.IsNone())
	{
		return nullptr;
	}
	
	for (const FCustomAmbienceEntry& Entry : CustomAmbienceEntries)
	{
		if (Entry.StateName == StateName)
		{
			return Entry.Cue;
		}
	}
	return nullptr;
}

float UGameAudioDirectorComponent::GetCustomAmbienceVolumeScale(FName StateName) const
{
	if (StateName.IsNone())
	{
		return 1.0f;
	}
	
	for (const FCustomAmbienceEntry& Entry : CustomAmbienceEntries)
	{
		if (Entry.StateName == StateName)
		{
			return Entry.VolumeScale;
		}
	}
	return 1.0f;
}

void UGameAudioDirectorComponent::AddOrUpdateCustomAmbienceEntry(const FCustomAmbienceEntry& Entry)
{
	for (FCustomAmbienceEntry& Existing : CustomAmbienceEntries)
	{
		if (Existing.StateName == Entry.StateName)
		{
			Existing = Entry;
			return;
		}
	}
	CustomAmbienceEntries.Add(Entry);
}

void UGameAudioDirectorComponent::RemoveCustomAmbienceEntry(FName StateName)
{
	CustomAmbienceEntries.RemoveAll([StateName](const FCustomAmbienceEntry& Entry)
	{
		return Entry.StateName == StateName;
	});
}

// ========= AMBIENCE SETTERS =========
void UGameAudioDirectorComponent::SetAmbienceState(EAudioAmbienceState NewState, float FadeTime)
{
	if (NewState == CurrentAmbienceState && CurrentAmbienceCustomName.IsNone() && AmbienceAudioComponents.Num() > 0)
	{
		return;
	}
	
	CurrentAmbienceState = NewState;
	CurrentAmbienceCustomName = NAME_None;
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// Stop all existing ambience audio components
	for (UAudioComponent* AC : AmbienceAudioComponents)
	{
		if (AC)
		{
			AC->OnAudioFinished.RemoveDynamic(this, &UGameAudioDirectorComponent::OnAmbienceFinishedPlaying);
			AC->Stop();
		}
	}
	AmbienceAudioComponents.Empty();
	AmbienceSpatialFlags.Empty();
	
	// Check for enhanced config
	if (const FAmbienceStateConfig* Config = EnhancedAmbienceCues.Find(NewState))
	{
		if (Config->Cues.Num() > 0)
		{
			// Spawn all cues simultaneously
			for (const FAmbienceCueConfig& CueConfig : Config->Cues)
			{
				if (CueConfig.Cue)
				{
					UAudioComponent* AC = nullptr;
					
					if (CueConfig.bUseRandomSpatialization)
					{
						// Spawn behind player with left/right variation
						FVector SpawnLocation = GetRandomLocationBehindPlayer();
						
						AC = UGameplayStatics::SpawnSoundAtLocation(
							World,
							CueConfig.Cue,
							SpawnLocation,
							FRotator::ZeroRotator,
							1.0f,
							1.0f,
							0.0f,
							nullptr,
							nullptr,
							false
						);
						
						if (AC)
						{
							AC->bAllowSpatialization = true;
							AC->bIsUISound = false;
							AmbienceSpatialFlags.Add(AC, true);  // Mark as spatial
							
							// Bind delegate to reposition on loop
							AC->OnAudioFinished.AddDynamic(this, &UGameAudioDirectorComponent::OnAmbienceFinishedPlaying);
						}
					}
					else
					{
						// Spawn as 2D
						AC = UGameplayStatics::SpawnSound2D(
							World,
							CueConfig.Cue,
							1.0f,
							1.0f,
							0.0f,
							nullptr,
							true,
							false
						);
						
						if (AC)
						{
							AmbienceSpatialFlags.Add(AC, false);  // Mark as non-spatial
						}
					}
					
					if (AC)
					{
						AmbienceAudioComponents.Add(AC);
					}
				}
			}
		}
	}
	else
	{
		// Fallback to default single cue
		USoundCue* NewCue = GetDefaultAmbienceCue(NewState);
		
		if (!NewCue)
		{
			AmbienceTargetVolume = 0.0f;
			AmbienceFadeSpeed = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
			return;
		}
		
		UAudioComponent* AC = UGameplayStatics::SpawnSound2D(
			World,
			NewCue,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
		);
		
		if (AC)
		{
			AmbienceAudioComponents.Add(AC);
			AmbienceSpatialFlags.Add(AC, false);
		}
	}
	
	AmbienceTargetVolume = 1.0f;
	AmbienceFadeSpeed = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
}

void UGameAudioDirectorComponent::SetAmbienceStateByName(FName NewStateName, float FadeTime)
{
	if (NewStateName == CurrentAmbienceCustomName && AmbienceAudioComponents.Num() > 0)
	{
		return;
	}
	
	CurrentAmbienceCustomName = NewStateName;
	USoundCue* NewCue = GetCustomAmbienceCue(NewStateName);
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// Stop all existing ambience
	for (UAudioComponent* AC : AmbienceAudioComponents)
	{
		if (AC)
		{
			AC->OnAudioFinished.RemoveDynamic(this, &UGameAudioDirectorComponent::OnAmbienceFinishedPlaying);
			AC->Stop();
		}
	}
	AmbienceAudioComponents.Empty();
	AmbienceSpatialFlags.Empty();
	
	if (!NewCue)
	{
		AmbienceTargetVolume = 0.0f;
		AmbienceFadeSpeed = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
		return;
	}
	
	UAudioComponent* AC = UGameplayStatics::SpawnSound2D(
		World,
		NewCue,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		true,
		false
	);
	
	if (AC)
	{
		AmbienceAudioComponents.Add(AC);
		AmbienceSpatialFlags.Add(AC, false);
	}
	
	AmbienceTargetVolume = 1.0f;
	AmbienceFadeSpeed = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
}

void UGameAudioDirectorComponent::UpdateAmbience(float DeltaTime)
{
	if (AmbienceAudioComponents.Num() == 0)
	{
		return;
	}
	
	float StateScale = 1.0f;
	if (!CurrentAmbienceCustomName.IsNone())
	{
		StateScale = GetCustomAmbienceVolumeScale(CurrentAmbienceCustomName);
	}
	else
	{
		if (const FAmbienceStateConfig* Config = EnhancedAmbienceCues.Find(CurrentAmbienceState))
		{
			StateScale = Config->VolumeScale;
		}
		else
		{
			StateScale = GetDefaultAmbienceVolumeScale(CurrentAmbienceState);
		}
	}
	
	const float Desired = AmbienceTargetVolume * MasterVolume * StateScale;
	
	// Apply volume fade to all ambience components
	for (UAudioComponent* AC : AmbienceAudioComponents)
	{
		if (AC)
		{
			const float Current = AC->VolumeMultiplier;
			if (FMath::IsNearlyEqual(Current, Desired, 0.001f))
			{
				AC->SetVolumeMultiplier(Desired);
			}
			else
			{
				const float NewVol = FMath::FInterpTo(Current, Desired, DeltaTime, AmbienceFadeSpeed);
				AC->SetVolumeMultiplier(NewVol);
			}
		}
	}
}

// ========= VOLUME =========
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

void UGameAudioDirectorComponent::SetPlayerSFXVolume(float NewVolume)
{
	PlayerSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetEnemySFXVolume(float NewVolume)
{
	EnemySFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetEnvironmentSFXVolume(float NewVolume)
{
	EnvironmentSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

float UGameAudioDirectorComponent::GetCategoryVolume(EAudioSFXCategory Category) const
{
	switch (Category)
	{
	case EAudioSFXCategory::Player: return PlayerSFXVolume;
	case EAudioSFXCategory::Enemy: return EnemySFXVolume;
	case EAudioSFXCategory::Environment: return EnvironmentSFXVolume;
	case EAudioSFXCategory::General:
	default: return 1.0f;
	}
}

// ========= SFX / UI =========
void UGameAudioDirectorComponent::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
	if (!Sound) return;
	const float FinalVolume = Volume * MasterVolume * SFXVolume;
	UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundAtLocation(USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (!Sound) return;
	UWorld* World = GetWorld();
	if (!World) return;
	const float FinalVolume = Volume * MasterVolume * SFXVolume;
	UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSound2D(USoundBase* Sound, EAudioSFXCategory Category,
	float Volume, float Pitch)
{
	if (!Sound) return;
	const float CatVol = GetCategoryVolume(Category);
	const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
	UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSoundAtLocation(USoundBase* Sound, EAudioSFXCategory Category,
	FVector Location, float Volume, float Pitch)
{
	if (!Sound) return;
	UWorld* World = GetWorld();
	if (!World) return;
	const float CatVol = GetCategoryVolume(Category);
	const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
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
	const FVector WorldOffset = CamRotation.RotateVector(Offset);
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

void UGameAudioDirectorComponent::PlayJumpscare(USoundBase* Stinger,
	float StingerVolume,
	float StingerPitch,
	float AmbienceDuck,
	float FadeBackTime)
{
	if (!Stinger)
		return;
	
	AmbienceDuck = FMath::Clamp(AmbienceDuck, 0.0f, 1.0f);
	
	for (UAudioComponent* AC : AmbienceAudioComponents)
	{
		if (AC)
		{
			AC->SetVolumeMultiplier(MasterVolume * AmbienceDuck);
		}
	}
	
	AmbienceTargetVolume = 1.0f;
	AmbienceFadeSpeed = (FadeBackTime > 0.01f) ? (1.0f / FadeBackTime) : 1000.0f;
	
	PlaySoundRelativeToPlayer(Stinger, FVector(100.0f, 0.0f, 0.0f),
		StingerVolume, StingerPitch);
}

// ========= RANDOM SPATIALIZATION =========

FVector UGameAudioDirectorComponent::GetRandomLocationBehindPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return FVector::ZeroVector;
	}
	
	FVector PlayerLocation;
	FRotator PlayerRotation;
	
	if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
	{
		PlayerLocation = CamMgr->GetCameraLocation();
		PlayerRotation = CamMgr->GetCameraRotation();
	}
	else if (APawn* Pawn = PC->GetPawn())
	{
		PlayerLocation = Pawn->GetActorLocation();
		PlayerRotation = Pawn->GetActorRotation();
	}
	else
	{
		return FVector::ZeroVector;
	}
	
	// Calculate position behind player with left/right variation
	const float BackwardDistance = FMath::FRandRange(RandomSpatializationConfig.MinRadius, RandomSpatializationConfig.MaxRadius);
	const float LeftRightOffset = FMath::FRandRange(-RandomSpatializationConfig.MaxRadius * 0.5f, RandomSpatializationConfig.MaxRadius * 0.5f);
	const float HeightOffset = FMath::FRandRange(-RandomSpatializationConfig.HeightRange, RandomSpatializationConfig.HeightRange);
	
	// Local space offset (X = forward/back, Y = left/right, Z = up/down)
	// Negative X = behind player
	const FVector LocalOffset(-BackwardDistance, LeftRightOffset, HeightOffset);
	
	// Transform to world space
	const FVector WorldOffset = PlayerRotation.RotateVector(LocalOffset);
	
	return PlayerLocation + WorldOffset;
}

FVector UGameAudioDirectorComponent::GetRandomLocationAroundPlayer() const
{
	return GetRandomLocationAroundPlayerCustom(
		RandomSpatializationConfig.MinRadius,
		RandomSpatializationConfig.MaxRadius,
		RandomSpatializationConfig.HeightRange
	);
}

FVector UGameAudioDirectorComponent::GetRandomLocationAroundPlayerCustom(
	float MinRadius,
	float MaxRadius,
	float HeightRange) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return FVector::ZeroVector;
	}
	
	FVector PlayerLocation;
	if (APawn* Pawn = PC->GetPawn())
	{
		PlayerLocation = Pawn->GetActorLocation();
	}
	else if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
	{
		PlayerLocation = CamMgr->GetCameraLocation();
	}
	else
	{
		return FVector::ZeroVector;
	}
	
	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const float Radius = FMath::FRandRange(MinRadius, MaxRadius);
	const float ZOffset = FMath::FRandRange(-HeightRange, HeightRange);
	
	const float AngleRad = FMath::DegreesToRadians(Angle);
	const FVector Offset(
		FMath::Cos(AngleRad) * Radius,
		FMath::Sin(AngleRad) * Radius,
		ZOffset
	);
	
	return PlayerLocation + Offset;
}

void UGameAudioDirectorComponent::PlaySoundRandomSpatial(USoundBase* Sound, float Volume, float Pitch)
{
	if (!Sound) return;
	const FVector SpawnLocation = GetRandomLocationAroundPlayer();
	PlaySoundAtLocation(Sound, SpawnLocation, Volume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundRandomSpatialCustom(
	USoundBase* Sound,
	float MinRadius,
	float MaxRadius,
	float HeightRange,
	float Volume,
	float Pitch)
{
	if (!Sound) return;
	const FVector SpawnLocation = GetRandomLocationAroundPlayerCustom(MinRadius, MaxRadius, HeightRange);
	PlaySoundAtLocation(Sound, SpawnLocation, Volume, Pitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSoundRandomSpatial(
	USoundBase* Sound,
	EAudioSFXCategory Category,
	float Volume,
	float Pitch)
{
	if (!Sound) return;
	const FVector SpawnLocation = GetRandomLocationAroundPlayer();
	PlayCategorizedSoundAtLocation(Sound, Category, SpawnLocation, Volume, Pitch);
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

void UGameAudioDirectorComponent::PlayFootstepAtLocation(FVector Location,
	EFootstepSurface Surface,
	float Volume,
	float Pitch)
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
	
	const float FinalVolume = Volume * MasterVolume * SFXVolume;
	UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location, FinalVolume, Pitch);
}

EFootstepSurface UGameAudioDirectorComponent::DetectSurfaceAtLocation(FVector Location, float TraceDistance)
{
	UWorld* World = GetWorld();
	if (!World) return EFootstepSurface::Default;
	
	FVector Start = Location;
	FVector End = Location - FVector(0.0f, 0.0f, TraceDistance);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	
	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		if (UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get())
		{
			EPhysicalSurface SurfaceType = PhysMat->SurfaceType;
			switch (SurfaceType)
			{
			case SurfaceType1: return EFootstepSurface::Wood;
			case SurfaceType2: return EFootstepSurface::Stone;
			case SurfaceType3: return EFootstepSurface::Metal;
			case SurfaceType4: return EFootstepSurface::Carpet;
			case SurfaceType5: return EFootstepSurface::Water;
			default: return EFootstepSurface::Default;
			}
		}
	}
	return EFootstepSurface::Default;
}

// ========= RANDOMIZATION HELPER =========
float UGameAudioDirectorComponent::ApplyRandomization(float BaseValue, float Variation) const
{
	if (Variation <= 0.0f)
		return BaseValue;
	float RandomOffset = FMath::FRandRange(-Variation, Variation);
	return FMath::Clamp(BaseValue + RandomOffset, 0.01f, 10.0f);
}

// ========= ENHANCED PLAYBACK WITH RANDOMIZATION =========
void UGameAudioDirectorComponent::PlayFootstepAtLocationEx(
	FVector Location,
	EFootstepSurface Surface,
	float Volume,
	float Pitch,
	bool bRandomizePitch,
	float PitchVariation,
	bool bRandomizeVolume,
	float VolumeVariation)
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
	
	float FinalPitch = bRandomizePitch ? ApplyRandomization(Pitch, PitchVariation) : Pitch;
	float FinalVolume = bRandomizeVolume ? ApplyRandomization(Volume, VolumeVariation) : Volume;
	FinalVolume *= MasterVolume * SFXVolume;
	
	UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location, FinalVolume, FinalPitch);
}

void UGameAudioDirectorComponent::PlaySound2DEx(
	USoundBase* Sound,
	float Volume,
	float Pitch,
	bool bRandomizePitch,
	float PitchVariation,
	bool bRandomizeVolume,
	float VolumeVariation)
{
	if (!Sound) return;
	float FinalPitch = bRandomizePitch ? ApplyRandomization(Pitch, PitchVariation) : Pitch;
	float FinalVolume = bRandomizeVolume ? ApplyRandomization(Volume, VolumeVariation) : Volume;
	FinalVolume *= MasterVolume * SFXVolume;
	UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, FinalPitch);
}

void UGameAudioDirectorComponent::PlaySoundAtLocationEx(
	USoundBase* Sound,
	FVector Location,
	float Volume,
	float Pitch,
	bool bRandomizePitch,
	float PitchVariation,
	bool bRandomizeVolume,
	float VolumeVariation,
	float AttenuationMultiplier)
{
	if (!Sound) return;
	UWorld* World = GetWorld();
	if (!World) return;
	
	float FinalPitch = bRandomizePitch ? ApplyRandomization(Pitch, PitchVariation) : Pitch;
	float FinalVolume = bRandomizeVolume ? ApplyRandomization(Volume, VolumeVariation) : Volume;
	FinalVolume *= MasterVolume * SFXVolume * AttenuationMultiplier;
	
	UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, FinalPitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSound2DEx(
	USoundBase* Sound,
	EAudioSFXCategory Category,
	float Volume,
	float Pitch,
	bool bRandomizePitch,
	float PitchVariation,
	bool bRandomizeVolume,
	float VolumeVariation)
{
	if (!Sound) return;
	float FinalPitch = bRandomizePitch ? ApplyRandomization(Pitch, PitchVariation) : Pitch;
	float FinalVolume = bRandomizeVolume ? ApplyRandomization(Volume, VolumeVariation) : Volume;
	const float CatVol = GetCategoryVolume(Category);
	FinalVolume *= MasterVolume * SFXVolume * CatVol;
	UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, FinalPitch);
}
