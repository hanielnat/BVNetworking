class BV_RestApiManager : Managed
{
	protected RestContext m_context;
	protected RestApi m_api;

	protected ref RestCallback m_testCallback;

	//------------------------------------------------------------------------------------------------
	void BV_RestApiManager(string url = "http://127.0.0.1:8000")
	{
		m_api = GetGame().GetRestApi();
		if (!m_api)
		{
			Print("[REST Hello Test] GetRestApi() returned null");
			return;
		}

		m_context = m_api.GetContext(url);
		if (!m_context)
		{
			Print("[REST Hello Test] Failed to create RestContext");
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	bool TryPost(string endpoint, string json, RestCallback cb)
	{
		// handle try-catch, logging, retries
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void TestHelloGet(string baseUrl = "http://127.0.0.1:8000", string endpoint = "/hello", string queryName = "Reforger")
	{
		if (!Replication.IsServer())
		{
			Print("[REST Hello Test] This example should preferably run on server");
			// continue anyway for local testing
		}

		string fullPath = endpoint;
		if (queryName != string.Empty)
		{
			fullPath = string.Format("%1?name=%2", endpoint, queryName);
		}

		m_testCallback = new RestCallback();
		m_testCallback.SetOnSuccess(BV_RestHelloCallBackOnSuccess);
		m_testCallback.SetOnError(BV_RestHelloCallBackOnError);

		// Perform the GET request (asynchronous)
		m_context.GET(m_testCallback, fullPath);

		PrintFormat("[REST Hello Test] Sent GET request to: %1%2", baseUrl, fullPath);
	}

	//------------------------------------------------------------------------------------------------
	void BV_RestHelloCallBackOnSuccess(RestCallback cb)
	{
		PrintFormat("[REST Hello Test] Success (code %1): %2", cb.GetHttpCode(), cb.GetData());
	}

	//------------------------------------------------------------------------------------------------
	void BV_RestHelloCallBackOnError(RestCallback cb)
	{
		PrintFormat("[REST Hello Test] Error: %1", cb.GetRestResult());
	}
}
