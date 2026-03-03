class BV_PlayerIdData : Managed
{
	static const int SIZE = 20;
	int playerID;
	UUID playerUID;
}

class BV_PlayerRoleData : Managed
{
	static const int SIZE = 4;
	int playerRoleMask;
}

class BV_PlayerStatusData : Managed
{
	static const int SIZE = 4;
	int kills;
}

[EDF_DbName("PlayerData")]
class BV_PlayerDataEntity : EDF_DbEntity
{
	protected static int SIZE;
	ref BV_PlayerIdData id;
	ref BV_PlayerRoleData role;
	ref BV_PlayerStatusData status;

	//------------------------------------------------------------------------------------------------
	static BV_PlayerDataEntity Create(
		BV_PlayerIdData pID = null,
		BV_PlayerRoleData pRole = null,
		BV_PlayerStatusData pStatus = null)
	{
		BV_PlayerDataEntity instance = BV_PlayerDataEntity();

		// player id data should not be null when giving it to the entity, but allow
		// passing null for easier usage
		if (pID == null)
		{
			// TODO: fix pID is null even after new()
			pID = new BV_PlayerIdData();
			pID.playerID = -1;
			pID.playerUID = UUID.NULL_UUID;
		}

		instance.id = pID;
		instance.role = pRole;
		instance.status = pStatus;

		instance.SIZE = pID.SIZE + pRole.SIZE + pStatus.SIZE;
		return instance;
	}

	//------------------------------------------------------------------------------------------------
	static bool Extract(BV_PlayerDataEntity instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(instance.id, instance.id.SIZE);
		snapshot.SerializeBytes(instance.role, instance.role.SIZE);
		snapshot.SerializeBytes(instance.status, instance.status.SIZE);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, BV_PlayerDataEntity instance)
	{
		snapshot.SerializeBytes(instance.id, instance.id.SIZE);
		snapshot.SerializeBytes(instance.role, instance.role.SIZE);
		snapshot.SerializeBytes(instance.status, instance.status.SIZE);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return !lhs.CompareSnapshots(rhs, this.SIZE);
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(BV_PlayerDataEntity instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		bool changed = false;

		changed = changed || !snapshot.Compare(instance.id, instance.id.SIZE);
		changed = changed || !snapshot.Compare(instance.role, instance.role.SIZE);
		changed = changed || !snapshot.Compare(instance.status, instance.status.SIZE);

		return changed; // true = needs update
	}

	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, this.SIZE);
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.Serialize(packet, this.SIZE);
		return true;
	}
}
