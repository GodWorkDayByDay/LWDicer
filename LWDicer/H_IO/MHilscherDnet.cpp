/* Generated by Together */
/* written by jsMoon on 2004.03.17 */

#include "stdafx.h"
#include "assert.h"
#include "MHilscherDnet.h"
#include <atlconv.h>

/** Default 생성자 */
MHilscherDnet::MHilscherDnet()
:m_iObjectID(0), m_iErrorBase(0), m_plogMng(NULL)
{
	m_usBoardNumber = 0;
	m_BDeviceOpened = FALSE;
}

/** 생성자
 * @param iObjectID : Component의 Object ID
 * @param iErrorBase : Error 시작 Offset
 * @param strFullFileName : 로그 파일 이름 및 Path
 * @param ucLevel : 로그 수준
	                DEF_MLOG_NONE_LOG_LEVEL    : 0x00;	// Log 안 함
					DEF_MLOG_ERROR_LOG_LEVEL   : 0x01;	// Error관련 Log
					DEF_MLOG_WARNING_LOG_LEVEL : 0x02;	// Warning 관련 Log
					DEF_MLOG_NORMAL_LOG_LEVEL  : 0x04;	// 정상 동작 관련 Log
  * @param iDays : 로그 파일 보관 기간
  */
MHilscherDnet::MHilscherDnet(
		int				iObjectID, 
		int				iErrorBase, 
		CString			strFullFileName, 
		BYTE			ucLevel, 
		int				iDays, 
		unsigned short	usBoardNumber
	)
: m_iObjectID(iObjectID), m_iErrorBase(iErrorBase)
{
	CString strLogMsg;

	m_usBoardNumber = usBoardNumber;
	m_BDeviceOpened = FALSE;

	// log file manager
	m_plogMng = new MLog;
	m_plogMng->SetLogAttribute(iObjectID, strFullFileName, ucLevel, iDays);

	strLogMsg.Format("MHilscherDnet() : ObjectID=%d OK", iObjectID);
	m_plogMng->WriteLog(DEF_MLOG_NORMAL_LOG_LEVEL, strLogMsg, __FILE__, __LINE__);
}

MHilscherDnet::MHilscherDnet(unsigned short usBoardNumber)
:m_iObjectID(0), m_iErrorBase(0), m_plogMng(NULL)
{
	m_usBoardNumber = usBoardNumber;
	m_BDeviceOpened = FALSE;
}

MHilscherDnet::~MHilscherDnet()
{
	if(m_plogMng) delete m_plogMng;
}

