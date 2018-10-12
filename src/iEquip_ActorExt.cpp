#include "iEquip_ActorExt.h"

#include "GameBSExtraData.h"  // BaseExtraList
#include "GameData.h"  // EquipManager
#include "GameExtraData.h"  // ExtraContainerChanges, InventoryEntryData, ExtraPoison
#include "GameForms.h"  // TESForm, BGSEquipSlot
#include "GameObjects.h"  // AlchemyItem, TESObjectWEAP
#include "GameReferences.h"  // Actor
#include "IDebugLog.h"  // gLog
#include "ITypes.h"  // SInt32
#include "PapyrusNativeFunctions.h"  // StaticFunctionTag, NativeFunction
#include "PapyrusVM.h"  // VMClassRegistry
#include "Utilities.h"  // CALL_MEMBER_FN

#include <bitset>  // bitset

#include "iEquip_ActorExtLib.h"  // IActorEquipItem


namespace iEquip_ActorExt
{
	bool IsWeaponBound(StaticFunctionTag* a_base, TESObjectWEAP* a_weap)
	{
		if (!a_weap) {
			_ERROR("ERROR: In IsWeaponBound() : Invalid weapon!");
			return false;
		}

		std::bitset<16> bits(a_weap->gameData.flags1);
		return (bits.test(13));
	}


	void EquipPoisonedItemEx(StaticFunctionTag* a_base, Actor* a_actor, TESForm* a_item, SInt32 a_slotID, AlchemyItem* a_poison, bool a_preventUnequip, bool a_equipSound)
	{
		ActorEquipPoisonedItem equipPoison(a_poison);
		EquipItemEx(a_actor, a_item, a_slotID, &equipPoison, a_preventUnequip, a_equipSound);
	}


	void EquipEnchantedItemEx(StaticFunctionTag* a_base, Actor* a_actor, TESForm* a_item, SInt32 a_slotID, EnchantmentItem* a_enchantment, bool a_preventUnequip, bool a_equipSound)
	{
		ActorEquipEnchantedItem equipEnch(a_enchantment);
		EquipItemEx(a_actor, a_item, a_slotID, &equipEnch, a_preventUnequip, a_equipSound);
	}


	void EquipPoisonedAndEnchantedItemEx(StaticFunctionTag* a_base, Actor* a_actor, TESForm* a_item, SInt32 a_slotID, AlchemyItem* a_poison, EnchantmentItem* a_enchantment, bool a_preventUnequip, bool a_equipSound)
	{
		ActorEquipPoisonedAndEnchantedItem equipPoisonAndEnch(a_poison, a_enchantment);
		EquipItemEx(a_actor, a_item, a_slotID, &equipPoisonAndEnch, a_preventUnequip, a_equipSound);
	}


