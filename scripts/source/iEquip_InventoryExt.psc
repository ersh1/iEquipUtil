ScriptName iEquip_InventoryExt


; @brief Registers the passes script to be notified when a ref handle becomes active
; @param a_thisForm The form to register for the event (i.e. Self).
Function RegisterForRefHandleActiveEvent(Form a_thisForm) Global Native


; @brief Fires whenever an item of a tracked type is added to the player's inventory
; @param a_item The form of the item added
; @param a_refHandle The handle for identifying the item
; @param a_itemCount The number of items added
; @notes Tracked types include:
; * Weapon
; * Armor
Event OnRefHandleActive(Form a_item, Int a_refHandle, Int a_itemCount)
EndEvent


; @brief Registers the passes script to be notified when a ref handle becomes invalidated
; @param a_thisForm The form to register for the event (i.e. Self).
Function RegisterForOnRefHandleInvalidatedEvent(Form a_thisForm) Global Native


; @brief Fires whenever an item of a tracked type is removed from the player's inventory
; @param a_item The form of the item added
; @param a_refHandle The handle for identifying the item
; @notes Tracked types include:
; * Weapon
; * Armor
Event OnRefHandleInvalidated(Form a_item, Int a_refHandle)
EndEvent


; @brief Retrieves the full display name of the item.
; @param a_item The item to retrieve the full display name of.
; @param a_refHandle The handle to the item.
; @return Returns "" error, else returns the full display name.
String Function GetLongName(Form a_item, Int a_refHandle) Global Native


; @brief Retrieves the short display name of the item, without the temper string.
; @param a_item The item to retrieve the short display name of.
; @param a_refHandle The handle to the item.
; @return Returns "" error, else returns the short display name.
String Function GetShortName(Form a_item, Int a_refHandle) Global Native


; @brief Retrieves the poison count of the item.
; @param a_item The item to retrieve the poison count of.
; @param a_refHandle The handle to the item.
; @return Returns 0 error, else returns the poison count.
Int Function GetPoisonCount(Form a_item, Int a_refHandle) Global Native


; @brief Sets the poison count of the item.
; @param a_item The item to set the poison count on.
; @param a_refHandle The handle to the item.
; @param a_newCount The new poison count.
Function SetPoisonCount(Form a_item, Int a_refHandle, Int a_newCount) Global Native


; @brief Retrieves the enchantment on the item.
; @param a_item The item to retrieve the enchantment of.
; @param a_refHandle The handle to the item.
; @return Returns NONE error, else returns the enchantment.
Enchantment Function GetEnchantment(Form a_item, Int a_refHandle) Global Native


; @brief Equips the given item to the given actor.
; @param a_item The item to equip.
; @param a_refHandle The handle to the item.
; @param a_actor The actor to equip the item to.
; @param a_equipSlot The slot to equip the item to.
; @param a_preventUnequip If True, prevents the player from unequiping the item.
; @param a_equipSound If True, plays the equip sound.
Function EquipItem(Form a_item, Int a_refHandle, Actor a_actor, Int a_equipSlot = 0, Bool a_preventUnequip = False, Bool a_equipSound = True) Global Native