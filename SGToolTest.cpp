// SGTool.cpp : main source file for SGTool.exe
//

#define   _WINSOCKAPI_  //防止网络接口重定义
#include <windows.h>
#include <TCHAR.H>
#include "MainEntry.h"
#include "MiniUp/miniupstdafx.h"
#include "ImeUi/InitImeUi_All.cpp"
#include "ImeTray/ImeTray.h"
//#include "Config/t_FuzzySetMain.h"
//#include "QuickInput/QuickInput.h"
#include "UserPage/UserPage.h"
#include "SkinReg/SkinReg.h"
#include "ScdReg/ScdRegEx.h"
#include "ScdMaker/ScdMakerMain.h"
#include "LogoutWebMode/LogoutWebMode.h"
#include "ImeRepairStub/ImeRepairUser.cpp"
#include "InstallDrv/InstallDrvMain.h"
#include "CrushReport/t_exceptionHandler.h"
#include "PinyinupEx/t_DebugLog.h"
#include "WizardEx/WizardExMain.h"
#include "ConfigEx/ConfigExMain.h"
#include "MedalWall/MedalWallMain.h"
#include "PrivilegeMakerMain.h"
#include "Eudc/EudcMain.h"
#include "UnCommonWordFont/UnCommonWordFontMain.h"
#include "IpcUtil/ProcessCommon.h"
#include "UniqueSgtool.h"
#include "SgImeRepairer/SgImeRepairer.h"
#include "SetLoginState/SetLoginMain.h"
#include "UpRecommend/UpRecommendMain.h"
#include "UpCellDict/UpCellDict.h"

#include "remoteDictToolMain.h"

#include "ShowTip/ShowTip.h"
#include "DelSkin/SkinDeleteMain.h"
#include "CompanyBox/CompanyBoxMain.h"

#include "Common/t_runtime.h"
#include "PinyinRepair/imerepair_util.h"
#include "Common/t_debug.h"


#include "Individual/IndividualCfgMain.h"
#include "Common/VerifySignature.h"
#include "PinyinUpFuncEx/WorkThread.h"
#include "UserPage/UserPage_Utility.h"

#include "Recorder/RecorderMain.h"

static t_exceptionHandler s_exceptionHandler;

// 苏延刚 2014-12-25 添加：用于解决全局变量：_Module的崩溃
t_csAutoInit g_csModuleInit(t_mutex::c_levelAtomProc2, _T("Local\\cs_moduleinit"));
bool g_bModuleInit = false;

// 苏延刚 2014-12-25 添加：用于解决快捷输入词库初始化
t_csAutoInit g_csQuickInputInit(t_mutex::c_levelAtomProc2, _T("Local\\cs_moduleinit"));
bool g_bQuickInputInit = false;

WTL::CAppModule _Module;
HINSTANCE g_hInstance = NULL;

CVerifySignature g_VerifySignature;

static int GetAppID_L( LPCTSTR p_szCmdLine, LPTSTR p_szAppID, int p_ciSize )
{	
	LPCTSTR szStart = _tcsstr( p_szCmdLine, _T("--appid=") );
	if( szStart == NULL )
	{
		return 0;
	}
	szStart += _tcslen( _T("--appid=") );

	LPCTSTR szEnd = _tcsstr(szStart, _T(" "));
	if( szEnd == NULL )
	{
		if( p_ciSize < (int)_tcslen(szStart) )
		{
			return 0;
		}

		_tcscpy_s( p_szAppID, p_ciSize, szStart );
		return _tcslen( szStart );
	}

	int nLen = szEnd - szStart;
	if( nLen >= p_ciSize )
	{
		return 0;
	}

	_tcsncpy_s( p_szAppID, p_ciSize, szStart, nLen );
	return nLen;
}

static bool ExcludeAppidOption(LPTSTR szCmdLine, LPCTSTR szAppidOption, LPTSTR szResult, int ccSize)
{
	int len = (int)_tcslen(szCmdLine);
	if(len >= ccSize)
	{
		return false;
	}

	_tcscpy_s(szResult, ccSize, szCmdLine);
	LPCTSTR szPos = _tcsstr(szCmdLine, szAppidOption);
	if( szPos != NULL )
	{
		int nPos = szPos - szCmdLine;
		int nOptLen = _tcslen(szAppidOption);
		memmove_s(szResult + nPos, (ccSize - nPos) * sizeof(TCHAR), 
			szResult + nPos + nOptLen, (len - nPos - nOptLen) * sizeof(TCHAR));
		szResult[len - nOptLen] = '\0';
	}
	return true;
}

