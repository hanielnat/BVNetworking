class BV_PlayerDataPersistentState : PersistentState
{
}

class BV_PlayerDataSerializer : ScriptedStateSerializer
{
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
		context.WriteValue("playerID", pData.id);
		context.WriteValue("roles", pData.role);
		context.WriteValue("status", pData.status);

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

		BV_PlayerIdData idData;
		context.Read(idData);

		BV_PlayerRoleData roleData;
		context.Read(roleData);

		BV_PlayerStatusData statusData;
		context.Read(statusData);

		pData.id = idData;
		pData.role = roleData;
		pData.status = statusData;

		return ESerializeResult.OK;
	}
}