int MHilscherDnet::Initialize()
{
	int i = 0;
	/* Initialize two buffers */
    for( i = 0;i<(MAX_DEVICE*4);i++)
    {
        m_ucOutgoingBuffer[i] = 0;
        m_ucIncomingBuffer[i] = 0;
    }
 
	memset((char *)&m_tblDnStatus,0x0,sizeof(m_tblDnStatus));	// Devicenet 상태 영역 초기화

#ifdef REAL_DEVICE_IO
	if (DevOpenDriver(0) != DRV_NO_ERROR)		// Open a driver
    {
        return generateErrorCode(ERR_IO_DRIVER_FAIL);
    }
    else if (DevInitBoard(m_usBoardNumber,NULL) != DRV_NO_ERROR) 
	{ // Initialize a board
        return generateErrorCode(ERR_IO_DRIVER_FAIL);
	}
    else if (DevSetHostState(m_usBoardNumber,HOST_READY,0L) != DRV_NO_ERROR)
	{ // Inform the board that the application is running
        return generateErrorCode(ERR_IO_DRIVER_FAIL);
	}
#endif
	m_BDeviceOpened = TRUE;
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::Terminate()
{
	m_BDeviceOpened = FALSE;
#ifdef REAL_DEVICE_IO

    if (DevSetHostState(0,HOST_NOT_READY,0L)!= DRV_NO_ERROR) {
	        return generateErrorCode(ERR_IO_DRIVER_FAIL);
    }

    if (DevExitBoard(0) != DRV_NO_ERROR) {
        return generateErrorCode(ERR_IO_DRIVER_FAIL);
    }

    if (DevCloseDriver(0) !=DRV_NO_ERROR) {
        return generateErrorCode(ERR_IO_DRIVER_FAIL);
    }
#endif	
	return ERR_IO_SUCCESS;
}

/**
 * I/O Device의 Digital Status (Bit) 를  읽어드린다.
 * @precondition 이 함수를 실행하기 전에 initialize 함수가 미리 실행되었어야 한다.
 * @param usIOAddr : IO Address
 * @param pbVal    : IO 값
 * @return  0      : SUCCESS
	        else   : Device \Error 코드 
 */
int MHilscherDnet::GetBit(unsigned short usIOAddr, BOOL *pbVal)
{
	UINT rel; 
	int port, bitnum;
	unsigned short usOffset;

	ASSERT(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);
	
	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;


	if(usIOAddr < OUTPUT_ORIGIN)
	{	// get a bit from the incoming buffer
		usOffset = usIOAddr - INPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;
		rel = (UINT)m_ucIncomingBuffer[port];
		rel = rel & 0x000000FF;
	}
	else { // get a bit from the outgoing buffer
		usOffset = usIOAddr - OUTPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;
		rel = (UINT)m_ucOutgoingBuffer[port];
		rel = rel & 0x000000FF;
	}
	
	if( rel & bitOnMask[bitnum] )
		*pbVal = TRUE;
	else
		*pbVal = FALSE;

	return ERR_IO_SUCCESS;

}


/**
 * I/O Device의 Digital Status (Bit) 를 읽어들여 bit 값을 아규먼트로 리턴한다.
 * @precondition 이 함수를 실행하기 전에 initialize 함수가 미리 실행되었어야 한다.
 * @param usIOAddr : IO Address
 * @param pbVal    : TRUE : 값이 1 임, FALSE : 값이 0 임
 * @return  0      : SUCCESS
	        else   : Device Error 코드 
 */
int MHilscherDnet::IsOn(unsigned short usIOAddr, BOOL *pbVal)
{
	return GetBit(usIOAddr,pbVal);
}

/**
 * I/O Device의 Digital Status (Bit) 를 읽어들여 bit 값을 아규먼트로 리턴한다.
 * @precondition 이 함수를 실행하기 전에 initialize 함수가 미리 실행되었어야 한다.
 * @param usIOAddr : IO Address
 * @param pbVal    : TRUE : 값이 0 임, FALSE : 값이 1 임
 * @return  0      : SUCCESS
	        else   : Device Error 코드 
 */
int MHilscherDnet::IsOff(unsigned short usIOAddr, BOOL *pbVal)
{
	BOOL	bVal;
	int		iRet;
	
	iRet = GetBit(usIOAddr,&bVal);
	if(iRet != ERR_IO_SUCCESS) return iRet;

	*pbVal = !bVal;

	return ERR_IO_SUCCESS;
}

int MHilscherDnet::OutputOn(unsigned short usIOAddr)
{
	BYTE data;
	int port, bitnum;
	unsigned short usOffset;

	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	if(usIOAddr < OUTPUT_ORIGIN)
	{
		return ERROR_ID;	// Don't touch the incoming buffer
	}
	else {
		usOffset = usIOAddr - OUTPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;
	}
	
	data = m_ucOutgoingBuffer[port];
	data = data | bitOnMask[bitnum];
	
	m_ucOutgoingBuffer[port] = data;

	return ERR_IO_SUCCESS;
}

int MHilscherDnet::OutputOff(unsigned short usIOAddr)
{
	BYTE data;
	int port, bitnum;
	unsigned short usOffset;

	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	if(usIOAddr < OUTPUT_ORIGIN)
	{
		return ERROR_ID;	// Don't touch the incoming buffer
	}
	else {
		usOffset = usIOAddr - OUTPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;
	}
	
	data = m_ucOutgoingBuffer[port];
	data = data & bitOffMask[bitnum];
	
	m_ucOutgoingBuffer[port] = data;
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::OutputToggle(unsigned short usIOAddr)
{
	BYTE data;
	int port, bitnum;
	unsigned short usOffset;
	BOOL	bVal;
	int		iRet;

	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	if(usIOAddr < OUTPUT_ORIGIN)
	{
		return ERROR_ID;
	}
	else {
		usOffset = usIOAddr - OUTPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;
	}
	
	iRet = GetBit(usIOAddr,&bVal);
	if(iRet != ERR_IO_SUCCESS) return iRet;

	if(bVal == 1) {
		data = m_ucOutgoingBuffer[port];
		data = data & bitOffMask[bitnum];
	}
	else{
		data = m_ucOutgoingBuffer[port];
		data = data | bitOnMask[bitnum];
	}
	
	m_ucOutgoingBuffer[port] = data;
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::GetByte(unsigned short usIOAddr, BYTE & pcValue)
{
	BYTE lowdata, highdata; 
	int port, bitnum;
	unsigned short usOffset;
	
	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	if(usIOAddr < OUTPUT_ORIGIN)	{	// get a byte from the incoming buffer
		usOffset = usIOAddr - INPUT_ORIGIN;	
		port = usOffset / 8;
		bitnum = usOffset % 8;	
		if(bitnum == 0) {
			pcValue = m_ucIncomingBuffer[port];}
		else{
			lowdata = m_ucIncomingBuffer[port];
			highdata = m_ucIncomingBuffer[port+1];

			lowdata = (lowdata >> bitnum);
			highdata = (highdata << (8-bitnum));
			pcValue = lowdata | highdata;
			}
	}
	else { // get a byte from the outgoing buffer
		usOffset = usIOAddr - OUTPUT_ORIGIN;
		port = usOffset / 8;
		bitnum = usOffset % 8;	
		if(bitnum == 0)	{
			pcValue = m_ucOutgoingBuffer[port];}
		else{
			lowdata = m_ucOutgoingBuffer[port];
			highdata = m_ucOutgoingBuffer[port+1];

			lowdata = (lowdata >> bitnum);
			highdata = (highdata << (8-bitnum));
			pcValue = lowdata | highdata;
		}
	}
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::PutByte(unsigned short usIOAddr, BYTE pcValue)
{
	BYTE data, lowdata, highdata; 
	int port, bitnum; 
	unsigned short usOffset;

	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	data = pcValue;
	
	if(usIOAddr < OUTPUT_ORIGIN)	{
		return ERROR_ID;
	}
	
	usOffset = usIOAddr - OUTPUT_ORIGIN;
	port = usOffset / 8;
	bitnum = usOffset % 8;	

	if(bitnum == 0)	{
		m_ucOutgoingBuffer[port] = data;}
	else{
		lowdata = m_ucOutgoingBuffer[port];
		lowdata = (lowdata & highbitsMask[bitnum]) | (data << bitnum);
		m_ucOutgoingBuffer[port] = lowdata;

		highdata = m_ucOutgoingBuffer[port+1];
		highdata = (highdata & lowbitsMask[bitnum-1]) | (data >> (8-bitnum));
		m_ucOutgoingBuffer[port+1] = highdata;
	}
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::GetWord(unsigned short usIOAddr, WORD & pwValue)
{
	WORD lowdata, middledata, highdata; 
	int port, bitnum;
	unsigned short usOffset;
	
	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	if(usIOAddr < OUTPUT_ORIGIN)	{ // get a word from the incoming buffer
		usOffset = usIOAddr - INPUT_ORIGIN;	
		port = usOffset / 8;
		bitnum = usOffset % 8;	
		if(bitnum == 0) {
			lowdata = (WORD) m_ucIncomingBuffer[port];
			highdata = (WORD) m_ucIncomingBuffer[port+1];
			
			highdata = highdata << 8;

			pwValue = lowdata | highdata;
		}
		else{ 
			lowdata = (WORD) m_ucIncomingBuffer[port];
			middledata = (WORD) m_ucIncomingBuffer[port+1];
			highdata = (WORD) m_ucIncomingBuffer[port+2];

			lowdata = lowdata >> bitnum;
			middledata = middledata << (8-bitnum);
			highdata = highdata << (16-bitnum);
			pwValue = lowdata | middledata | highdata;
			}
	}
	else { // get a word from the outgoing buffer
		usOffset = usIOAddr - OUTPUT_ORIGIN;	
		port = usOffset / 8;
		bitnum = usOffset % 8;	
		if(bitnum == 0) {
			lowdata = (WORD) m_ucOutgoingBuffer[port];
			highdata = (WORD) m_ucOutgoingBuffer[port+1];
			highdata = highdata << 8;
			pwValue = lowdata | highdata;
		}
		else{
			lowdata = (WORD) m_ucOutgoingBuffer[port];
			middledata = (WORD) m_ucOutgoingBuffer[port+1];
			highdata = (WORD) m_ucOutgoingBuffer[port+2];

			lowdata = lowdata >> bitnum;
			middledata = middledata << (8-bitnum);
			highdata = highdata << (16-bitnum);
			pwValue = lowdata | middledata | highdata;
			}
	}
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::PutWord(unsigned short usIOAddr, WORD pwValue)
{
	WORD data;
	BYTE lowdata, middledata, highdata, tmpdata; 
	int port, bitnum;
    unsigned short usOffset;
	
	assert(usIOAddr>=INPUT_ORIGIN && usIOAddr<=OUTPUT_END);

	// Check IO Status
	int iRet;
	if( (iRet=returnDnetStatus()) != ERR_IO_SUCCESS) return iRet;

	data = pwValue;
	
	if(usIOAddr < OUTPUT_ORIGIN)	{
		return ERROR_ID;
	}
	else {
		usOffset = usIOAddr - OUTPUT_ORIGIN;	
		port = usOffset / 8;
		bitnum = usOffset % 8;	
		if(bitnum == 0) {
			lowdata = (BYTE) (data & 0x00FF);
			highdata = (BYTE) (data >> 8);
			m_ucOutgoingBuffer[port] = lowdata;
			m_ucOutgoingBuffer[port+1] = highdata;
		}
		else{
			lowdata = (BYTE) ((data << bitnum) & 0x00FF);
			middledata = (BYTE) ((data >> (8-bitnum)) & 0x00FF);
			highdata = (BYTE) ((data >> (16-bitnum)) & 0x00FF);

			tmpdata = m_ucOutgoingBuffer[port];
			lowdata = lowdata | (tmpdata & highbitsMask[bitnum]);
			tmpdata = m_ucOutgoingBuffer[port+2];
			highdata = highdata | (tmpdata & lowbitsMask[bitnum-1]);

			m_ucOutgoingBuffer[port] = lowdata;
			m_ucOutgoingBuffer[port+1] = middledata;
			m_ucOutgoingBuffer[port+2] = highdata;
		}
	}
	return ERR_IO_SUCCESS;
}

int MHilscherDnet::IOAddrInterpreter(CString strIOAddr, unsigned short & usIOAddr)
{
	CString strTmp;
	int cnt;
	
	cnt = strIOAddr.Find(':');
	if (cnt == -1) return ERROR_ID;
	
	strTmp = strIOAddr.Left(cnt);
	usIOAddr = (unsigned short) atoi(strTmp);

	return ERR_IO_SUCCESS;
}

int MHilscherDnet::GetBit(CString strIOAddr,BOOL *pbVal)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return GetBit(usIOAddr,pbVal);
}

BOOL MHilscherDnet::IsOn(CString strIOAddr,BOOL *pbVal)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)
	{
		return ERROR_ID;
	}

	return IsOn(usIOAddr,pbVal);
}

int MHilscherDnet::OutputOn(CString strIOAddr)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return OutputOn(usIOAddr);
}


int MHilscherDnet::OutputOff(CString strIOAddr)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return OutputOff(usIOAddr);
}


int MHilscherDnet::OutputToggle(CString strIOAddr)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return OutputToggle(usIOAddr);
}


