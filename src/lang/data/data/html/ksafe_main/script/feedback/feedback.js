/**
 * @author linqihuan
 * @Email:linqihuan@kingsoft.com;
 * @date:2010.05.06
 */
var KResName = {};
KResName.defMAIN_MINI_CLICK_NORMAL_BTN		=	"images/main_mini_normal.jpg";
KResName.defMAIN_MINI_CLICK_HOVER_BTN		= 	"images/main_mini_hover.jpg";
KResName.defMAIN_MINI_CLICK_DOWN_BTN		=	"images/main_mini_down.jpg";

KResName.defMAIN_SENDMSG_CLICK_NORMAL		=	"resources/default/images/send_msg_normal.png";
KResName.defMAIN_SENDMSG_CLICK_HOVER		= 	"resources/default/images/send_msg_over.png";
KResName.defMAIN_SENDMSG_CLICK_DOWN		=	"resources/default/images/send_msg_down.png";

KResName.Str = {};
KResName.Str.MAIN_TITLE 					= "アンインストール　ウィザード";
KResName.Str.TIP_SENDEMSG_SECCUSS 			= "送信完了！\nアンインストールを中止します。";
KResName.Str.TIP_SENDEMSG_FAILDE 			= "転送失敗！ホンページへフィードバックしてください";
KResName.Str.TIP_SELECT_NOTHING				= "項目を選定してください。";
KResName.Str.MAIN_TITLE 					= "アンインストール　ウィザード";
KResName.Str.TIP_EAMIL_DEFAULT	 			= "返信用メールアドレスを入力してください";
KResName.Str.TIP_EAMIL_FORMATERR 			= "正しいメールアドレスを入力してください！";
KResName.Str.TIP_EMAIL_NOTFOUNDED 			= "メールアドレスを入力してください！";
KResName.Str.TIP_MSG_DEFAULT				= "●変のネット場所:       \r●他のフィードバック:      \r";
KResName.Str.TIP_MSG_NOTFOUNDED 			= "お問い合わせ内容を入力してください！";
KResName.Str.TIP_MSG_INPUTMSG	 			= "メモ";
KResName.Str.SEL_TEXT						= "●トラブル内容： ";

KResName.Link = {};
KResName.Link.EMAIL			= "https://pay.kingsoft.jp/contact2/mail_send2.php";
KResName.Link.FEEDBACK 		= "http://www.kingsoft.jp";
KResName.Link.HOMEPAGE 		= "http://www.kingsoft.jp/support/security/";
KResName.Link.PAYPAGE		= "https://pay.kingsoft.jp/shopping/products/list.php?category_id=3";
KResName.Link.ADSIMG_LEFT   = KResName.Link.PAYPAGE;
KResName.Link.ADSIMG_RIGHT  = KResName.Link.PAYPAGE;

var type = ""; 
var type2 = ""; 
var navgation = "1";
$(document).ready(function()
{
	KWebApp.WebMain();
});

