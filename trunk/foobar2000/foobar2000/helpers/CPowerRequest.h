#if (_WIN32_WINNT < _WIN32_WINNT_WIN7) && !defined(_MINWINBASE)

typedef struct _REASON_CONTEXT {
    ULONG Version;
    DWORD Flags;
    union {
        struct {
            HMODULE LocalizedReasonModule;
            ULONG LocalizedReasonId;
            ULONG ReasonStringCount;
            LPWSTR *ReasonStrings;

        } Detailed;

        LPWSTR SimpleReasonString;
    } Reason;
} REASON_CONTEXT, *PREASON_CONTEXT;

//
// Power Request APIs
//

typedef REASON_CONTEXT POWER_REQUEST_CONTEXT, *PPOWER_REQUEST_CONTEXT, *LPPOWER_REQUEST_CONTEXT;

WINBASEAPI
HANDLE
WINAPI
PowerCreateRequest (
    __in PREASON_CONTEXT Context
    );

WINBASEAPI
BOOL
WINAPI
PowerSetRequest (
    __in HANDLE PowerRequest,
    __in POWER_REQUEST_TYPE RequestType
    );

WINBASEAPI
BOOL
WINAPI
PowerClearRequest (
    __in HANDLE PowerRequest,
    __in POWER_REQUEST_TYPE RequestType
    );


#endif

typedef HANDLE (WINAPI * pPowerCreateRequest_t) (
    __in PREASON_CONTEXT Context
    );

typedef BOOL (WINAPI * pPowerSetRequest_t) (
    __in HANDLE PowerRequest,
    __in POWER_REQUEST_TYPE RequestType
    );

typedef BOOL (WINAPI * pPowerClearRequest_t) (
    __in HANDLE PowerRequest,
    __in POWER_REQUEST_TYPE RequestType
    );

class CPowerRequestAPI {
public:
	CPowerRequestAPI() : PowerCreateRequest(), PowerSetRequest(), PowerClearRequest()  {
		Bind();
	}
	bool Bind() {
		HMODULE kernel32 = GetModuleHandle(_T("kernel32.dll"));
		return Bind(PowerCreateRequest, kernel32, "PowerCreateRequest")
			&& Bind(PowerSetRequest, kernel32, "PowerSetRequest") 
			&& Bind(PowerClearRequest, kernel32, "PowerClearRequest") ;
	}
	bool IsValid() {return PowerCreateRequest != NULL;}

	void ToggleSystem(HANDLE hRequest, bool bSystem) {
		Toggle(hRequest, bSystem, PowerRequestSystemRequired);
	}

	void ToggleExecution(HANDLE hRequest, bool bSystem) {
		const POWER_REQUEST_TYPE _PowerRequestExecutionRequired = (POWER_REQUEST_TYPE)3;
		const POWER_REQUEST_TYPE RequestType = IsWin8() ? _PowerRequestExecutionRequired : PowerRequestSystemRequired;
		Toggle(hRequest, bSystem, RequestType);
	}

	void ToggleDisplay(HANDLE hRequest, bool bDisplay) {
		Toggle(hRequest, bDisplay, PowerRequestDisplayRequired);
	}

	void Toggle(HANDLE hRequest, bool bToggle, POWER_REQUEST_TYPE what) {
		if (bToggle) {
			PowerSetRequest(hRequest, what);
		} else {
			PowerClearRequest(hRequest, what);
		}

	}

	static bool IsWin8() {
		auto ver = myGetOSVersion();
		return ver >= 0x602;
	}
	static WORD myGetOSVersion() {
		const DWORD ver = GetVersion();
		return (WORD)HIBYTE(LOWORD(ver)) | ((WORD)LOBYTE(LOWORD(ver)) << 8);
	}

	pPowerCreateRequest_t PowerCreateRequest;
	pPowerSetRequest_t PowerSetRequest;
	pPowerClearRequest_t PowerClearRequest;
private:
	template<typename func_t> static bool Bind(func_t & f, HMODULE dll, const char * name) {
		f = reinterpret_cast<func_t>(GetProcAddress(dll, name));
		return f != NULL;
	}
};

class CPowerRequest {
public:
	CPowerRequest(const wchar_t * Reason) : m_Request(INVALID_HANDLE_VALUE), m_bSystem(), m_bDisplay() {
		HMODULE kernel32 = GetModuleHandle(_T("kernel32.dll"));
		if (m_API.IsValid()) {
			REASON_CONTEXT ctx = {POWER_REQUEST_CONTEXT_VERSION, POWER_REQUEST_CONTEXT_SIMPLE_STRING};
			ctx.Reason.SimpleReasonString = const_cast<wchar_t*>(Reason);
			m_Request = m_API.PowerCreateRequest(&ctx);
		}
	}

	void SetSystem(bool bSystem) {
		if (bSystem == m_bSystem) return;
		m_bSystem = bSystem;
		if (m_Request != INVALID_HANDLE_VALUE) {
			m_API.ToggleSystem( m_Request, bSystem );
		} else {
			_UpdateTES();
		}
	}

	void SetExecution(bool bExecution) {
		if (bExecution == m_bSystem) return;
		m_bSystem = bExecution;
		if (m_Request != INVALID_HANDLE_VALUE) {
			m_API.ToggleExecution( m_Request, bExecution );
		} else {
			_UpdateTES();
		}
	}
	
	void SetDisplay(bool bDisplay) {
		if (bDisplay == m_bDisplay) return;
		m_bDisplay = bDisplay;
		if (m_Request != INVALID_HANDLE_VALUE) {
			m_API.ToggleDisplay(m_Request, bDisplay);
		} else {
			_UpdateTES();
		}
	}

	~CPowerRequest() {
		if (m_Request != INVALID_HANDLE_VALUE) {
			CloseHandle(m_Request);
		} else {
			if (m_bDisplay || m_bSystem) SetThreadExecutionState(ES_CONTINUOUS);
		}
	}

private:
	void _UpdateTES() {
		SetThreadExecutionState(ES_CONTINUOUS | (m_bSystem ? ES_SYSTEM_REQUIRED : 0 ) | (m_bDisplay ? ES_DISPLAY_REQUIRED : 0) );
	}
	HANDLE m_Request;
	bool m_bSystem, m_bDisplay;
	CPowerRequestAPI m_API;
	CPowerRequest(const CPowerRequest&);
	void operator=(const CPowerRequest&);
};