	void EquipItemEx(Actor* a_actor, TESForm* a_item, SInt32 a_slotID, IActorEquipItem* a_iActorEquipItem, bool a_preventUnequip, bool a_equipSound)
	{
		if (!a_actor) {
			_ERROR("ERROR: In EquipItemEx() : Invalid actor!");
			return;
		} else if (!a_item || !a_item->Has3D()) {
			_ERROR("ERROR: In EquipItemEx() : Invalid item!");
			return;
		} else if (!a_iActorEquipItem->validate()) {
			_ERROR("ERROR: In EquipItemEx() : Failed validation!");
			return;
		}

		EquipManager* equipManager = EquipManager::GetSingleton();
		if (!equipManager) {
			_ERROR("ERROR: In EquipItemEx() : EquipManager not found!");
			return;
		}

		ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(a_actor->extraData.GetByType(kExtraData_ContainerChanges));
		ExtraContainerChanges::Data* containerData = containerChanges ? containerChanges->data : 0;
		if (!containerData) {
			_ERROR("ERROR: In EquipItemEx() : No container data!");
			return;
		}

		// Copy/merge of extraData can fail in edge cases. Obtain it ourselves.
		InventoryEntryData* entryData = findEntryData(containerData, a_item);
		if (!entryData) {
			_ERROR("ERROR: In EquipItemEx() : No entry data!");
			return;
		}

		BGSEquipSlot* targetEquipSlot = getEquipSlotByID(a_slotID);

		SInt32 itemCount = entryData->countDelta;

		// For ammo, use count, otherwise always equip 1
		SInt32 equipCount = a_item->IsAmmo() ? itemCount : 1;

		bool isTargetSlotInUse = false;

		bool hasItemMinCount = itemCount > 0;

		BaseExtraList* rightEquipList = 0;
		BaseExtraList* leftEquipList = 0;

		BaseExtraList* curEquipList = 0;
		BaseExtraList* extraList = 0;

		if (hasItemMinCount) {
			entryData->GetExtraWornBaseLists(&rightEquipList, &leftEquipList);

			// Case 1: Type already equipped in both hands.
			if (leftEquipList && rightEquipList) {
				isTargetSlotInUse = true;
				curEquipList = (targetEquipSlot == GetLeftHandSlot()) ? leftEquipList : rightEquipList;
				extraList = 0;
			}
			// Case 2: Type already equipped in right hand.
			else if (rightEquipList) {
				isTargetSlotInUse = targetEquipSlot == GetRightHandSlot();
				curEquipList = rightEquipList;
				extraList = 0;
			}
			// Case 3: Type already equipped in left hand.
			else if (leftEquipList) {
				isTargetSlotInUse = targetEquipSlot == GetLeftHandSlot();
				curEquipList = leftEquipList;
				extraList = 0;
			}
			// Case 4: Type not equipped yet.
			else {
				isTargetSlotInUse = false;
				curEquipList = 0;
			}

			extraList = a_iActorEquipItem->findExtraListByForm(entryData);
			if (!extraList) {
				_ERROR("ERROR: In EquipItemEx() : No extra list!");
				return;
			}
		}

		// Normally EquipManager would update CannotWear, if equip is skipped we do it here
		if (isTargetSlotInUse) {
			BSExtraData* xCannotWear = curEquipList->GetByType(kExtraData_CannotWear);
			if (xCannotWear && !a_preventUnequip) {
				curEquipList->Remove(kExtraData_CannotWear, xCannotWear);
			} else if (!xCannotWear && a_preventUnequip) {
				curEquipList->Add(kExtraData_CannotWear, ExtraCannotWear::Create());
			}

			// Slot in use, nothing left to do
			return;
		}

		// For dual wield, prevent that 1 item can be equipped in two hands if its already equipped
		bool isEquipped = (rightEquipList || leftEquipList);
		if (targetEquipSlot && isEquipped && CanEquipBothHands(a_actor, a_item)) {
			hasItemMinCount = itemCount > 1;
		}

		if (!isTargetSlotInUse && hasItemMinCount) {
			CALL_MEMBER_FN(equipManager, EquipItem)(a_actor, a_item, extraList, equipCount, targetEquipSlot, a_equipSound, a_preventUnequip, false, 0);
		}
	}


	bool RegisterFuncs(VMClassRegistry* a_registry)
	{
		a_registry->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, bool, TESObjectWEAP*>("IsWeaponBound", "iEquip_ActorExt", iEquip_ActorExt::IsWeaponBound, a_registry));

		a_registry->RegisterFunction(
			new NativeFunction6<StaticFunctionTag, void, Actor*, TESForm*, SInt32, AlchemyItem*, bool, bool>("EquipPoisonedItemEx", "iEquip_ActorExt", iEquip_ActorExt::EquipPoisonedItemEx, a_registry));

		a_registry->RegisterFunction(
			new NativeFunction6<StaticFunctionTag, void, Actor*, TESForm*, SInt32, EnchantmentItem*, bool, bool>("EquipEnchantedItemEx", "iEquip_ActorExt", iEquip_ActorExt::EquipEnchantedItemEx, a_registry));

		a_registry->RegisterFunction(
			new NativeFunction7<StaticFunctionTag, void, Actor*, TESForm*, SInt32, AlchemyItem*, EnchantmentItem*, bool, bool>("EquipPoisonedAndEnchantedItemEx", "iEquip_ActorExt", iEquip_ActorExt::EquipPoisonedAndEnchantedItemEx, a_registry));

		return true;
	}
}