int MHilscherDnet::GetByte(CString strIOAddr, BYTE & pcValue)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return GetByte(usIOAddr, pcValue);
}

int MHilscherDnet::PutByte(CString strIOAddr, BYTE pcValue)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return PutByte(usIOAddr, pcValue);
}


int MHilscherDnet::GetWord(CString strIOAddr, WORD & pwValue)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return GetWord(usIOAddr, pwValue);
}


int MHilscherDnet::PutWord(CString strIOAddr, WORD pwValue)
{
	unsigned short usIOAddr;

	if(IOAddrInterpreter(strIOAddr, usIOAddr) == ERROR_ID)	{
		return ERROR_ID;
	}

	return PutWord(usIOAddr, pwValue);
}


void MHilscherDnet::RunIOThread()
{
//	UINT	sRet;
	ULONG	StartTime, EndTime;
	int		iRet;
	BOOL	bVal;
	double	ScanTime;
	LPDWORD lpthreadID = NULL;	// WIN ThreadID

	StartTime = GetTickCount();
	while(1)
	{
		iRet = GetBit(INPUT_ORIGIN,&bVal); // Must Check
		if( iRet == ERR_IO_SUCCESS ) 	break;

		EndTime = GetTickCount();
		EndTime -= StartTime;
		ScanTime = (double)EndTime / (double)CLK_TCK;

		if( ScanTime >= 10.0 )
		{
//			MyMessageBox(17500, "Device Net Message Reading Error", M_ICONERROR);
						//17500=Device Net의 Message를 Reading하는데 Error가 발생하였습니다.
//			return ERROR_ID;
		}

	}

	Sleep(500);

//	AfxBeginThread(IOThread, this);

	CreateThread(NULL, 0, EntryPoint, (LPVOID)this, 0, lpthreadID );
}

