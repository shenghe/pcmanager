/**
 * @brief   提供给C++ 调用的接口
 * @author  hechangmin@gmail.com
 * @date    2010.12.10
 */
 
window.WebApp = window.WebApp || {};

WebApp.CallBack = {
    
    callBack4C : function(strFunName,jsonParam)
    {
        eval(strFunName)(jsonParam);
    },  
    
    test : function()
    {
        alert("duba");
    }
};
