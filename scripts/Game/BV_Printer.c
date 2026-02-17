ref BV_FmtPrinter g_BVPrinter = new BV_FmtPrinter();

class BV_FmtPrinter : Managed
{

	#ifdef WORKBENCH
	static bool m_bIsWorkbench = true;
	#else
	static bool m_bIsWorkbench = false;
	#endif

	protected string m_sNamespace = "BVNet  ";

	protected static BV_FmtPrinter m_sInstance;

	//------------------------------------------------------------------------------------------------
	void BV_FmtPrinter(string namespace = string.Empty)
	{
		if (namespace)
		{
			namespace = namespace + "  ";
			m_sNamespace = namespace;
		}

		m_sInstance = this;
	}

	//------------------------------------------------------------------------------------------------
	static BV_FmtPrinter Get()
	{
		return m_sInstance;
	}

	//------------------------------------------------------------------------------------------------
	protected void Print(string msg, string levelFmt = string.Empty, LogLevel level = LogLevel.NORMAL)
	{
		PrintFormat("%1 %2 %3", m_sNamespace, levelFmt, msg, level: level);
	}

	//------------------------------------------------------------------------------------------------
	void Info(string fmt)
	{
		this.Print(fmt);
	}

	//------------------------------------------------------------------------------------------------
	void Error(string fmt)
	{
		this.Print(fmt, level: LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	void Debug(string fmt)
	{
		this.Print(fmt, level: LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	void Trace(string fmt)
	{
		this.Print(fmt, level: LogLevel.SPAM);
	}

	//------------------------------------------------------------------------------------------------
	void InfoWB(string fmt)
	{
		if (!BV_FmtPrinter.m_bIsWorkbench)
			return;

		Info(fmt);
	}

	//------------------------------------------------------------------------------------------------
	void ErrorWB(string fmt)
	{
		if (!BV_FmtPrinter.m_bIsWorkbench)
			return;

		Error(fmt);
	}

	//------------------------------------------------------------------------------------------------
	void DebugWB(string fmt)
	{
		if (!BV_FmtPrinter.m_bIsWorkbench)
			return;

		this.Debug(fmt);
	}

	//------------------------------------------------------------------------------------------------
	void TraceWB(string fmt)
	{
		if (!BV_FmtPrinter.m_bIsWorkbench)
			return;

		Trace(fmt);
	}
}
