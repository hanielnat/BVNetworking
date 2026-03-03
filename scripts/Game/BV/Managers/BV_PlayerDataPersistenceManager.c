class BV_PlayerDataPersistenceManager : Managed
{
	private static ref BV_PlayerDataPersistenceManager s_instance;
	protected ref EDF_DbContext m_dbContext;

	//------------------------------------------------------------------------------------------------
	static BV_PlayerDataPersistenceManager GetInstance()
	{
		if (!s_instance)
			s_instance = new BV_PlayerDataPersistenceManager();

		return s_instance;
	}

	//------------------------------------------------------------------------------------------------
	static EDF_JsonFileDbConnectionInfo GetDbConnection()
	{
		EDF_JsonFileDbConnectionInfo conn = new EDF_JsonFileDbConnectionInfo();
		conn.m_sDatabaseName = "BVPlayerData";
		conn.m_bUseCache = true;

		#ifdef WORKBENCH
		conn.m_bPrettify = true;
		#endif

		return conn;
	}

	//------------------------------------------------------------------------------------------------
	EDF_DbContext GetDbContext(EDF_JsonFileDbConnectionInfo conn)
	{
		if (!m_dbContext)
			m_dbContext = EDF_DbContext.Create(conn);

		return m_dbContext;
	}

	//------------------------------------------------------------------------------------------------
	EDF_DbRepository<BV_PlayerDataEntity> GetDbRepository(EDF_JsonFileDbConnectionInfo conn)
	{
		return EDF_DbEntityHelper<BV_PlayerDataEntity>.GetRepository(GetDbContext(conn));
	}

	//------------------------------------------------------------------------------------------------
	bool SaveEntity(notnull BV_PlayerDataEntity pData)
	{
		EDF_JsonFileDbConnectionInfo conn = GetDbConnection();

		EDF_DbRepository<BV_PlayerDataEntity> repository = GetDbRepository(conn);
		if (!repository)
			return false;

		string playerUID = String(pData.id.playerUID);
		pData.SetId(playerUID);

		EDF_EDbOperationStatusCode status = repository.AddOrUpdate(pData);
		if (status != EDF_EDbOperationStatusCode.SUCCESS)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool LoadEntity(UUID playerUID, out notnull BV_PlayerDataEntity pData)
	{
		if (playerUID.IsNull())
			return false;

		string sPlayerUID = String(playerUID);

		EDF_JsonFileDbConnectionInfo conn = GetDbConnection();

		EDF_DbRepository<BV_PlayerDataEntity> repository = GetDbRepository(conn);
		if (!repository)
			return false;

		EDF_DbFindResultSingle<BV_PlayerDataEntity> result = repository.Find(sPlayerUID);
		if (!result.IsSuccess())
			return false;

		BV_PlayerDataEntity tempData = result.GetEntity();
		if (!tempData)
			return false;

		pData = tempData;
		return true;
	}
}