UINT MHilscherDnet::IOThread(LPVOID lParam)
{
	MHilscherDnet * This = (MHilscherDnet *) lParam;
	if( lParam != NULL ) {}


	
#ifdef VIRTUAL_DEVICE_IO
	HRESULT hr;
	hr = ::CoInitialize(NULL);
	if(FAILED(hr)) {
		printf("COM 라이브러리를 초기화할 수 없습니다!");
		return FALSE;
	}

	try {
		m_compDNet = IVirtualDeviceNetPtr(__uuidof(VirtualDeviceNet));
		m_compIIOBus = m_compDNet;
	}
	catch(_com_error& e) {
		ShowMessage(e);
	}
#endif

	while(This->m_BDeviceOpened)
	{
#ifdef REAL_DEVICE_IO
		dnStatusCheck();

		if( DevExchangeIO(This->m_usBoardNumber,
					  0,							/* usSendOffset    */
					  MAX_DEVICE * 4,				/* usSendSize      */
					  &This->m_ucOutgoingBuffer[0],		/* *pvSendData     */
					  0,							/* usReceiveOffset */
					  MAX_DEVICE * 4,				/* usReceiveSize   */
					  &This->m_ucIncomingBuffer[0],	/* *pvReceiveData  */
					  100L) != DRV_NO_ERROR  )		/* ulTimeout       */
		{
			//return ERROR_ID;
		}
		else 
			;//return	ERR_IO_SUCCESS;
#endif


#ifdef VIRTUAL_DEVICE_IO
		VirtualIOExchange();
#endif

		Sleep(10);
	}
	return 0;
}

