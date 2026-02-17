class BV_PlayerTrackingSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddPoint(WorldSystemPoint.PostFrame)
			.AddPoint(WorldSystemPoint.RuntimeStarted);
	}

	protected float m_fTimer = 0;

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
		/*
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
		*/
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
}
