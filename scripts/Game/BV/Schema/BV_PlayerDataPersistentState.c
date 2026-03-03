class BV_PlayerDataPersistentState : PersistentState
{
}

class BV_PlayerDataSerializer : ScriptedStateSerializer
{
	private static const int s_iStatusFieldsCount = 1;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return BV_PlayerDataPersistentState;
	}

	//------------------------------------------------------------------------------------------------
	override static event EDeserializeFailHandling GetDeserializeFailHandling()
	{
		return EDeserializeFailHandling.ERROR;
	}

	//------------------------------------------------------------------------------------------------
	override static event EScriptedStateDeserializeEvent GetDeserializeEvent()
	{
		return EScriptedStateDeserializeEvent.AFTER_CONSTRUCTOR;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		if (!Replication.IsRunning())
			return ESerializeResult.DEFAULT;

		BV_PlayerDataEntity pData = BV_PlayerDataEntity.Cast(instance);
		if (!pData)
			return ESerializeResult.ERROR;

		context.WriteValue("version", 1);
		context.WriteValue("playerID", pData.id.playerID);
		context.WriteValue("playerUID", pData.id.playerUID);

		context.WriteValue("rolesMask", pData.role.playerRoleMask);

		context.StartArray("status", s_iStatusFieldsCount);
		context.WriteValue("kills", pData.status.kills);
		context.EndArray();

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		BV_PlayerDataEntity pData = BV_PlayerDataEntity.Cast(instance);
		if (!pData)
			return ESerializeResult.ERROR;

		int version;
		context.Read(version);

		int playerId;
		context.Read(playerId);

		UUID playerUID;
		context.Read(playerUID);

		int roleMask;
		context.Read(roleMask);

		int statusKills;
		context.StartArray("status", s_iStatusFieldsCount);
		context.Read(statusKills);
		context.EndArray();

		pData.id.playerID = playerId;
		pData.id.playerUID = playerUID;
		pData.role.playerRoleMask = roleMask;
		pData.status.kills = statusKills;

		return ESerializeResult.OK;
	}
}