/**
 * Master 모듈 및 Slave 모듈 상태를 반환한다
 *
 * @param ubMacID : 대상 Slave Mac Address
          -1      : 모든 Slave 체크 (Default)
 * @return 0		= SUCCESS
           others	= ERROR Code
 */
int MHilscherDnet::dnStatusCheck(unsigned char ucMacID)
{
#define IO_STS_OFFSET	8

	int				nSlaveArray;		// Slave 주소 0 ~ 7그룹
	int				nSlaveLocate;		// Slave 주소 그룹당 0 ~ 7개
	unsigned char	cCheckbyte;			// Slave의 Bit 체크 정보	
	unsigned char	cBitMask;			// Bit Mask 
	int				iRet = 0;				// 리턴 Value
	DNM_DIAGNOSTICS	stDnmDiagonstics;


	if(ucMacID ==255) // All check
	{
		// DeviceNet에 제공하는 API 호출
		if(DevGetTaskState(m_usBoardNumber, 2, sizeof(DNM_DIAGNOSTICS), &stDnmDiagonstics)
			 != DRV_NO_ERROR)
		{
			m_tblDnStatus.dnm_status = STS_FAIL;
			return ERR_IO_UPDATE_FAIL;
		}
		else
		{
			m_tblDnStatus.dnm_status = STS_NORMAL;
			for(nSlaveArray=0; nSlaveArray<8; nSlaveArray++)
			{
				if(stDnmDiagonstics.abDv_cfg[nSlaveArray] 
					!= stDnmDiagonstics.abDv_state[IO_STS_OFFSET+nSlaveArray])
				{
					cCheckbyte = (stDnmDiagonstics.abDv_cfg[nSlaveArray] 
									^ stDnmDiagonstics.abDv_state[IO_STS_OFFSET+nSlaveArray]);
					for(nSlaveLocate=0; nSlaveLocate<8; nSlaveLocate++)
					{
						cBitMask	= 0x1 << (nSlaveLocate);
						if(cCheckbyte & cBitMask)	// 해당 비트가 에러임
						{
							m_tblDnStatus.dns_status[nSlaveArray*8+nSlaveLocate] = STS_FAIL;
							iRet = ERR_IO_MODULE_FAIL;
						}
						else
						{
							m_tblDnStatus.dns_status[nSlaveArray*8+nSlaveLocate] = STS_NORMAL;
						}
					}
				}
				else
				{
					for(nSlaveLocate=0; nSlaveLocate<8; nSlaveLocate++)
					{
						m_tblDnStatus.dns_status[nSlaveArray*8+nSlaveLocate] = STS_NORMAL;
					}
				}
			}
		}
	}

	else // MacID check
	{
		nSlaveArray = ucMacID/8;
		nSlaveLocate = ucMacID%8;
		
		if(DevGetTaskState(m_usBoardNumber, 2, sizeof(DNM_DIAGNOSTICS), &stDnmDiagonstics)
			!= DRV_NO_ERROR)
		{	
			m_tblDnStatus.dnm_status = STS_FAIL;
			return ERR_IO_UPDATE_FAIL;
		}
		else if(stDnmDiagonstics.abDv_cfg[nSlaveArray] 
					!= stDnmDiagonstics.abDv_state[IO_STS_OFFSET+nSlaveArray])
		{
			cBitMask	= 0x1 << (nSlaveLocate);
			cCheckbyte	= (stDnmDiagonstics.abDv_cfg[nSlaveArray] 
									^ stDnmDiagonstics.abDv_state[IO_STS_OFFSET+nSlaveArray]);
			if(cCheckbyte & cBitMask)
				iRet = ERR_IO_MODULE_FAIL;
		}
		m_tblDnStatus.dnm_status = STS_NORMAL;
	}
	return iRet;
}

