class BV_PingAnalyticsEvent : BV_AnalyticsEventBase
{
	string sMessage;

	protected static ref BV_FmtPrinter s_printer = new BV_FmtPrinter("BVPingEvent");

	//------------------------------------------------------------------------------------------------
	static void Fire(string message = "Ping")
	{
		BV_AnalyticsSystem sys = BV_AnalyticsSystem.GetInstance();
		if (!sys)
		{
			s_printer.Error("BV_AnalyticsSystem not present in world. Can't trigger event.");
			return;
		}

		BV_PingAnalyticsEvent evt = BV_PingAnalyticsEvent(message);
		sys.LogAndReplicate(evt);
	}
}
