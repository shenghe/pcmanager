/**
 * @brief   常用功能脚本
 * @author  hechangmin@gmail.com
 * @date	2010.5
 * @version 1.1
 * 
 * Copyright (c) 2010 hechangmin@gmail.com
 * Licensed under the MIT License:  
 * http://www.opensource.org/licenses/mit-license.php
 */
   
var Util = {
	/**
	 * @brief 从当前url中获取参数
	 * @param {Object} name
	 */
    getParam : function(name){
        var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
        var r = (unescape(decodeURI(window.location.search))).substr(1).match(reg);
        if (r != null) return unescape(r[2]);
        return null;

    },
	
	/**
	 * @brief 预加载图片
	 * @param {Object} arrImgSrc 图片src 数组
	 */
   preloader: function(arrImgSrc){
        for (var i = 0; i < arrImgSrc.length; i++) 
		{
            var preImg = new Image();
            preImg.src = arrImgSrc[i];

        }
    },
	
	/**
	 * @brief 创建名字空间
	 * @param {Object} router
	 */
	initNameSpace : function(router){
		if(!router||router=='')
		{
			return;
		}
		
		var p = window, arrNS = router.split('.');
		
		for(var i=0, len = arrNS.length; i<len; i++)
		{
			if(!p[arrNS[i]])
			{
				p[arrNS[i]] = {};
			}
			
			p = p[arrNS[i]];
		}
	},
	
	/**
	 * @brief 继承 扩展
	 * @param {Object} target
	 * @param {Object} src
	 */
	extend : function(target, src){
        for (var it in src) 
        {
            target[it] = src[it];
        }
        return target;
    }    
};