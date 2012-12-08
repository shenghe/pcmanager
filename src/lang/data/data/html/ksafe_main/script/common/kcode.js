/*
	@author	: zzm<zhangzhongming@kingsoft.com>
	@create	: 2011.2.26
	@des	: md5, utf8, unsigned operation.
*/
var Unsigned = 
{
	Add : function(uA, uB)
	{
		 var uA4, uB4, uA8, uB8, uRetCode;  
         uA8 = (uA & 0x80000000);  
         uB8 = (uB & 0x80000000);  
         uA4 = (uA & 0x40000000);  
         uB4 = (uB & 0x40000000);  
         uRetCode = (uA & 0x3FFFFFFF) + (uB & 0x3FFFFFFF); 
         if (uA4 & uB4)  
             return (uRetCode ^ 0x80000000 ^ uA8 ^ uB8);  
         if (uA4 | uB4) {  
             if (uRetCode & 0x40000000)  
                 return (uRetCode ^ 0xC0000000 ^ uA8 ^ uB8);  
             else  
                 return (uRetCode ^ 0x40000000 ^ uA8 ^ uB8);  
         } else {  
             return (uRetCode ^ uA8 ^ uB8); 
         }  
	},
	ToHex : function(uValue)
	{
		var hex = "", temp = "", uByte, uCount;  
		for (uCount = 0; uCount <= 3; uCount++) {  
			uByte = (uValue >>> (uCount * 8)) & 255;  
			temp = "0" + uByte.toString(16);  
			hex = hex + temp.substr(temp.length - 2, 2);  
		}  
		return hex;
	}
}

