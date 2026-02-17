class BV_TestCommand : ScrServerCommand
{
	//------------------------------------------------------------------------------------------------
	override event string GetKeyword()
	{
		return "getpdata";
	}

	//------------------------------------------------------------------------------------------------
	override protected int RequiredChatPermission()
	{
		return EPlayerRole.NONE;
	}

	//------------------------------------------------------------------------------------------------
	override event bool IsServerSide()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override event protected ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}

	//------------------------------------------------------------------------------------------------
	override event protected ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		BV_GameModeEventComponent comp = BV_GameModeEventComponent.GetInstance();
		if (!comp)
			return ScrServerCmdResult("Error getting BV_GameModeEventComponent", EServerCmdResultType.ERR);

		array<ref BV_PlayerData> pData = comp.GetReplicatedPlayerData();
		if (pData.Count() == 1)
			return ScrServerCmdResult("No players present", EServerCmdResultType.OK);

		string playerIds = string.Empty;
		for (int i = 1; i < pData.Count(); ++i)
		{
			playerIds += string.Format("[%1, %2], ", pData[i].m_playerId, pData[i].m_playerIdentityId);
		}

		return ScrServerCmdResult(playerIds, EServerCmdResultType.OK);
	}

	//------------------------------------------------------------------------------------------------
	protected override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
}