/**
 * Master 모듈 및 Slave 모듈 상태 정보를 얻어온다.
 *
 * @param  DnStatus : 마스터와 64개의 Slave에 대한 상태 정보 구조체
 * @return 0		= 모두 정상
           others	= 하나라도 실패
 */
int MHilscherDnet::DnStatusGet(DN_STATUS DnStatus)
{
	int			nSlave;
	int			iRet = 0;

	memcpy(&DnStatus,&m_tblDnStatus,sizeof(m_tblDnStatus));
	if(m_tblDnStatus.dnm_status == STS_FAIL)
	{
		return generateErrorCode(ERR_IO_UPDATE_FAIL);
	}
	else
	{
		for(nSlave=0;nSlave<MAX_DEVICE;nSlave++)
		{
			if(m_tblDnStatus.dns_status[nSlave] == STS_FAIL)
				return generateErrorCode(ERR_IO_MODULE_FAIL);
		}
	}

	return ERR_IO_SUCCESS;
}

DWORD WINAPI MHilscherDnet::EntryPoint(LPVOID pParam)
{
	MHilscherDnet *pSelf = (MHilscherDnet*)pParam;
	return pSelf->IOThread(pParam);
}

/**
 * Master 모듈 및 Slave 모듈 상태 체크
 *
 * @return 0		= SUCCESS
		   others	= ERROR Code
 */