var KWebApp = {
	WebMain: function(){
		KWebApp.InitPage();
	},
	/**
	 * @brief	网页事件初始化
	 */
	SendMsg : function()
	{
		if(!KWebApp.CheckMsg())return;
		var param = {};
		param.content = $.trim($("#msg").text().replace(/(\n)|(\r)/g, "<br/>"));
		param.mail = $.trim($("#email").val());
		param.subject = "Webdefenderからのデータ";
		param.to = "webdefender@kingsoft.jp";
		param.cs = KCode.Md5(KCode.UTF8(param.mail+param.to+"GhB6hmB7IL4VHJ"));
		//alert(param.mail+param.content+param.subject+param.to+"GhB6hmB7IL4VHJ");
		//alert("contet=" + param.content + "&mail=" + param.mail + "&subject=" + param.subject + "&to=" + param.to + "&cs=" + param.cs);
		//alert($.param(param));
		$.ajax({
			type: "POST",
			url: KResName.Link.EMAIL,
			data: $.param(param),//$.param({"mail": email, "content": msg, "cs" : cs}),
			dataType: 'xml',
			beforeSend:function(){
			},
			error:function(data, status){
				alert(KResName.Str.TIP_SENDEMSG_FAILDE);
				retcode = 4;//未知错误导致发送消息失败
			},
			success: function(data, textStatus){
				var result = $(data).find("result").text();
				if("NG"==result)
				{
					alert($(data).find("err_detail").text());
					//alert($(data).find("err_detail").text());
					retcode = 5; //服务器返回的错误
				}
				else if("OK" == result)
				{
					KWebApp.Reset();
					alert(KResName.Str.TIP_SENDEMSG_SECCUSS);
					retcode = 6; //发送消息成功					
				}
				else
				{
					alert(KResName.Str.TIP_SENDEMSG_FAILDE);
					retcode = 7; //服务器没有正确返回
				}
			},
			complete:function(data,state){
				//sendInfoc(retcode);
			}
		});
	},
	CheckMsg : function()
	{
		var str = $("#msg").text().replace(/(^\s*)|(\s*$)/g, "");
		if(KResName.Str.TIP_MSG_DEFAULT.replace(/(^\s*)|(\s*$)/g, "") == str || "" == str)
		{
			alert(KResName.Str.TIP_MSG_NOTFOUNDED);
			$("#msg").focus();
			return false;
		}
		str = $("#email").val().replace(/(^\s*)|(\s*$)/g, "");
		if(str == "")
		{
			alert(KResName.Str.TIP_EMAIL_NOTFOUNDED);
			$("#email").focus();
			return false;
		}
		if(str.search(/^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/))
		{
			alert(KResName.Str.TIP_EAMIL_FORMATERR );
			$("#email").focus();
			return false;
		}
		return true;
		
	},
	InitPage:function(selected, info)
	{
		
		$("#send_msg_btn").mouseover(function(){
			$(this).css({"background-image":"url(" + KResName.defMAIN_SENDMSG_CLICK_HOVER + ")"});
		})
		.mouseout(function(){
			$(this).css({"background-image":"url(" + KResName.defMAIN_SENDMSG_CLICK_NORMAL + ")"});	
		})
		.mousedown(function(){
			$(this).css({"background-image":"url(" + KResName.defMAIN_SENDMSG_CLICK_DOWN + ")"});
		})
		.mouseup(function(){
			$(this).css({"background-image":"url(" + KResName.defMAIN_SENDMSG_CLICK_NORMAL + ")"});
		})
		.click(function(){
			KWebApp.SendMsg();
		});
		$("#msg").text(KResName.Str.TIP_MSG_DEFAULT).focus(function(){				
			var str = $(this).text().replace(/(^\s*)|(\s*$)/g, "");
			if(str == "")
			{
				$(this).text(KResName.Str.TIP_MSG_DEFAULT);
			}
			$(this).css({"color":"#000"});
		}).blur(function(){
			var str = $(this).text().replace(/(^\s*)|(\s*$)/g, "");
			if(str == "" || str == KResName.Str.TIP_MSG_DEFAULT.replace(/(^\s*)|(\s*$)/g, ""))
			{
				$(this).text(KResName.Str.TIP_MSG_DEFAULT)
				.css({			 
					"color":"#999"
				});				
			}
		});
		
		$("#email").focus(function(){				
			var str = $(this).val().replace(/(^\s*)|(\s*$)/g, "");
			if(str == "" || str == KResName.Str.TIP_EAMIL_DEFAULT)
			{
				$(this).val(" ")
				.css({			 
					"color":"#000"
				});
			}
		}).blur(function(){
			var str = $(this).val().replace(/(^\s*)|(\s*$)/g, "");
			if(str == "" || str == KResName.Str.TIP_EAMIL_DEFAULT)
			{
				$(this).val(KResName.Str.TIP_EAMIL_DEFAULT)
				.css({			 
					"color":"#999"
				});
			}
		});
	},
	Reset : function()
	{
		$("#msg").val(KResName.Str.TIP_MSG_DEFAULT)
		.css({"color":"#999"});	
		$("#email").val(KResName.Str.TIP_EAMIL_DEFAULT)
		.css({"color":"#999"});	
	}
};