int _InnerMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	TCHAR szAppID[MAX_PATH] = {0};
	if( GetAppID_L(lpstrCmdLine, szAppID, MAX_PATH) <= 0 )
	{
		return ConfigExMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}

	g_hInstance = hInstance;

	if(_tcsicmp(szAppID, _T("sysdicmk")) == 0)
	{
		return SysDicMakerMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("userdict")) == 0)
	{
		return UserDicMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("check")) == 0)
	{
		return CheckMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("install")) == 0)
	{
		return InstallMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("fcpuncture")) == 0)
	{
		return FCPunctureMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("preloader")) == 0)
	{
		return PreloaderMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	/*else if(_tcsicmp(szAppID, _T("speedmeter")) == 0)
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=speedmeter"), szCmdLine, 1024);
		return SpeedMeterExMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}*/
	else if(_tcsicmp(szAppID, _T("errorreport")) == 0)
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=errorreport"), szCmdLine, 1024);
		return ErrorReportMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("wizard")) == 0)
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=wizard"), szCmdLine, 1024);
		return WizardExMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("mergenfa")) == 0 )	// 合并数字调频
	{
		return NumFreqAdjustMerge(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("dictconv")) == 0 )	// 词库兼容转换
	{
		return DictConvertMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	/*else if(_tcsicmp(szAppID, _T("wizardOld")) == 0)
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=wizardOld"), szCmdLine, 1024);
		return WizardMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}*/
	else if(_tcsicmp(szAppID, _T("pinyinrepair")) == 0)
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=pinyinrepair"), szCmdLine, 1024);
		return PinyinRepairMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("miniup")) == 0)
	{
		return MiniUpMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("textconvertor")) == 0)
	{
		//return TextConvertorMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);

		// 苏延刚 2013-10-15 修改 使用快捷键：Ctrl+Shift+U弹出新的搜狗酷字
		TCHAR szCmdStr[100] = {0};
		_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=sogoucool"));	
		return RichInputMain( hInstance, hPrevInstance, szCmdStr, nCmdShow );
	}
	else if(_tcsicmp(szAppID, _T("sohunewserr")) == 0)
	{
		return SohuNewsErr(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if(_tcsicmp(szAppID, _T("xdelta")) == 0)
	{
		// 调用patch工具
		return Xdelta3Main(hInstance,hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("vdigitfunc")) == 0 )
	{
		return ShowDigitFuncMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("imetray")) == 0 )
	{
		return ImeTrayMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("config")) == 0 )
	{
		int nStrCmdLine = _tcslen(lpstrCmdLine) + 1;
		TCHAR* pszCmdLine = new TCHAR[nStrCmdLine];
		memset(pszCmdLine, 0, sizeof(TCHAR) * nStrCmdLine);
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=config"), pszCmdLine, nStrCmdLine);
		int nRet = ConfigExMain(hInstance, hPrevInstance, pszCmdLine, nCmdShow);
		delete []pszCmdLine;
		pszCmdLine = NULL;
		return nRet;
	}
	// 模糊音设置
	else if( _tcsicmp(szAppID, _T("fuzzyset")) == 0 )
	{
		//TCHAR szCmdLine[1024] = {0};
		//ExcludeAppidOption(lpstrCmdLine, _T("--appid=fuzzyset"), szCmdLine, 1024);
		//return FuzzyConfigMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );

		return FuzzySetMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("quickinput")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=quickinput"), szCmdLine, 1024);
		//return QuickInputMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );

		// 苏延刚 2013-12-26 修改 使用快捷键：Ctrl+Shift+M弹出新的搜狗表情
		TCHAR szCmdStr[100] = {0};
		switch ( szCmdLine[1] )
		{
		//case _T('0'):
		//case _T('1'):
		//	{
		//		_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=charexp"));	
		//		break;
		//	}
		case _T('2'):
			{
				_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=totalsym"));	
				break;
			}
		case _T('3'):
			{
#if 0
			_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=charpainting"));
#else
			return 0;
#endif
			break;
			}
		case _T('4'):
			{
#if 0
				_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=datetime"));
#else
				return 0;
#endif
				break;
			}
		default:
			{
				//_tcscpy_s(szCmdStr, 100, _T("--appid=exinput -cid=charexp"));
				//8.4 各个入口改为新版颜文字
				TCHAR szPath[MAX_PATH] = {0};
				::GetModuleFileName(0, szPath, MAX_PATH);
				TCHAR* pszStart = _tcsrchr(szPath, _T('\\'));
				if (pszStart != NULL)
				{
					*pszStart = 0;
				}
				t_path pathKaomoji(szPath);
				pathKaomoji.Append(_T("SGKaomoji.exe"));
				t_error err;
				pathKaomoji.Exec(err, szCmdLine);
				return 0;

				//break;
			}
		}

		return RichInputMain( hInstance, hPrevInstance, szCmdStr, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("userpage")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=userpage"), szCmdLine, 1024);
		return UserPageMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	// fastpassport page
	else if( _tcsicmp(szAppID, _T("fppage")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=fppage"), szCmdLine, 1024);
		return FastpassportPage( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	// 新的账户窗口
	else if( _tcsicmp(szAppID, _T("useraccpage")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=useraccpage"), szCmdLine, 1024);
		return UserAccPage( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	// 同步图片表情
	else if( _tcsicmp(szAppID, _T("syncpicface")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=syncpicface"), szCmdLine, 1024);
		return UserSyncPicface( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	// 导入其他帐号数据
	else if( _tcsicmp(szAppID, _T("importaccdata")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=importaccdata"), szCmdLine, 1024);
		return UserImportAccData( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	// 智能登录提示
	else if( _tcsicmp(szAppID, _T("smartlogontip")) == 0 )
	{
		// @liugao <! 2017-05-27 根据产品8.6需求，去掉智能登录提示窗口，去掉方式：在SGTool入口处干掉
		return 0;
		/*TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=smartlogontip"), szCmdLine, 1024);
		return SmartLogonTip( hInstance, hPrevInstance, szCmdLine, nCmdShow );*/
	}
	// qq管家登录
	else if( _tcsicmp(szAppID, _T("logonqq")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=logonqq"), szCmdLine, 1024);
		return LogonQQAccount( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("skinreg")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=skinreg"), szCmdLine, 1024);
		return SkinRegMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("scdreg")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=scdreg"), szCmdLine, 1024);
		return ScdRegMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("scdmaker")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=scdmaker"), szCmdLine, 1024);
		return ScdMakerMain( hInstance, hPrevInstance, szCmdLine, nCmdShow );
	}
	//else
	//{
	//	MessageBox(NULL, _T("没有找到对应的应用类型。"), _T("错误"), MB_OK);
	//	return -1;
	//}
	else if( _tcsicmp(szAppID, _T("upcelldict")) == 0 )
	{
		return UpCellDictMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	// 需要界面
	else if( _tcsicmp(szAppID, _T("upcelldictfunc")) == 0 )
	{
		return UpCellDictFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	// 需要界面
	else if( _tcsicmp(szAppID, _T("upUrlGuidefunc")) == 0 )
	{
		return UpUrlGuideFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("upImeRepair")) == 0 )
	{
		return UpImeRepairFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("upInfoKeyDict")) == 0 )
	{
		return UpInfoKeyDictFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("upPsgDict")) == 0 )
	{
		return UpPsgDictFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}

	else if( _tcsicmp(szAppID, _T("upAutoUp")) == 0 )
	{
		std::wstring strTmp = _T("SgTool_upAutoUp_Run");
		return UpAutoUpFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("ServerPopUp")) == 0 )
	{
		t_exceptionHandler::SetContinueHandle(false);
		return ServerPopUpFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("GetEnvFunc")) == 0 )
	{
		return GetEnvInfoFuncMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("LogoutWebMode")) == 0 )
	{
		return LogoutWebModeMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if( _tcsicmp(szAppID, _T("pandorabox")) == 0 )
	{
		return PandoraBoxMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if ( _tcsicmp(szAppID, _T("exinput")) == 0 )
	{
		if (_tcsstr(lpstrCmdLine, _T("-cid=charexp")) != NULL)
		{
			TCHAR szPath[260] = {0};
			::GetModuleFileName(0, szPath, MAX_PATH);
			TCHAR *pszStart = _tcsrchr(szPath, _T('\\'));
			if( pszStart )
			{
				*pszStart = 0;
			}
			t_path pathKaomoji(szPath);
			pathKaomoji.Append(_T("SGKaomoji.exe"));
			t_str strParam;
			if (_tcsstr(lpstrCmdLine, _T("-from=box")) != NULL)
			{
				strParam = _T("--fromTool");
			}
			else if (_tcsstr(lpstrCmdLine, _T("-from=center")) != NULL)
			{
				strParam = _T("--fromCenter");
			}
			else if (_tcsstr(lpstrCmdLine, _T("-from=shortcut")) != NULL)
			{
				strParam = _T("--fromShortCut");
			}
			t_error err;
			pathKaomoji.Exec(err, strParam.sz());
			return 0;
		}
		return RichInputMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	else if ( _tcsicmp(szAppID, _T("installdrv")) == 0 )
	{
		return InstallDrvMain( hInstance, hPrevInstance, lpstrCmdLine, nCmdShow );
	}
	//by continue:
	//else if( 0 == _tcsicmp(szAppID, _T("remotedicttool")) )
	//{
	//	TCHAR szCmdLine[1024] = {0};
	//	ExcludeAppidOption(lpstrCmdLine, _T("--appid=remotedicttool"), szCmdLine, 1024);
	//	return remoteDictNetIoProc(hInstance,hPrevInstance,lpstrCmdLine,nCmdShow);
	//}
	else if( _tcsicmp(szAppID, _T("monudev")) == 0 )
	{
		//2015.01.12:songtao:之前支持的命令，现在不工作
		return 0;
		//// 苏延刚 2014-05-22 添加
		//s_exceptionHandler.SetContinueHandle(false);
		//return UsbDeviceMonitorMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("medalwall") ) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=medalwall"), szCmdLine, 1024);
		return MedalWallMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("uprecommend") ) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=uprecommend"), szCmdLine, 1024);
		return UpRecommendMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("privilegeDict")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=privilegeDict"), szCmdLine, 1024);
		return PrivilegeMakerMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("usrZoneTool")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=usrZoneTool"), szCmdLine, 1024);
		return UsrZoneToolMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("eudc")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=eudc"), szCmdLine, 1024);
		return EudcMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("ucfont")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=ucfont"), szCmdLine, 1024);
		return UnCommonWordFontMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if( _tcsicmp(szAppID, _T("phraseedit")) == 0 )
	{
		return PhraseEditMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("showtip")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=showtip"), szCmdLine, 1024);
		return ShowTipInSGTool(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("setlogin")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=setlogin"), szCmdLine, 1024);
		return SetLoginMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("loadime")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=loadime"), szCmdLine, 1024);
		return LoadSogouPinpinTask(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID ,_T("accountmanager")) == 0 )
	{
		TCHAR szCmdLine[1024] = {0};
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=accountmanager"), szCmdLine, 1024);
		return PCAccountManager(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if (_tcsicmp(szAppID, _T("skinRenderUpdate")) == 0)
	{
		TCHAR szCmdLine[1024] = { 0 };
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=skinRenderUpdate"), szCmdLine, 1024);
		return SkinRenderNeedUpdateMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if (_tcsicmp(szAppID, _T("skindelete")) == 0)
	{
		TCHAR szCmdLine[1024] = { 0 };
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=skindelete"), szCmdLine, 1024);
		return SkinDeleteMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if ( _tcsicmp(szAppID, _T("companybox")) == 0 )
	{
		TCHAR szCmdLine[1024] = { 0 };
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=companybox"), szCmdLine, 1024);
		return CompanyBoxMain(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}
	else if (_tcsicmp(szAppID, _T("recorder")) == 0)
	{
		return RecorderMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}

	//个性化接口
	else if (_tcsicmp(szAppID, _T("individual")) == 0)
	{
		return IndividualMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
	}
	
	/** 手动通知UsbMonitor更新,方便测试 2019.8.5  liuqiang*/
	else if (_tcsicmp(szAppID, _T("dictationconfig")) == 0)
	{
		TCHAR szCmdLine[1024] = { 0 };
		ExcludeAppidOption(lpstrCmdLine, _T("--appid=dictationconfig"), szCmdLine, 1024);
		return NotifyUsbMonitorDefault(hInstance, hPrevInstance, szCmdLine, nCmdShow);
	}

	/** 修复refreshtoken 2019.8.5  liuqiang*/
	else if (_tcsicmp(szAppID, _T("fixlogin")) == 0)
	{
		return CheckNeedRelogin() ? 1 : 0;
	}



	MessageBox(NULL, _T("没有找到对应的应用类型。"), _T("错误"), MB_OK);
	return -1;
}

int _RepariMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	return n_sgImeRepairer::SgImeRepairerMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
}

// 调试时，只需加参数--appid=sysdicmk 与应用程序相关的参数均保持不变
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	//!< Crt异常时不弹错误框
	_set_error_mode(_OUT_TO_STDERR);

	return UniqueSgtool::_UniqueToolMain(hInstance, hPrevInstance, lpstrCmdLine, nCmdShow);
}