int MHilscherDnet::returnDnetStatus()
{
#if FALSE		// 2004.9.3 Slave Module 상태 확인하면서 오류 발생하여 일단 막음. (PCB 장비 동일 구성 상의 문제점)
	int			nSlave;
#endif
	int			iRet = 0;

	if(m_tblDnStatus.dnm_status == STS_FAIL)
	{
		return generateErrorCode(ERR_IO_UPDATE_FAIL);
	}
#if FALSE		// 2004.9.3 Slave Module 상태 확인하면서 오류 발생하여 일단 막음. (PCB 장비 동일 구성 상의 문제점)
	else
	{
		for(nSlave=0;nSlave<MAX_DEVICE;nSlave++)
		{
			if(m_tblDnStatus.dns_status[nSlave] == STS_FAIL)
				return generateErrorCode(ERR_IO_MODULE_FAIL);
		}
	}
#endif

	return ERR_IO_SUCCESS;
}

/***************** Common Implementation *************************************/

/**
  * Error Code Base를 설정한다. 
  *
  * @param	iErrorBase : (OPTION=0) 설정할 Error Base 값
  */
void MHilscherDnet::SetErrorBase(int iErrorBase)
{
	m_iErrorBase = iErrorBase;
}

/**
  * Error Code Base를 읽는다. 
  *
  * @return	int : Error Base 값
  */
int MHilscherDnet::GetErrorBase(void) const
{
	return m_iErrorBase;
}


/**
  * Object ID를 설정한다. 
  *
  * @param	iObjectID : 설정할 Object ID 값
  */
void MHilscherDnet::SetObjectID(int iObjectID)
{
	m_iObjectID = iObjectID;
}

/**
  * Object ID를 읽는다. 
  *
  * @return	int : Object ID 값
  */
int MHilscherDnet::GetObjectID(void)
{
	return m_iObjectID;
}

/** 
 * Log Class의 개체 Pointer를 설정한다.
 *
 * @param		*pLogObj: 연결할 Log Class의 개체 Pointer
 * @return		Error Code : 0 = Success, 그 외 = Error
 */
int MHilscherDnet::SetLogObject(MLog *pLogObj)
{
	if(m_plogMng != NULL)
	{
		delete m_plogMng;
		m_plogMng = NULL;
	}
	m_plogMng = pLogObj;
	return ERR_IO_SUCCESS;
}

/**
  * Reel Supplier Component의 Log File 정보를 set한다.
  *
  * @param	iObjectID		: Object ID
  * @param	strFilePath		: log file의 Full Path를 지정한다.
  * @param	ucLogLevel		: bitwise형태의 log level을 설정한다.
  * @param	iDays			: log file을 저장하는 기간
  * @return	Error Code		: 0 = Success
  *							  그외 = Error
  */
int MHilscherDnet::SetLogAttribute(int iObjectID, CString strFullFileName, BYTE ucLevel, int iDays)
{
	int iErrorCode;
	
	if( m_plogMng == NULL )
	{
		iErrorCode = generateErrorCode(ERR_IO_NULL_DATA);
		return iErrorCode;
	}
	m_plogMng->SetLogAttribute(m_iObjectID, strFullFileName, ucLevel, iDays );
	return ERR_IO_SUCCESS;
}

