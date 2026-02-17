class BV_AnalyticsEventBase : JsonApiStruct
{
	string sEventType;
	int iTimestamp;

	//------------------------------------------------------------------------------------------------
	void BV_AnalyticsEventBase(string eventType = string.Empty)
	{
		RegV("sEventType");
		RegV("iTimestamp");

		sEventType = eventType;
		iTimestamp = System.GetUnixTime();
	}

	//------------------------------------------------------------------------------------------------
	string Repr()
	{
		return string.Format("%1(sEventType='%2', iTimestamp=%3)", this.ClassName(), sEventType, iTimestamp);
	}

	//------------------------------------------------------------------------------------------------
	string ToJsonString()
	{
		Pack();
		return AsString();
	}
}
