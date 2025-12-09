#include "pch.h"

Misc::Misc() {};

bool Misc::Setup()
{
	if (!Cheat::localization->AddToLocale("ENG", "FULLBRIGHT", "Fullbright"))
		return false;

	if (!Cheat::localization->AddToLocale("GER", "FULLBRIGHT", "Vollhelligkeit"))
		return false;

	if (!Cheat::localization->AddToLocale("POL", "FULLBRIGHT", "Pełna Jasność"))
		return false;

	Cheat::localization->UpdateLocale();

	Utils::LogDebug(Utils::GetLocation(CurrentLoc), "Feature: Fullbright Initialized");

	Initialized = true;
	return Initialized;
}

void Misc::Destroy() {
	Initialized = false;

	Unreal* pUnreal = Cheat::unreal.get();

	SDK::UGameViewportClient* pViewportClient = pUnreal->GetViewportClient();
	if (!pViewportClient)
		return;

	pViewportClient->ViewMode = 3;
}

void Misc::HandleKeys() {}

void Misc::PopulateMenu()
{
	if (!Initialized)
		return;

	Child* Misc = new Child("Fullbright", []() { return ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 2); }, ImGuiChildFlags_Border);
	Misc->AddElement(new Checkbox(Cheat::localization->Get("FULLBRIGHT"), &bFullbright));
	Misc->AddElement(new Button("Get Ore Veins", []() {
		std::vector<SDK::ADeepCSGWorld*> TerrainObjects = Cheat::unreal->GetActors<SDK::ADeepCSGWorld>();
		for (SDK::ADeepCSGWorld* pTerrain : TerrainObjects)
		{
			if (!IsValidObjectPtr(pTerrain))
				continue;

			SDK::UTerrainMaterialsCollection* pTerrainMaterials = pTerrain->TerrainMaterials;
			if (!IsValidObjectPtr(pTerrainMaterials))
				continue;

			for (int i = 0; i < pTerrainMaterials->Materials.Count(); i++)
			{
				SDK::UTerrainMaterial* pTerrainMaterial = pTerrainMaterials->Materials[i];
				if (!IsValidObjectPtr(pTerrainMaterial))
					continue;

				SDK::UResourceData* pResourceData = pTerrainMaterial->ResourceData;
				if (!IsValidObjectPtr(pResourceData))
					continue;

				
				wchar_t* wsName = pResourceData->Title.Get();
				if (!IsValidObjectPtr(wsName))
					continue;

				std::wstring ws(wsName);
				std::string str(ws.begin(), ws.end());

				Utils::LogDebug(Utils::GetLocation(CurrentLoc), str);
			}
		}
	}));

	Cheat::menu->AddElement(Misc, true);
}

void Misc::Render() {}

void Misc::Run() {
	if (!Initialized)
		return;

	Unreal* pUnreal = Cheat::unreal.get();

	SDK::UGameViewportClient* pViewportClient = pUnreal->GetViewportClient();
	if (!pViewportClient)
		return;

	pViewportClient->ViewMode = bFullbright ? 2 : 3;
}

void Misc::SaveConfig() { Cheat::config->PushEntry("FULLBRIGHT", "bool", std::to_string(bFullbright)); }

void Misc::LoadConfig()
{
	ConfigEntry entry = Cheat::config->GetEntryByName("FULLBRIGHT");
	if (entry.Name == "FULLBRIGHT")
		bFullbright = std::stoi(entry.Value);
}