#pragma once
#include "pch.h"
#if FRAMEWORK_UNREAL // Not sure if this is needed but it's here anyway

#ifndef GAME_FNAMES
#define GAME_FNAMES(x) \
	x(Invalid)
#endif

#define CREATE_ENUM(n) n,
#define CREATE_CLASS_LOOKUP(n) FNames::ClassLookupEntry_t{std::string_view(#n), 0, FNames::EFNames::n},
namespace FNames
{
	enum EFNames {
		GAME_FNAMES(CREATE_ENUM)
	};

	typedef struct ActorInfo_t {
		SDK::AActor* pActor;
		EFNames iLookupIndex;
		float flDistance;
	} PawnInfo_t;

	typedef struct ClassLookupEntry_t {
		std::string_view sName;
		int32_t ComparisonIndex;
		EFNames iLookupIndex;
	} ClassLookupEntry_t;

	inline std::vector<ClassLookupEntry_t> vecClassLookups{
		GAME_FNAMES(CREATE_CLASS_LOOKUP)
	};
	
	inline EFNames GetLookupIndex(int32_t ComparisonIndex) {
		for (const auto entry : vecClassLookups) {
			if (ComparisonIndex == entry.ComparisonIndex)
				return entry.iLookupIndex;
		}

		return EFNames::Invalid;
	}

	inline void Initialize()
	{
		Utils::LogDebug(Utils::GetLocation(CurrentLoc), (std::stringstream() << "GNames: 0x" << SDK::FName::GNames).str());
		Utils::LogDebug(Utils::GetLocation(CurrentLoc), (std::stringstream() << "GNames Count: " << SDK::FName::GNames->Count()).str());
		
		size_t iGNameSize = 0;
		int lastBlock = 0;
		uintptr_t nextFNameAddress = reinterpret_cast<uintptr_t>(SDK::FName::GNames->Allocator.Blocks[0]);

		while (1) {

		RePlay:
			int32_t nextFNameComparisonId = MAKELONG((uint16_t)((nextFNameAddress - reinterpret_cast<uintptr_t>(SDK::FName::GNames->Allocator.Blocks[lastBlock])) / 2), (uint16_t)lastBlock);
			int32_t block = nextFNameComparisonId >> 16;
			int32_t offset = (uint16_t)nextFNameComparisonId;
			int32_t offsetFromBlock = static_cast<int32_t>(nextFNameAddress - reinterpret_cast<uintptr_t>(SDK::FName::GNames->Allocator.Blocks[lastBlock]));

			// Get entry information
			const uintptr_t entryOffset = nextFNameAddress;
			const int toAdd = 0x00 + 0x02; // HeaderOffset + HeaderSize

			const uint16_t* pNameHeader = reinterpret_cast<uint16_t*>(entryOffset);

			if (!IsValidObjectPtr(pNameHeader))
				break;

			const uint16_t nameHeader = *pNameHeader;
			int nameLength = nameHeader >> 6;
			bool isWide = (nameHeader & 1) != 0;
			if (isWide)
				nameLength += nameLength;

			// if odd number (odd numbers are aligned with 0x00)
			if (!isWide && nameLength % 2 != 0)
				nameLength += 1;

			// Block end ?
			if (offsetFromBlock + toAdd + (nameLength * 2) >= 0xFFFF * SDK::FNameEntryAllocator::Stride || nameHeader == 0x00 || block == SDK::FName::GNames->Allocator.CurrentBlock && offset >= SDK::FName::GNames->Allocator.CurrentByteCursor)
			{
				nextFNameAddress = reinterpret_cast<uintptr_t>(SDK::FName::GNames->Allocator.Blocks[++lastBlock]);
				goto RePlay;
			}

			std::string sName = std::string(reinterpret_cast<SDK::FNameEntry*>(entryOffset)->AnsiName, nameHeader >> 6);

			for (int i = 0; i < vecClassLookups.size(); i++) {
				if (vecClassLookups[i].ComparisonIndex)
					continue;

				if (sName != vecClassLookups[i].sName)
					continue;

				vecClassLookups[i].ComparisonIndex = nextFNameComparisonId;
				Utils::LogDebug(Utils::GetLocation(CurrentLoc), "Initalized " + sName);
				break;
			}

			// We hit last Name in last Block
			if (lastBlock > SDK::FName::GNames->Allocator.CurrentBlock)
				break;

			// Get next name address
			nextFNameAddress = entryOffset + toAdd + nameLength;
			++iGNameSize;
		}

		for (ClassLookupEntry_t stLookupEntry : vecClassLookups) {
			if (!stLookupEntry.ComparisonIndex)
				Utils::LogError(Utils::GetLocation(CurrentLoc), "Didnt Find " + std::string(stLookupEntry.sName));
		}

		Utils::LogDebug(Utils::GetLocation(CurrentLoc), (std::stringstream() << "Resolved GNames Count: " << iGNameSize).str());
	};
}
#undef CREATE_ENUM
#undef CREATE_CLASS_LOOKUP