/**
 * 오래된 Log file을 삭제한다.
 *
 * @return	Error Code : 0 = Success, 그 외 = Error
 */
int MHilscherDnet::DeleteOldLogFiles(void)
{
	int iErrorCode;

	if( m_plogMng == NULL )
	{
		iErrorCode = generateErrorCode(ERR_IO_NULL_DATA);
		return iErrorCode;
	}
	m_plogMng->DeleteOldLogFiles();
	return ERR_IO_SUCCESS;
}

MLog* MHilscherDnet::GetLogManager()
{
	return m_plogMng;
}

/** 
 * Component의 Error Code Base를 반환한다.
 *
 * @param		Error Code: ObjectID + Error Base 
 * @return		ErrorBase가 제거된 Component Error Code 
 */
int MHilscherDnet::DecodeError(int iErrCode)
{
#ifndef	NO_ERROR_ENCODING
	return ( (iErrCode & 0xFF) - m_iErrorBase );
#else
	return iErrCode;
#endif
}

/** Internal Operation */
/**
  * Error Code 생성하기
  *  +-----------+-------------------------+
  *  | Object ID | Error Code + Error Base |
  *  | (2 bytes) |        (2 bytes)        |
  *  +-----------+-------------------------+
  *
  * @param	iErrCode : 발생한 Error Code
  * @return	Error Code : Object ID (2bytes)와 Error Code + Error Base (2bytes)를 4bytes로 조합한 코드
  */
int MHilscherDnet::generateErrorCode(int iErrCode)
{
#ifndef	NO_ERROR_ENCODING
	int	iError = ERR_IO_SUCCESS;

	// Error Code가 SUCCESS가 아니면 코드 생성
	if (iErrCode != ERR_IO_SUCCESS)
		iError = (m_iObjectID << 16) + (iErrCode + m_iErrorBase);
	// Error Code가 SUCCESS이면 SUCCESS return
	else
		iError = ERR_IO_SUCCESS;

	return iError;
#else
	return iErrCode;
#endif
}


#ifdef VIRTUAL_DEVICE_IO

void MHilscherDnet::VirtualIOExchange()
{
	int i = 0;
	VARIANT OutGoingBuffer;
	VariantInit(&OutGoingBuffer);
	VARIANT IncomminBuffer;
	VariantInit(&IncomminBuffer);
	VARIANT vDest;
	VariantInit(&vDest);
		
	for( i = 0; i < MAX_DEVICE * 4; i++)
	{
		
		OutGoingBuffer.vt = VT_UI1;
		OutGoingBuffer.bVal = this->m_ucOutgoingBuffer[i]; 

		try
		{
			m_compIIOBus->SetOutputByte(i, OutGoingBuffer);	
		}
		catch(_com_error& e) 
		{
			ShowMessage(e);
		}

		try
		{
			m_compIIOBus->GetInputByte(i, &IncomminBuffer);	
		}
		catch(_com_error& e) 
		{
			ShowMessage(e);
		}

		HRESULT hr = VariantChangeType(&vDest, &IncomminBuffer, 0, VT_UI1);
		if(SUCCEEDED(hr))
		{
			this->m_ucIncomingBuffer[i] = vDest.bVal;
		}
	}


}

void MHilscherDnet::ShowMessage(_com_error &e)
{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	CString szMsg = "에러가 발생했습니다!\n";
	CString szTemp;
		
	szTemp.Format("에러 코드 : %081x\n", e.Error());
	szMsg += szTemp;
	szTemp.Format("에러 내용 : %s\n", e.ErrorMessage());
	szMsg += szTemp;
	szTemp.Format("에러 소스 : %s\n",
		bstrSource.length() ? (LPCTSTR)bstrSource : _T("없음"));
	szMsg += szTemp;
	szTemp.Format("에러 설명 : %s\n",
	bstrDescription.length() ? (LPCTSTR)bstrDescription : _T("없음"));
	szMsg += szTemp;

	AfxMessageBox(szMsg);
}
#endif