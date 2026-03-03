[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Gamemode event capturing component for use in event systems")]
class BV_GameModeEventComponentClass : SCR_BaseGameModeComponentClass
{
}

class BV_GameModeEventComponent : SCR_BaseGameModeComponent
{
	protected static BV_GameModeEventComponent s_instance;
	protected static ref BV_FmtPrinter s_printer = new BV_FmtPrinter("BVGameModeEvents");

	protected BV_PlayerTrackingSystem m_playerTrackingSystem;
	protected BV_PlayerEventSystem m_playerEventSystem;
	protected BV_AnalyticsSystem m_analyticsSystem;
	protected BV_PlayerDataPersistenceManager m_dataManager;
	protected PersistenceSystem m_persistenceSystem;

	protected ref map<int, ref BV_PlayerDataEntity> m_mPlayerData = new map<int, ref BV_PlayerDataEntity>();

	[RplProp()]
	protected ref array<ref BV_PlayerDataEntity> m_aReplicatedPlayers = {};

	//------------------------------------------------------------------------------------------------
	static BV_GameModeEventComponent GetInstance()
	{
		if (!s_instance)
			return null;

		return s_instance;
	}

	//------------------------------------------------------------------------------------------------
	array<ref BV_PlayerDataEntity> GetReplicatedPlayerData()
	{
		return m_aReplicatedPlayers;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		s_instance = this;

		m_aReplicatedPlayers.Reserve(256);
		m_aReplicatedPlayers.Insert(BV_PlayerDataEntity.Create()); // for playerID 0

		m_playerEventSystem = BV_PlayerEventSystem.GetInstance();
		if (!m_playerEventSystem)
		{
			s_printer.Error("OnPostInit  BV_PlayerEventSystem not present in World, exiting...");
			return;
		}

		m_analyticsSystem = BV_AnalyticsSystem.GetInstance();
		if (!m_analyticsSystem)
		{
			s_printer.Error("OnPostInit  BV_AnalyticsSystem not present in World, exiting...");
			return;
		}

		m_playerTrackingSystem = BV_PlayerTrackingSystem.GetInstance();
		if (!m_playerTrackingSystem)
		{
			s_printer.Error("OnPostInit  BV_PlayerTrackingSystem not present in World, exiting...");
			return;
		}

		m_persistenceSystem = PersistenceSystem.GetInstance();
		if (!m_persistenceSystem)
		{
			s_printer.Error("OnPostInit  PersistenceSystem not present in World, exiting...");
			return;
		}

		m_dataManager = BV_PlayerDataPersistenceManager.GetInstance();
		if (!m_dataManager)
		{
			s_printer.Error("OnPostInit  BV_PlayerDataPersistenceManager not present, exiting...");
			return;
		}

		BV_RestApiManager restManager = BV_RestApiManager.GetInstance();
		if (!restManager)
		{
			s_printer.Error("OnPostInit  BV_RestApiManager not present, exiting...");
			return;
		}

		m_analyticsSystem.SetRestManager(restManager);

		EventProvider.ConnectEvent(m_playerEventSystem.OnPlayerConnected, this.HandleOnPlayerConnected);
		EventProvider.ConnectEvent(m_playerEventSystem.OnPlayerDisconnected, this.HandleOnPlayerDisconnected);

		#ifdef WORKBENCH
		GetGame().GetCallqueue().CallLater(TestFireEvents, 4000, true);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	#ifdef WORKBENCH
	private void TestFireEvents()
	{
		s_printer.DebugWB("creating 10 ping events");
		for (int i = 0; i < 10; ++i)
		{
			BV_PingAnalyticsEvent.Fire();
		}
		s_printer.DebugWB("ping events creation done");
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_playerEventSystem)
			EventProvider.DisconnectEvents(m_playerEventSystem, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (m_playerEventSystem)
			m_playerEventSystem.OnPlayerConnected(playerId, SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout = -1)
	{
		if (!Replication.IsServer())
			return;

		if (m_playerEventSystem)
			m_playerEventSystem.OnPlayerDisconnected(playerId, SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDataRegistered(int playerId)
	{
		BV_PlayerDataEntity pData = m_mPlayerData.Get(playerId);
		m_aReplicatedPlayers.Insert(pData);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDataUnRegistered(int playerId)
	{
		if (!m_aReplicatedPlayers.IsIndexValid(playerId))
			return;

		m_aReplicatedPlayers.Remove(playerId);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	private BV_PlayerDataEntity CreateDataForPlayer(int playerId)
	{
		BV_PlayerIdData idData = BV_PlayerIdData();
		BV_PlayerRoleData roleData = BV_PlayerRoleData();
		BV_PlayerStatusData statusData = BV_PlayerStatusData();

		UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);

		idData.playerID = playerId;
		idData.playerUID = playerUID;

		roleData.playerRoleMask = 0;

		statusData.kills = 0;

		return BV_PlayerDataEntity.Create(idData, roleData, statusData);
	}

	//------------------------------------------------------------------------------------------------
	void RegisterPlayer(int playerId)
	{
		BV_PlayerDataEntity pData = CreateDataForPlayer(playerId);

		if (!m_dataManager.LoadEntity(playerUID, pData))
		{
			s_printer.DebugWB("RegisterPlayer  creating entity for first join player " + playerUID);

			if (!m_dataManager.SaveEntity(pData))
			{
				s_printer.Error("RegisterPlayer  error creating entity for first join player " + playerUID);
				return;
			}
		}

		m_mPlayerData.Set(playerId, pData);

		// TODO: test
		m_persistenceSystem.SetId(pData, playerUID);
		m_persistenceSystem.StartTracking(pData);
		m_persistenceSystem.Save(pData);
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPlayer(int playerId)
	{
		BV_PlayerDataEntity pData = BV_PlayerDataEntity.Create();
		m_mPlayerData.Take(playerId, pData);
		m_persistenceSystem.ReleaseTracking(pData);

		if (!m_dataManager.SaveEntity(pData))
		{
			s_printer.Error("UnregisterPlayer  error saving player data entity");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	void HandleOnPlayerConnected(int playerId, UUID playerUID)
	{
		s_printer.Trace(string.Format("HandleOnPlayerConnected(playerId: %1, playerUID: %2)", playerId, playerUID));
		RegisterPlayer(playerId);
		OnPlayerDataRegistered(playerId);

		BV_AnalyticsEventBase evt = new BV_AnalyticsEventBase("connected");
		BV_ReplicatedAnalyticsMessage msg = new BV_ReplicatedAnalyticsMessage();
		msg.iCategory = BV_EReplicatedMessageCategory.GAMEMODE | BV_EReplicatedMessageCategory.ADMIN;

		m_analyticsSystem.LogAndReplicate(evt, msg);
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	void HandleOnPlayerDisconnected(int playerId, UUID playerUID)
	{
		s_printer.Trace(string.Format("HandleOnPlayerDisconnected(playerId: %1, playerUID: %2)", playerId, playerUID));
		UnregisterPlayer(playerId);
		OnPlayerDataUnRegistered(playerId);

		BV_AnalyticsEventBase evt = new BV_AnalyticsEventBase("disconnected");
		BV_ReplicatedAnalyticsMessage msg = new BV_ReplicatedAnalyticsMessage();
		msg.iCategory = BV_EReplicatedMessageCategory.GAMEMODE | BV_EReplicatedMessageCategory.ADMIN;

		m_analyticsSystem.LogAndReplicate(evt, msg);
	}
}
