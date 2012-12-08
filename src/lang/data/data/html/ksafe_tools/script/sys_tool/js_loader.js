/**
 * @brief 统一脚本加载
 */
 
$(function()
{
    $.ajaxSetup({async : false});
    $.getScript("script/common/util.js");
	//alert("begin webapp.js");
    $.getScript("script/sys_tool/webapp.js");
	//alert("end webapp.js");
    $.getScript("script/sys_tool/callback.js");
	//alert("end callback.js");
    $.ajaxSetup({async : true});
	//alert("end ajaxSetup");
    WebApp.main();
});