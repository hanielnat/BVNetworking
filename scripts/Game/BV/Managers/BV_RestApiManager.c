class BV_BaseRestCallback : RestCallback
{
	protected static ref BV_FmtPrinter s_printer = new BV_FmtPrinter("BVRestCallback");

	//------------------------------------------------------------------------------------------------
	void BV_BaseRestCallback()
	{
		SetOnSuccess(OnSuccessFunction);
		SetOnError(OnErrorFunction);
	}

	//------------------------------------------------------------------------------------------------
	void OnSuccessFunction(RestCallback cb)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnErrorFunction(RestCallback cb)
	{
	}
}

class BV_PingRestCallback : BV_BaseRestCallback
{
	//------------------------------------------------------------------------------------------------
	override void OnSuccessFunction(RestCallback cb)
	{
		s_printer.TraceWB(string.Format("OnSuccessFunction(cb: %1)", cb));
		s_printer.TraceWB(string.Format("HttpCode=%1, Data=%2", cb.GetHttpCode(), cb.GetData()));
		BV_RestApiManager.GetInstance().OnPingHandled(cb.GetData());
	}

	//------------------------------------------------------------------------------------------------
	override void OnErrorFunction(RestCallback cb)
	{
		s_printer.Error(string.Format("%1  HttpCode=%2  request failed", ClassName(), cb.GetHttpCode()));
		BV_RestApiManager.GetInstance().OnPingHandled(string.Empty);
	}
}

class BV_RestApiManager : Managed
{
	protected static const string URL_FILE_PATH = "$profile:.bv/api";
	protected static const string URL_FILE_NAME = "apiManager.json";

	protected static const string API_STR = "/api";
	protected static const string API_V1_STR = "/api/v1";

	protected static ref BV_FmtPrinter s_printer = new BV_FmtPrinter("BVRestManager");
	protected static ref BV_RestApiManager s_instance;

	protected bool m_bIsAvailable;

	protected RestApi m_api;
	protected RestContext m_context;
	protected ref BV_BaseRestCallback m_callback;

	//------------------------------------------------------------------------------------------------
	void BV_RestApiManager(string url = "http://127.0.0.1:8000")
	{
		m_api = GetGame().GetRestApi();
		if (!m_api)
		{
			s_printer.Error("GetRestApi() returned null");
			return;
		}

		FileIO.MakeDirectory(URL_FILE_PATH);
		if (!FileIO.FileExists(URL_FILE_PATH))
		{
			s_printer.Error("REST API manager folder creation failed");
			return;
		}

		string fullUrlFile = string.Format("%1/%2", URL_FILE_PATH, URL_FILE_NAME);
		if (!FileIO.FileExists(fullUrlFile))
		{
			SCR_JsonSaveContext saveCtx = new SCR_JsonSaveContext();
			saveCtx.WriteValue("baseUrl", url);

			if (!saveCtx.SaveToFile(fullUrlFile))
			{
				s_printer.Error("REST API manager JSON file creation failed");
				return;
			}
		}
		else
		{
			SCR_JsonLoadContext loadCtx = new SCR_JsonLoadContext();

			if (!loadCtx.LoadFromFile(fullUrlFile))
			{
				s_printer.Error("REST API manager JSON file loading failed");
				return;
			}

			loadCtx.ReadValue("baseUrl", url);
		}

		m_context = m_api.GetContext(url);
		if (!m_context)
		{
			s_printer.Error("Default RestContext creation failed");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~BV_RestApiManager()
	{
		m_bIsAvailable = false;
	}

	//------------------------------------------------------------------------------------------------
	static BV_RestApiManager GetInstance()
	{
		if (!s_instance)
			s_instance = new BV_RestApiManager();

		return s_instance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAvailable()
	{
		return m_bIsAvailable;
	}

	//------------------------------------------------------------------------------------------------
	void OnPingHandled(string data)
	{
		if (data != "Pong")
			m_bIsAvailable = false;

		m_bIsAvailable = true;
	}

	//------------------------------------------------------------------------------------------------
	void SendPing()
	{
		m_context.POST(m_callback, API_V1_STR + "/ping", "Ping");
	}

	//------------------------------------------------------------------------------------------------
	void SendAnalytics(BV_AnalyticsEventBase ev)
	{
		if (!ev.HasData())
			ev.Pack();

		string payload = ev.AsString();

		s_printer.Trace(string.Format("sending analytics event '%1'", payload));
		m_context.POST(m_callback, API_V1_STR + "/analytics/log", payload);
	}

	//------------------------------------------------------------------------------------------------
	void SendAnalyticsBatch(notnull array<ref BV_AnalyticsEventBase> batch)
	{
		if (batch.IsEmpty())
			return;

		array<string> aPayload = {};
		aPayload.Reserve(batch.Count());

		foreach (BV_AnalyticsEventBase ev : batch)
		{
			if (!ev.HasData())
				ev.Pack();

			aPayload.Insert(ev.AsString());
		}

		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		ctx.WriteValue("count", aPayload.Count());
		ctx.WriteValue("events", aPayload);

		string sPayload = ctx.ExportToString();
		if (sPayload == string.Empty)
		{
			s_printer.Error("can't serialize analytics event batch, unknown error");
			return;
		}

		s_printer.Trace(string.Format("sending analytics event batch, payload='%1'", sPayload));
		m_context.POST(m_callback, API_V1_STR + "/analytics/log", sPayload);
	}

	//------------------------------------------------------------------------------------------------
	void SendPlayerData(UUID playerUID, BV_PlayerData pData)
	{
		string params = string.Format("%1/player/%2", API_V1_STR, playerUID);
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		string payload = ctx.ExportToString();

		m_context.POST(m_callback, params, payload);
	}

	//------------------------------------------------------------------------------------------------
	void SendAdminCommand(string cmd)
	{
		string payload = "\{\"cmd\": " + cmd + "\"\}";
		m_context.POST(m_callback, API_V1_STR + "/admin", payload)
	}
}