static SDK::UFont* pFont;
static DWORD dwOldProtect;

typedef void(__thiscall* PostRender) (SDK::UObject* pViewportClient, SDK::UCanvas* pCanvas);
static PostRender oPostRender;

class Unreal
{
public:
	static void HookPostRender()
	{
		pFont = SDK::UObject::FindObject<SDK::UFont>("Font Roboto.Roboto");
		if (!IsValidObjectPtr(pFont))
			return;

		SDK::UGameViewportClient* pViewportClient = GetViewportClient();
		if (!IsValidObjectPtr(pViewportClient))
			return;

		void** VFTable = pViewportClient->VfTable;
		VirtualProtect(&VFTable[POST_RENDER_INDEX], 8, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		oPostRender = reinterpret_cast<PostRender>(VFTable[POST_RENDER_INDEX]);
		VFTable[POST_RENDER_INDEX] = &hkPostRender;
		VirtualProtect(&VFTable[POST_RENDER_INDEX], 8, dwOldProtect, &dwOldProtect);
	}

	static void RestorePostRender()
	{
		SDK::UGameViewportClient* pViewportClient = GetViewportClient();
		if (!IsValidObjectPtr(pViewportClient))
			return;

		void** VFTable = pViewportClient->VfTable;
		VirtualProtect(&VFTable[POST_RENDER_INDEX], 8, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		VFTable[POST_RENDER_INDEX] = oPostRender;
		VirtualProtect(&VFTable[POST_RENDER_INDEX], 8, dwOldProtect, &dwOldProtect);
	}

	std::vector<SDK::AActor*> Actors;
	std::vector<FNames::ActorInfo_t> ActorList;
	std::mutex ActorLock;

	// Shortcut functions to get pointers to important classes used for many things
	static SDK::UKismetMathLibrary* GetMathLibrary() { return reinterpret_cast<SDK::UKismetMathLibrary*>(SDK::UKismetMathLibrary::StaticClass()); }
	static SDK::UKismetSystemLibrary* GetSystemLibrary() { return reinterpret_cast<SDK::UKismetSystemLibrary*>(SDK::UKismetSystemLibrary::StaticClass()); }
	static SDK::UGameplayStatics* GetGameplayStatics() { return reinterpret_cast<SDK::UGameplayStatics*>(SDK::UGameplayStatics::StaticClass()); }
	static SDK::UKismetStringLibrary* GetStringLibrary() { return reinterpret_cast<SDK::UKismetStringLibrary*>(SDK::UKismetStringLibrary::StaticClass()); }

	// LIFEHAAACK BITCH (◣_◢)
	inline bool IsAFast(SDK::UClass* in, FNames::EFNames iLookupIndex)
	{
		int32_t iComparisonIndex = FNames::vecClassLookups[iLookupIndex].ComparisonIndex;
		for (SDK::UStruct* pStruct = static_cast<SDK::UStruct*>(in); IsValidObjectPtr(pStruct); pStruct = pStruct->SuperField) {
			if (pStruct->Name.ComparisonIndex == iComparisonIndex)
				return true;
		}

		return false;
	};

	// Kinda like UObject->IsA, but safer and faster as we are already storing fname comparison indexes!!!
	inline bool IsAFast(SDK::UClass* in, int iComparisonIndex)
	{
		for (SDK::UStruct* pStruct = static_cast<SDK::UStruct*>(in); IsValidObjectPtr(pStruct); pStruct = pStruct->SuperField) {
			if (pStruct->Name.ComparisonIndex == iComparisonIndex)
				return true;
		}

		return false;
	};

	inline void RefreshActorList() // A function to refresh the actor list
	{
		std::vector<SDK::AActor*> lActors{};
		std::vector<FNames::ActorInfo_t> lActorList{};
		std::vector<float> AllDistances{};

		static std::unordered_map<uint32_t, FNames::EFNames> umClassLookupCache{};

		if (SDK::UWorld::GWorld == nullptr) {
			ActorLock.lock();
			umClassLookupCache.clear(); // This is probably not needed due to fname comparison indexes not changing
			Actors.clear();
			ActorList.clear();
			ActorLock.unlock();
			return;
		}

		SDK::AFSDGameState* pGameState = reinterpret_cast<SDK::AFSDGameState*>(GetGameStateBase());
		if (!IsValidObjectPtr(pGameState) || pGameState->IsOnSpaceRig) {
			ActorLock.lock();
			umClassLookupCache.clear(); // This is probably not needed due to fname comparison indexes not changing
			Actors.clear();
			ActorList.clear();
			ActorLock.unlock();
			return;
		}

		FNames::EFNames iMatchState = FNames::GetLookupIndex(pGameState->MatchState.ComparisonIndex);
		if (iMatchState != FNames::InProgress) {
			ActorLock.lock();
			umClassLookupCache.clear(); // This is probably not needed due to fname comparison indexes not changing
			Actors.clear();
			ActorList.clear();
			ActorLock.unlock();
			return;
		}
		
		SDK::APawn* pAcknowledgedPawn = GetAcknowledgedPawn();
		if (!pAcknowledgedPawn) {
			ActorLock.lock();
			Actors.clear();
			ActorList.clear();
			ActorLock.unlock();
			return;
		}


		SDK::FVector vecLocation = pAcknowledgedPawn->K2_GetActorLocation();

		for (int i = 0; i < SDK::UWorld::GWorld->Levels.Count(); i++)
		{
			SDK::ULevel* Level = SDK::UWorld::GWorld->Levels[i];

			if (!Level)
				continue;

			if (!Level->NearActors.Data() || !Level->NearActors.Count())
				continue;

			for (int j = 0; j < Level->NearActors.Count(); j++)
			{
				SDK::AActor* Actor = Level->NearActors[j];
				if (!Actor)
					continue;

				bool bFailed = false;
				for (SDK::AActor* pOtherActor : lActors) {
					if (pOtherActor != Actor)
						continue;

					bFailed = true;
					break;
				}

				if (!bFailed)
					lActors.push_back(Actor);
			}
		}

		for (SDK::AActor* pActor : lActors)
		{
			if (!IsValidObjectPtr(pActor))
				continue;

			int32_t iComparisonIndex = pActor->Name.ComparisonIndex;
			FNames::ActorInfo_t stActorInfo{
				pActor,
				FNames::Invalid,
				vecLocation.Distance(pActor->K2_GetActorLocation())
			};


			if (auto itr = umClassLookupCache.find(iComparisonIndex); itr != umClassLookupCache.end()) {
				stActorInfo.iLookupIndex = itr->second;
			}
			else {
				for (SDK::UStruct* pStruct = static_cast<SDK::UStruct*>(pActor->Class); IsValidObjectPtr(pStruct); pStruct = pStruct->SuperField) {
					for (FNames::ClassLookupEntry_t stEntry : FNames::vecClassLookups) {
						if (pStruct->Name.ComparisonIndex == stEntry.ComparisonIndex) {
							stActorInfo.iLookupIndex = stEntry.iLookupIndex;
							break;
						}
					}

					if (stActorInfo.iLookupIndex != FNames::Invalid)
						break;
				}

				umClassLookupCache.emplace(iComparisonIndex, stActorInfo.iLookupIndex);
			}

			//if (stActorInfo.iLookupIndex == FNames::Invalid)
			//	continue;

			while (1) {
				bool bFixed = true;
				for (float flOtherDistance : AllDistances)
				{
					if (flOtherDistance == stActorInfo.flDistance) {
						stActorInfo.flDistance = nextafterf(stActorInfo.flDistance, INFINITY);
						bFixed = false;
						break;
					}
				}

				if (bFixed)
					break;
			}

			lActorList.push_back(stActorInfo);
			AllDistances.push_back(stActorInfo.flDistance);
		}

		std::stable_sort(lActorList.begin(), lActorList.end(), [](FNames::ActorInfo_t A, FNames::ActorInfo_t B) {
			return A.flDistance > B.flDistance;
		});


		ActorLock.lock();

		Actors.clear();
		for (SDK::AActor* v : lActors)
			Actors.push_back(v);

		ActorList.clear();
		for (auto v : lActorList)
			ActorList.push_back(v);

		ActorLock.unlock();
	}

	template <typename T>
	inline std::vector<T*> GetActors() // A function to get all the actors of a certain type can be useful for things like ESP where you only want to draw for certain actor types
	{
		std::vector<T*> actors;

		SDK::AGameStateBase* pGameState = GetGameStateBase();
		SDK::UClass* pComparisonClass = T::StaticClass();
		if (!IsValidObjectPtr(pComparisonClass) || !pGameState || pGameState->ReplicatedWorldTimeSeconds <= 5.f)
			return actors;

		for (SDK::AActor* pActor : Actors)
		{
			if (!IsValidObjectPtr(reinterpret_cast<T*>(pActor)))
				continue;

			if (pActor->IsA(pComparisonClass)) // Check if the actor is of the type we want
				actors.push_back(reinterpret_cast<T*>(pActor)); // If it is add it to the vector
		}

		return actors; // Return the vector
	}

	// These functions are to make getting pointers to important classes and objects easier and cleaner
	static SDK::AGameStateBase* GetGameStateBase()
	{
		if (!SDK::UWorld::GWorld)
			return nullptr;

		SDK::AGameStateBase* pGameState = SDK::UWorld::GWorld->GameState;
		if (!IsValidObjectPtr(pGameState))
			return nullptr;

		return pGameState;
	}
	static SDK::UGameInstance* GetGameInstance()
	{
		if (!SDK::UWorld::GWorld)
			return nullptr;

		SDK::UGameInstance* pGameInstance = SDK::UWorld::GWorld->OwningGameInstance;
		if (!IsValidObjectPtr(pGameInstance))
			return nullptr;

		return pGameInstance;
	}

	static SDK::ULocalPlayer* GetLocalPlayer(int index = 0)
	{
		SDK::UGameInstance* pGameInstance = GetGameInstance();
		if (!IsValidObjectPtr(pGameInstance))
			return nullptr;

		SDK::ULocalPlayer* pLocalPlayer = pGameInstance->LocalPlayers[index];
		if (!IsValidObjectPtr(pLocalPlayer))
			return nullptr;

		return pLocalPlayer;
	}
	static SDK::UGameViewportClient* GetViewportClient()
	{
		SDK::ULocalPlayer* pLocalPlayer = GetLocalPlayer();
		if (!IsValidObjectPtr(pLocalPlayer))
			return nullptr;

		SDK::UGameViewportClient* pViewportClient = pLocalPlayer->ViewportClient;
		if (!IsValidObjectPtr(pViewportClient))
			return nullptr;

		return pViewportClient;
	}
	static SDK::APlayerController* GetPlayerController()
	{
		SDK::ULocalPlayer* LocalPlayer = GetLocalPlayer();
		if (!IsValidObjectPtr(LocalPlayer))
			return nullptr;

		SDK::APlayerController* pPlayerController = LocalPlayer->PlayerController;
		if (!IsValidObjectPtr(pPlayerController))
			return nullptr;

		return pPlayerController;
	}
	static SDK::APawn* GetAcknowledgedPawn()
	{
		SDK::APlayerController* pPlayerController = GetPlayerController();
		if (!IsValidObjectPtr(pPlayerController))
			return nullptr;

		SDK::APawn* pAcknowledgedPawn = pPlayerController->AcknowledgedPawn;
		if (!IsValidObjectPtr(pAcknowledgedPawn))
			return nullptr;

		return pAcknowledgedPawn;
	}

	static SDK::APlayerCameraManager* GetPlayerCameraManager()
	{
		SDK::APlayerController* pPlayerController = GetPlayerController();
		if (!IsValidObjectPtr(pPlayerController))
			return nullptr;

		SDK::APlayerCameraManager* pPlayerCameraManager = pPlayerController->PlayerCameraManager;
		if (!IsValidObjectPtr(pPlayerCameraManager))
			return nullptr;

		return pPlayerCameraManager;
	}

	static bool WorldToScreen(SDK::FVector in, SDK::FVector2D& out, bool relative = false)
	{
		SDK::APlayerController* pPlayerController = GetPlayerController();
		if (!IsValidObjectPtr(pPlayerController))
			return false;

		return pPlayerController->ProjectWorldLocationToScreen(in, &out, relative);
	}

	// I made this function so I would have to type less to get the screen position of a world position
	static SDK::FVector2D W2S(SDK::FVector in, bool relative = false)
	{
		SDK::FVector2D out = { 0, 0 };
		SDK::APlayerController* PlayerController = GetPlayerController();
		if (!IsValidObjectPtr(PlayerController))
			return out;

		PlayerController->ProjectWorldLocationToScreen(in, &out, relative);

		return out;
	}

	template <typename T>
	static std::vector<T> SortActorsByDistance(std::vector<T> actors)
	{
		SDK::APawn* pAcknowledgedPawn = GetAcknowledgedPawn();
		if (!IsValidObjectPtr(pAcknowledgedPawn))
			return actors;

		std::vector<T> SortedActors{};

		typedef struct SortHack_t {
			T pActor;
			float flDistance;
		} SortHack_t;

		std::vector<SortHack_t> ActorAndDistances{};
		std::vector<float> AllDistances{};

		for (T pActor : actors) 
		{
			if (!IsValidObjectPtr(pActor))
				continue;

			SortHack_t stActorHack{
				pActor,
				pActor->GetDistanceTo(pAcknowledgedPawn)
			};

			while (1) {
				bool bFixed = true;
				for (float flOtherDistance : AllDistances)
				{
					if (flOtherDistance == stActorHack.flDistance) {
						stActorHack.flDistance = nextafterf(stActorHack.flDistance, INFINITY);
						bFixed = false;
						break;
					}
				}

				if (bFixed)
					break;
			}

			ActorAndDistances.push_back(stActorHack);
			AllDistances.push_back(stActorHack.flDistance);
		}

		std::stable_sort(ActorAndDistances.begin(), ActorAndDistances.end(), [](SortHack_t A, SortHack_t B) {
			return A.flDistance > B.flDistance;
		});

		for (SortHack_t stHack : ActorAndDistances)
			SortedActors.push_back(stHack.pActor);

		return SortedActors;
	}

	static void hkPostRender(SDK::UObject* pViewportClient, SDK::UCanvas* pCanvas)
	{
		SDK::ULocalPlayer* pLocalPlayer = GetLocalPlayer();
		if (!IsValidObjectPtr(pLocalPlayer))
			return oPostRender(pViewportClient, pCanvas);

		SDK::FLinearColor Cyan = { 0.f, 1.f, 1.f, 1.f };
		SDK::FLinearColor Black = { 0.f, 0.f, 0.f, 1.f };

		// The canvas reports its size as the current resolution but in my testing it is always 2048, 1280
		SDK::FVector2D TextSize = pCanvas->K2_TextSize(pFont, L"OmegaWare.xyz", { 1.f, 1.f });
		//pCanvas->K2_DrawText(pFont, L"OmegaWare.xyz", { 1024.f - (TextSize.X / 2), 0.f }, { 1.f, 1.f }, Cyan, 1.f, Black, { 0.f, 0.f }, false, false, true, Black);

		//pCanvas->K2_DrawLine({ 0.f, 0.f }, { 1024.f, 640.f }, 1.f, Cyan);

		oPostRender(pViewportClient, pCanvas);
	}
};

#endif