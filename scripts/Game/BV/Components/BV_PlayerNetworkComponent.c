[ComponentEditorProps(category: "GameScripted/PlayerController/Components", description: "Networking component to handle RPC/replication on client side")]
class BV_PlayerNetworkComponentClass : ScriptComponentClass
{
}

class BV_PlayerNetworkComponent : ScriptComponent
{
	protected BV_PlayerEventSystem m_eventSystem;
	protected RplComponent m_rplComp;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		m_rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_rplComp)
		{
			g_BVPrinter.Error("BV_PlayerNetworkComponent::OnPostInit: PlayerController doesn't have a RplComponent");
			return;
		}

		m_eventSystem = BV_PlayerEventSystem.GetInstance();
		if (!m_eventSystem)
		{
			g_BVPrinter.Error("BV_PlayerEventSystem not present in world");
			return;
		}

		EventProvider.ConnectEvent(BV_AnalyticsSystem.GetInstance().OnReplicatedMessage, this.OnReplicatedMessage);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		auto analytics = BV_AnalyticsSystem.GetInstance();
		if (analytics)
			EventProvider.DisconnectEvents(analytics, this);
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	protected void OnReplicatedMessage(BV_ReplicatedAnalyticsMessage msg)
	{
		g_BVPrinter.DebugWB(string.Format("BV_PlayerNetworkComponent::OnReplicatedMessage(msg: %1)", msg.Repr()));
	}
}
