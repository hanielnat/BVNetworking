class BV_ReplicatedAnalyticsMessage : Managed
{
	// int(4) + int(4)
	protected const int SIZE = 8;

	[RplProp()]
	int iTimestamp;

	[RplProp()]
	int iCategory;

	//------------------------------------------------------------------------------------------------
	void BV_ReplicatedAnalyticsMessage()
	{
		iTimestamp = System.GetUnixTime();
		iCategory = BV_EReplicatedMessageCategory.NONE;
	}

	//------------------------------------------------------------------------------------------------
	string AsString()
	{
		return string.Format("%1(iTimestamp=%2, iCategory=%3)", this.ClassName(), iTimestamp, iCategory);
	}

	//------------------------------------------------------------------------------------------------
	// Server: instance → snapshot (raw bytes)
	static bool Extract(BV_ReplicatedAnalyticsMessage instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(instance.iTimestamp, 4);
		snapshot.SerializeBytes(instance.iCategory, 4);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Client: snapshot → instance
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, BV_ReplicatedAnalyticsMessage instance)
	{
		snapshot.SerializeBytes(instance.iTimestamp, 4);
		snapshot.SerializeBytes(instance.iCategory, 4);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Server: compare two snapshots (binary equality over fixed size)
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return !lhs.CompareSnapshots(rhs, this.SIZE); // 4 + 4 = 8 bytes; ! to match "different = true" convention
	}

	//------------------------------------------------------------------------------------------------
	// Server: instance vs snapshot (check if fields differ)
	static bool PropCompare(BV_ReplicatedAnalyticsMessage instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		// Compare each field individually against snapshot content
		bool changed = false;

		changed = changed || !snapshot.Compare(instance.iTimestamp, 4);
		changed = changed || !snapshot.Compare(instance.iCategory, 4);

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
