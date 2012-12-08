/**
 * @brief 系统工具
 * 
 */
var WebApp = {
    
    data : [{
		//网路测速
        strExecPath : "macropath|%ksafepath%",
		strDownloadPath : "macropath|%ksafepath%",
        strFileName : "kmspeed.exe",
        strFileVer	: "2.2.0.1124",
        strDownloadUrl : "http://dl.ijinshan.com/safe/kmspeed.exe",
        strSetupParam : "nosetup",
		strExeParam:"",
		strTitle:"网络测速"
        
    },{ 
		//隐私保护器
        strExecPath : "regpath|HKLM|SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\kprivacy.exe|",
		strDownloadPath : "macropath|%ksafepath%\\temp",
        strFileName : "kprivacy.exe",
        strFileVer	: "2010.12.9.110",
        strDownloadUrl : "http://download.duba.net/2011/KPrivacySetup/KPrivacySetup.exe",
        strSetupParam : "/S",
		strExeParam:"",
		strTitle:"隐私保护器"
    }],
        
    main : function()
    {
        WebApp.initPage();
        WebApp.initEvent();
        WebApp.preloadImg();
    },
    
    initPage : function()
    {
        WebApp.disabledSelect();
    },
	
    disabledSelect : function()
	{
		document.body.oncontextmenu = function(){return false;};  
		
		document.body.onselectstart = function(event)
		{
			event = event? event : window.event;
			var bRet = false;
			
			var objEventSrc = event.srcElement ? event.srcElement : event.target;
			
			if("INPUT" == objEventSrc.tagName.toUpperCase())
			{
				bRet = true;
			}
			
			return bRet;
		};
	},
	
    initEvent : function()
    {
		
        $("#content li").mouseout(function()
        {
            $(this).removeClass("on_menu");
        }).mouseover(function()
        {
             $(this).addClass("on_menu");
        }).click(function(){
            

            var i = $("#content li").index($(this));            
            
            var strExecPath		= WebApp.data[i].strExecPath;
            var strFileName		= WebApp.data[i].strFileName;
            var strFileVer		= WebApp.data[i].strFileVer;
            var strDownloadUrl	= WebApp.data[i].strDownloadUrl;
            var strSetupParam	= WebApp.data[i].strSetupParam;
			var strDownloadPath = WebApp.data[i].strDownloadPath;
			var strExeParam		= WebApp.data[i].strExeParam;
			var strTitle		= WebApp.data[i].strTitle;
            
            //console.dir(WebApp.data[i]);
            
            var ret = window.external.check(strExecPath,strFileName,strFileVer);
            
            if("0" == ret)
            {
                 window.external.install(strTitle,strDownloadUrl,strDownloadPath,strSetupParam);  
            }
			//下载完了，在调用安装，如果是直接下载EXE，有可能会重复调用的问题，所以被调用程序要处理单实例
			window.external.exec(strExecPath,strFileName,strExeParam);
        });
        
        
    },
    
    preloadImg : function()
    {
           
    }
};
$(function()
{
    WebApp.main();
});