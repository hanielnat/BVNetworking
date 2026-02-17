class BV_PlayerTrackingSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddExecuteAfter(BV_PlayerEventSystem, WorldSystemPoint.RuntimeStarted)
			.AddExecuteAfter(BV_AnalyticsSystem, WorldSystemPoint.RuntimeStarted)
			.AddPoint(WorldSystemPoint.PostFrame)
			.AddPoint(WorldSystemPoint.RuntimeStarted);
	}

	protected float m_fTimer = 0;
	protected ref map<int, ref BV_PlayerData> m_mPlayerData = new map<int, ref BV_PlayerData>();
	protected BV_PlayerEventSystem m_eventSystem;
	protected BV_AnalyticsSystem m_analyticsSystem;

	[RplProp()]
	protected ref array<ref BV_PlayerData> m_aReplicatedPlayers = {};

	[Attribute(defvalue: "10")]
	protected int m_iCheckInterval;

	//------------------------------------------------------------------------------------------------
	static BV_PlayerTrackingSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return BV_PlayerTrackingSystem.Cast(world.FindSystem(BV_PlayerTrackingSystem));
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		m_aReplicatedPlayers.Reserve(256);
		m_aReplicatedPlayers.Insert(BV_PlayerData.Create()); // for playerID 0

		m_eventSystem = BV_PlayerEventSystem.GetInstance();
		if (!m_eventSystem)
		{
			g_BVPrinter.Error("BV_PlayerEventSystem not present in world");
			return;
		}

		m_analyticsSystem = BV_AnalyticsSystem.GetInstance();
		if (!m_analyticsSystem)
		{
			g_BVPrinter.Error("BV_AnalyticsSystem not present in world");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float deltaSeconds = args.GetTimeSliceSeconds();

		m_fTimer += deltaSeconds;
		if (m_fTimer < m_iCheckInterval)
			return;

		m_fTimer = 0.0;

		g_BVPrinter.TraceWB(string.Format("BV_PlayerTrackingSystem::OnUpdatePoint  deltaSeconds=%1", deltaSeconds));
	}

	//------------------------------------------------------------------------------------------------
	array<ref BV_PlayerData> GetReplicatedPlayerData()
	{
		return m_aReplicatedPlayers;
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
	[ReceiverAttribute()]
	void HandleOnPlayerConnected(int playerId, UUID playerUID)
	{
		g_BVPrinter.Trace(string.Format("BV_PlayerTrackingSystem::HandleOnPlayerConnected(playerId: %1, playerUID: %2)", playerId, playerUID));
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
		g_BVPrinter.Trace(string.Format("BV_PlayerTrackingSystem::HandleOnPlayerDisconnected(playerId: %1, playerUID: %2)", playerId, playerUID));
		UnregisterPlayer(playerId);
		OnPlayerDataUnRegistered(playerId);

		BV_AnalyticsEventBase evt = new BV_AnalyticsEventBase("disconnected");
		BV_ReplicatedAnalyticsMessage msg = new BV_ReplicatedAnalyticsMessage();
		msg.iCategory = BV_EReplicatedMessageCategory.GAMEMODE | BV_EReplicatedMessageCategory.ADMIN;

		m_analyticsSystem.LogAndReplicate(evt, msg);
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
}
