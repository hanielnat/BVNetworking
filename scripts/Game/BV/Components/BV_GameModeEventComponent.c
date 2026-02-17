[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Gamemode event capturing component for use in event systems")]
class BV_GameModeEventComponentClass : SCR_BaseGameModeComponentClass
{
}

class BV_GameModeEventComponent : SCR_BaseGameModeComponent
{
	protected static BV_GameModeEventComponent s_instance;

	protected BV_PlayerTrackingSystem m_playerTrackingSystem;
	protected BV_PlayerEventSystem m_playerEventSystem;
	protected BV_AnalyticsSystem m_analyticsSystem;

	protected ref map<int, ref BV_PlayerData> m_mPlayerData = new map<int, ref BV_PlayerData>();

	[RplProp()]
	protected ref array<ref BV_PlayerData> m_aReplicatedPlayers = {};

	//------------------------------------------------------------------------------------------------
	static BV_GameModeEventComponent GetInstance()
	{
		if (!s_instance)
			return null;

		return s_instance;
	}

	//------------------------------------------------------------------------------------------------
	array<ref BV_PlayerData> GetReplicatedPlayerData()
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
		m_aReplicatedPlayers.Insert(BV_PlayerData.Create()); // for playerID 0

		m_playerEventSystem = BV_PlayerEventSystem.GetInstance();
		if (!m_playerEventSystem)
		{
			g_BVPrinter.Error("BV_GameModeEventComponent::OnPostInit  BV_PlayerEventSystem not present in World, exiting...");
			return;
		}

		m_playerTrackingSystem = BV_PlayerTrackingSystem.GetInstance();
		if (m_playerTrackingSystem)
		{
			EventProvider.ConnectEvent(m_playerEventSystem.OnPlayerConnected, this.HandleOnPlayerConnected);
			EventProvider.ConnectEvent(m_playerEventSystem.OnPlayerDisconnected, this.HandleOnPlayerDisconnected);
		}

		m_analyticsSystem = BV_AnalyticsSystem.GetInstance();
		if (!m_analyticsSystem)
		{
			g_BVPrinter.Error("BV_GameModeEventComponent::OnPostInit  BV_AnalyticsSystem not present in World, exiting...");
			return;
		}
	}

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

		g_BVPrinter.Trace("BV_GameModeEventComponent::OnPlayerAuditSuccess  captured event");
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout = -1)
	{
		if (!Replication.IsServer())
			return;

		if (m_playerEventSystem)
			m_playerEventSystem.OnPlayerDisconnected(playerId, SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));

		g_BVPrinter.Trace("BV_GameModeEventComponent::OnPlayerDisconnected  captured event");
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDataRegistered(int playerId)
	{
		BV_PlayerData pData = m_mPlayerData.Get(playerId);
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
	void RegisterPlayer(int playerId)
	{
		BV_PlayerData pData = BV_PlayerData.Create(
			playerId,
			SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId)
		);

		m_mPlayerData.Set(playerId, pData);

		PersistenceSystem pSystem = PersistenceSystem.GetInstance();
		if (!pSystem)
		{
			g_BVPrinter.ErrorWB("BV_PlayerTrackingSystem::RegisterPlayer  PersistenceSystem is not present in world");
			return;
		}

		pSystem.StartTracking(pData);
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPlayer(int playerId)
	{
		BV_PlayerData pData = BV_PlayerData.Create();
		m_mPlayerData.Take(playerId, pData);

		PersistenceSystem pSystem = PersistenceSystem.GetInstance();
		if (!pSystem)
		{
			g_BVPrinter.ErrorWB("BV_PlayerTrackingSystem::UnregisterPlayer  PersistenceSystem is not present in world");
			return;
		}

		pSystem.ReleaseTracking(pData);
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	void HandleOnPlayerConnected(int playerId, UUID playerUID)
	{
		g_BVPrinter.Trace(string.Format("BV_GameModeEventComponent::HandleOnPlayerConnected(playerId: %1, playerUID: %2)", playerId, playerUID));
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
		g_BVPrinter.Trace(string.Format("BV_GameModeEventComponent::HandleOnPlayerDisconnected(playerId: %1, playerUID: %2)", playerId, playerUID));
		UnregisterPlayer(playerId);
		OnPlayerDataUnRegistered(playerId);

		BV_AnalyticsEventBase evt = new BV_AnalyticsEventBase("disconnected");
		BV_ReplicatedAnalyticsMessage msg = new BV_ReplicatedAnalyticsMessage();
		msg.iCategory = BV_EReplicatedMessageCategory.GAMEMODE | BV_EReplicatedMessageCategory.ADMIN;

		m_analyticsSystem.LogAndReplicate(evt, msg);
	}
}
