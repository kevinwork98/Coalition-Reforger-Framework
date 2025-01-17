[ComponentEditorProps(category: "Game Mode Component", description: "")]
class CRF_AdminMenuGameComponentClass: SCR_BaseGameModeComponentClass
{
	
}

class CRF_AdminMenuGameComponent: SCR_BaseGameModeComponent
{
	protected SCR_GroupsManagerComponent m_groupsManager;
	protected CRF_GearScriptGamemodeComponent m_GearScriptEditor;
	protected ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams();
	
	protected Widget m_wSavedHintWidget;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static CRF_AdminMenuGameComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return CRF_AdminMenuGameComponent.Cast(gameMode.FindComponent(CRF_AdminMenuGameComponent));
		else
			return null;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(AddMsgAction, 0, false);
	}
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Admin Messaging
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddMsgAction()
	{
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("admin");
		invoker.Insert(SendAdminMessage_Callback);
		ChatCommandInvoker invoker2 = chatPanelManager.GetCommandInvoker("a");
		invoker2.Insert(SendAdminMessage_Callback);
		ChatCommandInvoker invoker3 = chatPanelManager.GetCommandInvoker("r");
		invoker3.Insert(ReplyAdminMessage_Callback);
		ChatCommandInvoker invoker4 = chatPanelManager.GetCommandInvoker("reply");
		invoker4.Insert(ReplyAdminMessage_Callback);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminMessage_Callback(SCR_ChatPanel panel, string data)
	{
		CRF_ClientComponent.GetInstance().SendAdminMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReplyAdminMessage_Callback(SCR_ChatPanel panel, string data)
	{
		if(!SCR_Global.IsAdmin())
			return;
		
		CRF_ClientComponent.GetInstance().ReplyAdminMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminMessage(string data)
	{
		Rpc(RpcAsk_SendAdminMessage, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_SendAdminMessage(string data)
	{
		if(!SCR_Global.IsAdmin())
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		chatComponent.ShowMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReplyAdminMessage(string data, int playerID)
	{
		Rpc(RpcAsk_ReplyAdminMessage, data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_ReplyAdminMessage(string data, int playerID)
	{
		if(GetGame().GetPlayerController().GetPlayerId() != playerID)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		
		chatComponent.ShowMessage(string.Format("Admin: %1", data));

		LogAdminAction(string.Format("Reply to %1: %2", GetGame().GetPlayerManager().GetPlayerName(playerID), data), playerID, false);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Respawn
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SpawnGroupServer(int playerId, string prefab, vector spawnLocation, int groupID)
	{
		Rpc(Respawn, playerId, prefab, spawnLocation, groupID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Respawn(int playerId, string prefab, vector spawnLocation, int groupID)
	{
//		Rpc(RpcAsk_CloseMap, playerId);
		
		if(prefab.IsEmpty())
		{
			switch(m_groupsManager.FindGroup(groupID).m_faction)
			{
				case "BLUFOR" : {prefab = "{268EAF6C56517778}Prefabs/Characters/Factions/BLUFOR/US_Army/BLUFOR_AMG.et"; break;}
				case "OPFOR"  : {prefab = "{FC0904D71EF8DB6A}Prefabs/Characters/Factions/OPFOR/CRF_GS_OPFOR_Rifleman_P.et";   break;}
				case "INDFOR" : {prefab = "{A303C25424BC7149}Prefabs/Characters/Factions/INDFOR/CRF_GS_INDFOR_Rifleman_P.et"; break;}
				case "CIV"    : {prefab = "{71EF8F2C5207403C}Prefabs/Characters/Factions/CIV/CRF_GS_CIV_Rifleman_P.et";       break;}
			}
		}

		m_groupsManager = SCR_GroupsManagerComponent.GetInstance();
		Resource resource = Resource.Load(prefab);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
		vector finalSpawnLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(finalSpawnLocation, spawnLocation, 3);

		CLB_Gamemode.GetInstance().RespawnPlayer(playerId, prefab, finalSpawnLocation, groupID);
		LogAdminAction(string.Format("%1 was respawned to %2", GetGame().GetPlayerManager().GetPlayerName(playerId), m_groupsManager.FindGroup(groupID).m_faction), playerId, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RpcAsk_CloseMap(int playerID)
//	{
//		if(playerID == 0 || SCR_PlayerController.GetLocalPlayerId() != playerID)
//			return;
//		
//		CRF_SpectatorMenuUI spectatorMenu = CRF_SpectatorMenuUI.Cast(GetGame().GetMenuManager().GetTopMenu());
//		
//		if(spectatorMenu)
//			spectatorMenu.CloseMap();
//	}
		
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	void SwitchToSpawnedEntity(int playerId, IEntity entity, int frameCounter, int groupID)
//	{
//		if (frameCounter > 0) // Await four frames
//		{		
//			GetGame().GetCallqueue().Call(SwitchToSpawnedEntity, playerId, entity, frameCounter - 1, groupID);
//			return;
//		}
//		
//		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
//		
//		PS_PlayableComponent playableComponent = PS_PlayableComponent.Cast(entity.FindComponent(PS_PlayableComponent));
//		RplId playableId = playableComponent.GetId();
//		
//		if (playerId >= 0)
//		{
//			playableManager.SetPlayerPlayable(playerId, playableId);
//			playableManager.ForceSwitch(playerId);
//		}
//		SCR_AIGroup playerGroup = m_groupsManager.FindGroup(groupID);
//		
//		GetGame().GetCallqueue().CallLater(SetPlayerGroup, 1250, false, playerGroup, playerId, playableManager);
//	}
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetPlayerGroup(SCR_AIGroup group, int playerID)
	{
		SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(group.GetFaction());
		m_groupsManager.AddPlayerToGroup(group.GetGroupID(), playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Gear
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetPlayerGear(int playerID, string prefab)
	{	
		IEntity entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		m_GearScriptEditor = CRF_GearScriptGamemodeComponent.GetInstance();
		if(m_GearScriptEditor)
		{
			m_GearScriptEditor.SetupAddGearToEntity(entity, prefab);
			m_GearScriptEditor.SetPlayerGearScriptsMapValue(prefab, playerID, "GSR"); // GSR = Gear Script Resource
		}
		
		LogAdminAction(string.Format("%1's gear was set to %2", GetGame().GetPlayerManager().GetPlayerName(playerID), prefab.Substring(prefab.LastIndexOf("/") + 1, prefab.LastIndexOf(".") - prefab.LastIndexOf("/") - 1)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddItem(int playerID, string prefab)
	{
		if(playerID == 0 || prefab.IsEmpty())
			return;
		
		IEntity entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		SCR_InventoryStorageManagerComponent entityInventoryManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		m_SpawnParams.TransformMode = ETransformMode.WORLD;
        m_SpawnParams.Transform[3] = entity.GetOrigin();
		ref IEntity resourceSpawned = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), m_SpawnParams);
		if(!entityInventoryManager.TryInsertItem(resourceSpawned))
			delete resourceSpawned;
		
		LogAdminAction(string.Format("%2 was added to %1's inventory", GetGame().GetPlayerManager().GetPlayerName(playerID), prefab.Substring(prefab.LastIndexOf("/") + 1, prefab.LastIndexOf(".") - prefab.LastIndexOf("/") - 1)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teleport
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void TeleportPlayers(int playerID1, int playerID2)
	{
		Rpc(Rpc_TeleportPlayers, playerID1, playerID2);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_TeleportPlayers(int playerID1, int playerID2)
	{
		if(SCR_PlayerController.GetLocalPlayerId() != playerID1)
			return;
		
		IEntity entity2 = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID2);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
	    spawnParams.TransformMode = ETransformMode.WORLD;
		vector teleportLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(teleportLocation, entity2.GetOrigin(), 3);
	    spawnParams.Transform[3] = teleportLocation;
	
		SCR_Global.TeleportPlayer(playerID1, teleportLocation);
		
		LogAdminAction(string.Format("%1 was teleported to %2", GetGame().GetPlayerManager().GetPlayerName(playerID1), GetGame().GetPlayerManager().GetPlayerName(playerID2)), playerID1, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Hints
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintAll(string data)
	{
		Rpc(Rpc_SendHintAll, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintAll(string data)
	{
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintPlayer(string data, int playerID)
	{
		Rpc(Rpc_SendHintPlayer, data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintPlayer(string data, int playerID)
	{
		if(playerID == 0 || SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;
		
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintFaction(string data, string factionKey)
	{
		Rpc(Rpc_SendHintFaction, data, factionKey);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintFaction(string data, string factionKey)
	{
		if(factionKey.IsEmpty() && !SCR_FactionManager.SGetLocalPlayerFaction() && (SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction()).GetFactionKey() != factionKey))
			return;
		
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminHint(string data)
	{
		Widget widget;
		widget = GetGame().GetWorkspace().CreateWidgets("{43FC66BA3D85E9C7}UI/layouts/Hint/hint.layout");
		
		if (!widget)
			return;
		
		if(m_wSavedHintWidget)
			delete m_wSavedHintWidget;
		
		m_wSavedHintWidget = widget;
		
		CRF_Hint hint = CRF_Hint.Cast(widget.FindHandler(CRF_Hint));
		hint.ShowHint(data, 8000);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Heal
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayer(int playerID)
	{
		IEntity PlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(PlayerEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return;
		
		damageComponent.FullHeal();
		damageComponent.SetHealthScaled(1);
		
		LogAdminAction(string.Format("%1's was healed", GetGame().GetPlayerManager().GetPlayerName(playerID)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayerVehicle(int playerID)
	{
		IEntity PlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		IEntity VehicleEntity = SCR_CompartmentAccessComponent.GetVehicleIn(PlayerEntity);
		if (!VehicleEntity)
			return;
			
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(VehicleEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return;
		
		damageComponent.FullHeal();
		damageComponent.SetHealthScaled(1);
		
		LogAdminAction(string.Format("%1's vehicle was repaired", GetGame().GetPlayerManager().GetPlayerName(playerID)), playerID, true);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Log Admin Actions
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	void SendAdminMessage(string data)
	void LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		Rpc(RpcAsk_LogAdminAction, data, playerID, sendToPlayer);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		if (sendToPlayer)
		{
			if(GetGame().GetPlayerController().GetPlayerId() != playerID && !SCR_Global.IsAdmin())
				return;
		} else {
			if(!SCR_Global.IsAdmin())
				return;
		}

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		chatComponent.ShowMessage(data);
	}
}