var Md5 = function()
{
	var S11 = 7, S12 = 12, S13 = 17, S14 = 22;  
	var S21 = 5, S22 = 9, S23 = 14, S24 = 20;  
	var S31 = 4, S32 = 11, S33 = 16, S34 = 23;  
	var S41 = 6, S42 = 10, S43 = 15, S44 = 21;  
	
	var F = function(x, y, z) {  
		return (x & y) | ((~x) & z);  
	}  
	var G = function(x, y, z) {  
		return (x & z) | (y & (~z));  
	}  
	var H = function(x, y, z) {  
		return (x ^ y ^ z);  
	}  
	var I = function(x, y, z) {  
		return (y ^ (x | (~z)));  
	}  
	
	var RotateLeft = function(x, n) {  
		return (x << n) | (x >>> (32 - n));  
	}
	 
	var FF = function(a, b, c, d, x, s, ac) {  
		a = Unsigned.Add(a, Unsigned.Add(Unsigned.Add(F(b, c, d), x), ac));
		return Unsigned.Add(RotateLeft(a, s), b);  
	};  
	var GG = function(a, b, c, d, x, s, ac) {  
		a = Unsigned.Add(a, Unsigned.Add(Unsigned.Add(G(b, c, d), x), ac));  
		return Unsigned.Add(RotateLeft(a, s), b);  
	};  
	var HH = function(a, b, c, d, x, s, ac) {  
		a = Unsigned.Add(a, Unsigned.Add(Unsigned.Add(H(b, c, d), x), ac));  
		return Unsigned.Add(RotateLeft(a, s), b);  
	};  
	var II = function(a, b, c, d, x, s, ac) {  
		a = Unsigned.Add(a, Unsigned.Add(Unsigned.Add(I(b, c, d), x), ac));  
		return Unsigned.Add(RotateLeft(a, s), b);  
	}; 
	
	var ToHexStr = function(iState, nType)
	{
		var strRet = Unsigned.ToHex(iState[0]) + Unsigned.ToHex(iState[1]) + Unsigned.ToHex(iState[2]) + Unsigned.ToHex(iState[3]); 
		switch(nType)
		{
			case 1:return strRet.toLowerCase();
			case 0:return strRet.toUpperCase(); 
			default:
				return strRet.toUpperCase();
		}
	}
	
	var MakeStrToWords = function(strValue)
	{
		var uStrLen = strValue.length;   
		var uWordCout = ((((uStrLen + 8) - ((uStrLen + 8) % 64)) / 64) + 1) * 16;  
		var uArrary = Array(uWordCout - 1);  
		var uCount = 0, uBytePos = 0, uByteTotal = 0;
		for (uByteTotal = 0; uByteTotal < uStrLen; uByteTotal++) {  
			uBytePos = (uByteTotal % 4) * 8;  
			uCount = (uByteTotal - (uByteTotal % 4)) / 4;  
			uArrary[uCount] = (uArrary[uCount] | (strValue.charCodeAt(uByteTotal) << uBytePos));  
		}  
		uCount = (uByteTotal - (uByteTotal % 4)) / 4;  
		uBytePos = (uByteTotal % 4) * 8;  
		uArrary[uCount] = uArrary[uCount] | (0x80 << uBytePos);  
		uArrary[uWordCout - 2] = uStrLen << 3;  
		uArrary[uWordCout - 1] = uStrLen >>> 29;
		return uArrary;
	}		
	var Transform = function(iState, x, k)
	{
		var is0 = iState[0], is1 = iState[1], is2 = iState[2], is3 = iState[3];		
		iState[0] = FF(iState[0], iState[1], iState[2], iState[3], x[k + 0], S11, 0xD76AA478); 
		iState[3] = FF(iState[3], iState[0], iState[1], iState[2], x[k + 1], S12, 0xE8C7B756);  
		iState[2] = FF(iState[2], iState[3], iState[0], iState[1], x[k + 2], S13, 0x242070DB);  
		iState[1] = FF(iState[1], iState[2], iState[3], iState[0], x[k + 3], S14, 0xC1BDCEEE);  
		iState[0] = FF(iState[0], iState[1], iState[2], iState[3], x[k + 4], S11, 0xF57C0FAF);  
		iState[3] = FF(iState[3], iState[0], iState[1], iState[2], x[k + 5], S12, 0x4787C62A);  
		iState[2] = FF(iState[2], iState[3], iState[0], iState[1], x[k + 6], S13, 0xA8304613);  
		iState[1] = FF(iState[1], iState[2], iState[3], iState[0], x[k + 7], S14, 0xFD469501);  
		iState[0] = FF(iState[0], iState[1], iState[2], iState[3], x[k + 8], S11, 0x698098D8);  
		iState[3] = FF(iState[3], iState[0], iState[1], iState[2], x[k + 9], S12, 0x8B44F7AF);  
		iState[2] = FF(iState[2], iState[3], iState[0], iState[1], x[k + 10], S13, 0xFFFF5BB1);  
		iState[1] = FF(iState[1], iState[2], iState[3], iState[0], x[k + 11], S14, 0x895CD7BE);  
		iState[0] = FF(iState[0], iState[1], iState[2], iState[3], x[k + 12], S11, 0x6B901122);  
		iState[3] = FF(iState[3], iState[0], iState[1], iState[2], x[k + 13], S12, 0xFD987193);  
		iState[2] = FF(iState[2], iState[3], iState[0], iState[1], x[k + 14], S13, 0xA679438E);  
		iState[1] = FF(iState[1], iState[2], iState[3], iState[0], x[k + 15], S14, 0x49B40821);  
		iState[0] = GG(iState[0], iState[1], iState[2], iState[3], x[k + 1], S21, 0xF61E2562);  
		iState[3] = GG(iState[3], iState[0], iState[1], iState[2], x[k + 6], S22, 0xC040B340);  
		iState[2] = GG(iState[2], iState[3], iState[0], iState[1], x[k + 11], S23, 0x265E5A51);  
		iState[1] = GG(iState[1], iState[2], iState[3], iState[0], x[k + 0], S24, 0xE9B6C7AA);  
		iState[0] = GG(iState[0], iState[1], iState[2], iState[3], x[k + 5], S21, 0xD62F105D);  
		iState[3] = GG(iState[3], iState[0], iState[1], iState[2], x[k + 10], S22, 0x2441453);  
		iState[2] = GG(iState[2], iState[3], iState[0], iState[1], x[k + 15], S23, 0xD8A1E681);  
		iState[1] = GG(iState[1], iState[2], iState[3], iState[0], x[k + 4], S24, 0xE7D3FBC8);  
		iState[0] = GG(iState[0], iState[1], iState[2], iState[3], x[k + 9], S21, 0x21E1CDE6);  
		iState[3] = GG(iState[3], iState[0], iState[1], iState[2], x[k + 14], S22, 0xC33707D6);  
		iState[2] = GG(iState[2], iState[3], iState[0], iState[1], x[k + 3], S23, 0xF4D50D87);  
		iState[1] = GG(iState[1], iState[2], iState[3], iState[0], x[k + 8], S24, 0x455A14ED);  
		iState[0] = GG(iState[0], iState[1], iState[2], iState[3], x[k + 13], S21, 0xA9E3E905);  
		iState[3] = GG(iState[3], iState[0], iState[1], iState[2], x[k + 2], S22, 0xFCEFA3F8);  
		iState[2] = GG(iState[2], iState[3], iState[0], iState[1], x[k + 7], S23, 0x676F02D9);  
		iState[1] = GG(iState[1], iState[2], iState[3], iState[0], x[k + 12], S24, 0x8D2A4C8A);  
		iState[0] = HH(iState[0], iState[1], iState[2], iState[3], x[k + 5], S31, 0xFFFA3942);  
		iState[3] = HH(iState[3], iState[0], iState[1], iState[2], x[k + 8], S32, 0x8771F681);  
		iState[2] = HH(iState[2], iState[3], iState[0], iState[1], x[k + 11], S33, 0x6D9D6122);  
		iState[1] = HH(iState[1], iState[2], iState[3], iState[0], x[k + 14], S34, 0xFDE5380C);  
		iState[0] = HH(iState[0], iState[1], iState[2], iState[3], x[k + 1], S31, 0xA4BEEA44);  
		iState[3] = HH(iState[3], iState[0], iState[1], iState[2], x[k + 4], S32, 0x4BDECFA9);  
		iState[2] = HH(iState[2], iState[3], iState[0], iState[1], x[k + 7], S33, 0xF6BB4B60);  
		iState[1] = HH(iState[1], iState[2], iState[3], iState[0], x[k + 10], S34, 0xBEBFBC70);  
		iState[0] = HH(iState[0], iState[1], iState[2], iState[3], x[k + 13], S31, 0x289B7EC6);  
		iState[3] = HH(iState[3], iState[0], iState[1], iState[2], x[k + 0], S32, 0xEAA127FA);  
		iState[2] = HH(iState[2], iState[3], iState[0], iState[1], x[k + 3], S33, 0xD4EF3085);  
		iState[1] = HH(iState[1], iState[2], iState[3], iState[0], x[k + 6], S34, 0x4881D05);  
		iState[0] = HH(iState[0], iState[1], iState[2], iState[3], x[k + 9], S31, 0xD9D4D039);  
		iState[3] = HH(iState[3], iState[0], iState[1], iState[2], x[k + 12], S32, 0xE6DB99E5);  
		iState[2] = HH(iState[2], iState[3], iState[0], iState[1], x[k + 15], S33, 0x1FA27CF8);  
		iState[1] = HH(iState[1], iState[2], iState[3], iState[0], x[k + 2], S34, 0xC4AC5665);  
		iState[0] = II(iState[0], iState[1], iState[2], iState[3], x[k + 0], S41, 0xF4292244);  
		iState[3] = II(iState[3], iState[0], iState[1], iState[2], x[k + 7], S42, 0x432AFF97);  
		iState[2] = II(iState[2], iState[3], iState[0], iState[1], x[k + 14], S43, 0xAB9423A7);  
		iState[1] = II(iState[1], iState[2], iState[3], iState[0], x[k + 5], S44, 0xFC93A039);  
		iState[0] = II(iState[0], iState[1], iState[2], iState[3], x[k + 12], S41, 0x655B59C3);  
		iState[3] = II(iState[3], iState[0], iState[1], iState[2], x[k + 3], S42, 0x8F0CCC92);  
		iState[2] = II(iState[2], iState[3], iState[0], iState[1], x[k + 10], S43, 0xFFEFF47D);  
		iState[1] = II(iState[1], iState[2], iState[3], iState[0], x[k + 1], S44, 0x85845DD1);  
		iState[0] = II(iState[0], iState[1], iState[2], iState[3], x[k + 8], S41, 0x6FA87E4F);  
		iState[3] = II(iState[3], iState[0], iState[1], iState[2], x[k + 15], S42, 0xFE2CE6E0);  
		iState[2] = II(iState[2], iState[3], iState[0], iState[1], x[k + 6], S43, 0xA3014314);  
		iState[1] = II(iState[1], iState[2], iState[3], iState[0], x[k + 13], S44, 0x4E0811A1);  
		iState[0] = II(iState[0], iState[1], iState[2], iState[3], x[k + 4], S41, 0xF7537E82);  
		iState[3] = II(iState[3], iState[0], iState[1], iState[2], x[k + 11], S42, 0xBD3AF235);  
		iState[2] = II(iState[2], iState[3], iState[0], iState[1], x[k + 2], S43, 0x2AD7D2BB);  
		iState[1] = II(iState[1], iState[2], iState[3], iState[0], x[k + 9], S44, 0xEB86D391);
		iState[0] = Unsigned.Add(iState[0], is0);  
		iState[1] = Unsigned.Add(iState[1], is1);  
		iState[2] = Unsigned.Add(iState[2], is2);  
		iState[3] = Unsigned.Add(iState[3], is3);
	}
	
	this.ToMd5 = function(strValue, nType)
	{
		var arr = MakeStrToWords(strValue);
		var iState = [0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476];
		
		for(var i = 0; i < arr.length; i += 16)
			Transform(iState, arr, i);
			
		return ToHexStr(iState, nType);
	}
}

var KCode = 
{
	Md5:function(strSrc, nType)
	{
		var md5 = new Md5();
		return md5.ToMd5(strSrc, nType);
	},
	UTF8:function(strSrc)
	{
		strSrc = strSrc.replace(/\x0d\x0a/g, "\x0a");  
		var strDest = "";  
		for ( var n = 0; n < strSrc.length; n++) {  
			var c = strSrc.charCodeAt(n);  
			if (c < 128) {  
				strDest += String.fromCharCode(c);  
			} else if ((c > 127) && (c < 2048)) {  
				strDest += String.fromCharCode((c >> 6) | 192);  
				strDest += String.fromCharCode((c & 63) | 128);  
			} else {  
				strDest += String.fromCharCode((c >> 12) | 224);  
				strDest += String.fromCharCode(((c >> 6) & 63) | 128);  
				strDest += String.fromCharCode((c & 63) | 128);  
			}  
		}  
		return strDest;  
	}
}