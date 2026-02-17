[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Gamemode event capturing component for use in event systems")]
class BV_GameModeEventComponentClass : SCR_BaseGameModeComponentClass
{
}

class BV_GameModeEventComponent : SCR_BaseGameModeComponent
{
	protected BV_PlayerEventSystem m_playerEventSystem;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_playerEventSystem = BV_PlayerEventSystem.GetInstance();
		if (!m_playerEventSystem)
		{
			g_BVPrinter.Error("BV_GameModeEventComponent::OnPostInit  BV_PlayerEventSystem not present in World, exiting...");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (m_playerEventSystem)
			m_playerEventSystem.OnPlayerConnected(playerId, SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));

		g_BVPrinter.Trace("BV_GameModeEventComponent::OnPlayerAuditSuccess  captured event");
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout = -1)
	{
		if (!Replication.IsServer())
			return;

		if (m_playerEventSystem)
			m_playerEventSystem.OnPlayerDisconnected(playerId, SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));

		g_BVPrinter.Trace("BV_GameModeEventComponent::OnPlayerDisconnected  captured event");
	}
}
