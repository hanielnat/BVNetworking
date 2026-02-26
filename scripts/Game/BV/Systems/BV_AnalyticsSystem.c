class BV_AnalyticsSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddPoint(WorldSystemPoint.RuntimeStarted)
			.AddPoint(WorldSystemPoint.PostFrame);
	}

	[Attribute(defvalue: "1")]
	protected bool m_bSendEventsBatched;

	[Attribute(defvalue: "100")]
	protected int m_iMaxQueueSize;

	[Attribute(defvalue: "4.0")]
	protected float m_fFlushIntervalSeconds;

	protected static const string EVENTLOG_FILE_PATH = "$profile:.bv/api";
	protected static const string EVENTLOG_FILE_NAME = "eventLog_%1_%2";

	protected static ref BV_FmtPrinter s_printer = new BV_FmtPrinter("BVAnalytics");

	protected ref array<ref BV_AnalyticsEventBase> m_aEventQueue = {};
	protected BV_RestApiManager m_restManager;
	protected ref FileHandle m_eventLogFileHandle;

	protected int m_iTotalEventCount;
	protected int m_iEventsThisMinute;

	protected float m_fFlushTimer;
	protected float m_fLastMinuteCheck;

	private string m_sCurrentEventLogFileName;

	//------------------------------------------------------------------------------------------------
	static BV_AnalyticsSystem GetInstance()
	{
		World w = GetGame().GetWorld();
		if (!w)
			return null;

		return BV_AnalyticsSystem.Cast(w.FindSystem(BV_AnalyticsSystem));
	}

	//------------------------------------------------------------------------------------------------
	BV_RestApiManager GetRestManager()
	{
		return m_restManager;
	}

	//------------------------------------------------------------------------------------------------
	void SetRestManager(notnull BV_RestApiManager instance)
	{
		m_restManager = instance;
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnInit()
	{
		m_aEventQueue.Reserve(m_iMaxQueueSize);

		FileIO.MakeDirectory(EVENTLOG_FILE_PATH);
		if (!FileIO.FileExists(EVENTLOG_FILE_PATH))
		{
			s_printer.Error("event log folder creation failed");
			return;
		}

		string logFileName = string.Empty;
		GetEventLogFileName(logFileName);

		FileMode fMode = FileMode.APPEND;
		m_eventLogFileHandle = FileIO.OpenFile(logFileName, fMode);

		if (!m_eventLogFileHandle)
		{
			s_printer.Error(string.Format("unkown error trying to open event log file, logFileName='%1'  FileMode='%2'", logFileName, fMode));
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnCleanup()
	{
		if (m_eventLogFileHandle && m_eventLogFileHandle.IsOpen())
			m_eventLogFileHandle.Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		// update events per second metric
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fLastMinuteCheck >= 60)
		{
			if (m_iEventsThisMinute > 0)
			{
				float rate = m_iEventsThisMinute / 60.0;
				s_printer.Trace(string.Format("BV_AnalyticsSystem::OnUpdatePoint  Analytics events last 60 s: %1 (%2 / s)", m_iEventsThisMinute, rate));
			}

			m_iEventsThisMinute = 0;
			m_fLastMinuteCheck = now;
		}

		// update queue flush timer
		m_fFlushTimer += args.GetTimeSliceSeconds();
		if (m_fFlushTimer >= m_fFlushIntervalSeconds && !m_aEventQueue.IsEmpty())
		{
			FlushQueue();
			m_fFlushTimer = 0.0;
		}
	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnAnalyticsEvent(notnull BV_AnalyticsEventBase eventData)
	{
		ThrowEvent(OnAnalyticsEvent, eventData);
	}

	//------------------------------------------------------------------------------------------------
	[EventAttribute()]
	void OnReplicatedMessage(notnull BV_ReplicatedAnalyticsMessage msg)
	{
		ThrowEvent(OnReplicatedMessage, msg);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_BroadcastAnalyticsMessage(BV_ReplicatedAnalyticsMessage msg)
	{
		OnReplicatedMessage(msg);
		s_printer.DebugWB("Got replicated message " + msg.Repr());
	}

	//------------------------------------------------------------------------------------------------
	protected void SwapQueue(inout notnull array<ref BV_AnalyticsEventBase> other)
	{
		other.Reserve(m_aEventQueue.Count());
		SCR_ArrayHelperRefT<BV_AnalyticsEventBase>.CopyReferencesFromTo(m_aEventQueue, other);
		m_aEventQueue.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void FlushQueue()
	{
		if (m_aEventQueue.IsEmpty())
			return;

		int droppedEvents = 0;
		array<ref BV_AnalyticsEventBase> batch = {};
		SwapQueue(batch);

		if (m_restManager.IsAvailable()) // send to rest api primarily
		{
			if (m_bSendEventsBatched)
				m_restManager.SendAnalyticsBatch(batch);

			foreach (BV_AnalyticsEventBase e : batch)
			{
				m_restManager.SendAnalytics(e);
			}
		}
		else // write to file as fallback
		{
			foreach (BV_AnalyticsEventBase e : batch)
			{
				if (!AppendEventToFile(e))
				{
					s_printer.Error(string.Format("dropping event that has failed to be appended to log file,  event='%1'", e.Repr()));
					droppedEvents += 1;
				}
			}
		}

		int succesfullEvents = batch.Count() - droppedEvents;
		m_iEventsThisMinute += succesfullEvents;
		m_iTotalEventCount += succesfullEvents;

		s_printer.Trace(string.Format("flushed %1 analytics events, failed to flush %2", succesfullEvents, droppedEvents));
	}

	//------------------------------------------------------------------------------------------------
	protected void GetEventLogFileName(out string outFileName, string extension = ".jsonl")
	{
		if (m_sCurrentEventLogFileName != string.Empty)
		{
			outFileName = m_sCurrentEventLogFileName;
			return;
		}

		int hour, minute, second;
		System.GetHourMinuteSecondUTC(hour, minute, second);
		string hourMinuteSecond = string.Format("%1-%2-%3", hour, minute, second);

		int year, month, day;
		System.GetYearMonthDayUTC(year, month, day);
		string yearMonthDay = string.Format("%1-%2-%3", year, month, day);

		string logSession = string.Format(EVENTLOG_FILE_NAME, hourMinuteSecond, yearMonthDay);
		string logFullFileName = string.Format("%1/%2%3", EVENTLOG_FILE_PATH, logSession, extension);

		m_sCurrentEventLogFileName = logFullFileName;
		outFileName = logFullFileName;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AppendEventToFile(notnull BV_AnalyticsEventBase ev)
	{
		if (!m_eventLogFileHandle)
			return false;

		if (!ev.HasData())
			ev.Pack();

		string evJson = ev.AsString();
		if (evJson == "{}") // FIXME: it seems that after 64 'JsonApiStruct's instantiated, 'AsString()' returns '{}'
		{
			s_printer.Error("can't serialize event, unknown error");
			return false;
		}

		s_printer.DebugWB(string.Format("sending analytics event to log file, event='%1'  logFile='%2'", evJson, m_sCurrentEventLogFileName));

		int currentPos = m_eventLogFileHandle.GetPos();
		m_eventLogFileHandle.WriteLine(evJson);
		int afterWritePos = m_eventLogFileHandle.GetPos();

		if (afterWritePos == 0 || afterWritePos <= currentPos)
		{
			s_printer.Error(string.Format("error appending JSON object to event log file, object='%1'  logFile='%2'", evJson, m_sCurrentEventLogFileName));
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void QueueEvent(notnull BV_AnalyticsEventBase evt)
	{
		if (m_aEventQueue.Count() >= m_iMaxQueueSize)
		{
			s_printer.Info("Analytics queue full, dropping event: " + evt.sEventType);
			return;
		}

		m_aEventQueue.Insert(evt);
	}

	//------------------------------------------------------------------------------------------------
	void ReplicateToClients(notnull BV_ReplicatedAnalyticsMessage msg)
	{
		s_printer.DebugWB("Sending replicated message " + msg.Repr());

		if (!Replication.IsServer())
			return;

		Rpc(Rpc_BroadcastAnalyticsMessage, msg);
	}

	//------------------------------------------------------------------------------------------------
	void LogAndReplicate(notnull BV_AnalyticsEventBase backendEvent, BV_ReplicatedAnalyticsMessage replicatedMsg = null)
	{
		// Backend
		QueueEvent(backendEvent);
		OnAnalyticsEvent(backendEvent);

		// Replication
		if (replicatedMsg)
			ReplicateToClients(replicatedMsg);
	}
}
