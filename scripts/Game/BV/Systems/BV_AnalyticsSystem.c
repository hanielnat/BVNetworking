class BV_AnalyticsSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddPoint(WorldSystemPoint.RuntimeStarted)
			.AddPoint(WorldSystemPoint.PostFrame);
	}

	protected const float FLUSH_INTERVAL_SEC = 4.0;
	protected const int MAX_QUEUE_SIZE = 100;

	protected ref array<ref BV_AnalyticsEventBase> m_aEventQueue = {};
	protected float m_fFlushTimer = 0;

	protected int m_iEventsThisMinute = 0;
	protected float m_fLastMinuteCheck = 0;

	//------------------------------------------------------------------------------------------------
	static BV_AnalyticsSystem GetInstance()
	{
		World w = GetGame().GetWorld();
		if (!w)
			return null;

		return BV_AnalyticsSystem.Cast(w.FindSystem(BV_AnalyticsSystem));
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		// update events per second metric
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fLastMinuteCheck >= 60)
		{
			if (m_iEventsThisMinute > 0)
			{
				float rate = m_iEventsThisMinute / 60.0;
				g_BVPrinter.InfoWB(string.Format("BV_AnalyticsSystem::OnUpdatePoint  Analytics events last 60 s: %1 (%.1f / s)", m_iEventsThisMinute, rate));
			}

			m_iEventsThisMinute = 0;
			m_fLastMinuteCheck = now;
		}

		// update queue flush timer
		m_fFlushTimer += args.GetTimeSliceSeconds();
		if (m_fFlushTimer >= FLUSH_INTERVAL_SEC && !m_aEventQueue.IsEmpty())
		{
			FlushQueue();
			m_fFlushTimer = 0.0;
		}

	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnAnalyticsEvent(notnull BV_AnalyticsEventBase eventData)
	{
		ThrowEvent(OnAnalyticsEvent, eventData);
	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnReplicatedMessage(notnull BV_ReplicatedAnalyticsMessage msg)
	{
		ThrowEvent(OnReplicatedMessage, msg);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_BroadcastAnalyticsMessage(BV_ReplicatedAnalyticsMessage msg)
	{
		OnReplicatedMessage(msg);
		g_BVPrinter.InfoWB("Got replicated message " + msg.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void FlushQueue()
	{
		if (m_aEventQueue.IsEmpty())
			return;

		/*
		if (!m_restManager || !m_restManager.IsReady())
			return;
		*/

		array<ref BV_AnalyticsEventBase> batch = m_aEventQueue;
		m_aEventQueue = new array<ref BV_AnalyticsEventBase>();

		/*
			if (m_bSendEventsBatched)
				m_restManager.SendAnalyticsBatch(batch);
		*/

		foreach (BV_AnalyticsEventBase e : batch)
		{
			// m_restManager.SendAnalytics(e);
			g_BVPrinter.InfoWB("Sending event " + e.ToString());
		}

		m_iEventsThisMinute += batch.Count();
		g_BVPrinter.TraceWB(string.Format("Flushed %1 analytics events", batch.Count()));
	}

	//------------------------------------------------------------------------------------------------
	void QueueEvent(notnull BV_AnalyticsEventBase evt)
	{
		if (m_aEventQueue.Count() >= MAX_QUEUE_SIZE)
		{
			g_BVPrinter.Debug("Analytics queue full, dropping event " + evt.sEventType);
			return;
		}

		m_aEventQueue.Insert(evt);
	}

	//------------------------------------------------------------------------------------------------
	void ReplicateToClients(notnull BV_ReplicatedAnalyticsMessage msg)
	{
		g_BVPrinter.InfoWB("Sending replicated message " + msg.ToString());

		if (!Replication.IsServer())
			return;

		Rpc(Rpc_BroadcastAnalyticsMessage, msg);
	}

	//------------------------------------------------------------------------------------------------
	void LogAndReplicate(
		notnull BV_AnalyticsEventBase backendEvent,
		BV_ReplicatedAnalyticsMessage replicatedMsg = null
	)
	{
		// Backend
		QueueEvent(backendEvent);
		OnAnalyticsEvent(backendEvent);

		// Replication
		if (replicatedMsg)
			ReplicateToClients(replicatedMsg);
	}
}
