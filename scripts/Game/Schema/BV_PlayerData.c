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

		// BV_PlayerDataPersistentState pData = BV_PlayerDataPersistentState.Cast(instance);
		BV_PlayerData pData = BV_PlayerData.Cast(instance);
		if (!pData)
			return ESerializeResult.ERROR;

		context.WriteValue("version", 1);
		context.WriteValue("playerID", pData.m_playerId);
		context.WriteValue("playerUID", pData.m_playerIdentityId);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		// BV_PlayerDataPersistentState pData = BV_PlayerDataPersistentState.Cast(instance);
		BV_PlayerData pData = BV_PlayerData.Cast(instance);
		if (!pData)
			return ESerializeResult.ERROR;

		int version;
		context.Read(version);

		int playerId;
		context.Read(playerId);

		UUID playerUID;
		context.Read(playerUID);

		pData.m_playerId = playerId;
		pData.m_playerIdentityId = playerUID;

		return ESerializeResult.OK;
	}
}

class BV_PlayerData : Managed
{
	// int(4) + UUID(16)
	protected const int SIZE = 20;

	int m_playerId = 0;
	UUID m_playerIdentityId = UUID.NULL_UUID;

	//------------------------------------------------------------------------------------------------
	static BV_PlayerData Create(int playerId = 0, UUID playerUID = UUID.NULL_UUID)
	{
		BV_PlayerData pData = BV_PlayerData();
		pData.m_playerId = playerId;
		pData.m_playerIdentityId = playerUID;

		return pData;
	}

	//------------------------------------------------------------------------------------------------
	// Server: instance → snapshot (raw bytes)
	static bool Extract(BV_PlayerData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(instance.m_playerId, 4);
		snapshot.SerializeBytes(instance.m_playerIdentityId, 16); // UUID = 16-byte binary
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Client: snapshot → instance
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, BV_PlayerData instance)
	{
		snapshot.SerializeBytes(instance.m_playerId, 4);
		snapshot.SerializeBytes(instance.m_playerIdentityId, 16);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Server: compare two snapshots (binary equality over fixed size)
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return !lhs.CompareSnapshots(rhs, this.SIZE); // 4 + 16 = 20 bytes; ! to match "different = true" convention
	}

	//------------------------------------------------------------------------------------------------
	// Server: instance vs snapshot (check if fields differ)
	static bool PropCompare(BV_PlayerData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		// Compare each field individually against snapshot content
		bool changed = false;

		changed = changed || !snapshot.Compare(instance.m_playerId, 4);
		changed = changed || !snapshot.Compare(instance.m_playerIdentityId, 16);

		return changed; // true = needs update
	}

	//------------------------------------------------------------------------------------------------
	// Server: snapshot → network packet (full copy, fixed size)
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, this.SIZE); // exact size of our data
	}

	//------------------------------------------------------------------------------------------------
	// Client: network packet → snapshot
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.Serialize(packet, this.SIZE);
		return true;
	}
}
