class BV_PlayerEventSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddPoint(WorldSystemPoint.RuntimeStarted);
	}

	//------------------------------------------------------------------------------------------------
	static BV_PlayerEventSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return BV_PlayerEventSystem.Cast(world.FindSystem(BV_PlayerEventSystem));
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		super.OnInit();
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
		super.OnCleanup();
	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnPlayerConnected(int playerId, UUID playerUID)
	{
		ThrowEvent(OnPlayerConnected, playerId, playerUID);
	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnPlayerDisconnected(int playerId, UUID playerUID)
	{
		ThrowEvent(OnPlayerDisconnected, playerId, playerUID);
	}
}
