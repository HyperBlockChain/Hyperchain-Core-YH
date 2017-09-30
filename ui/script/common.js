if (typeof Common !== 'object') {
    Common = {};
}

(function () {
    'use strict';

    if(typeof Common.myDefineProperty !== 'string'){
        Common.myDefineProperty = "hah, world";
    }
	
	if(typeof Common.formatFileSize !== 'function'){
		Common.formatFileSize = function(byteSize){
			if(byteSize <= 1024){
				return '' + byteSize + '字节';
			}else if(byteSize > 1024 && byteSize < 1048576){
				return '' + Math.ceil( byteSize / 1024) + 'k';
			}
			
			return '' + Math.ceil( byteSize / 1048576) + 'M';
		}
	}
	
	if(typeof Common.timeStamp !== 'function'){
		Common.timeStamp = function(time){
			if(time == 0){
				return '0';
			}
			
			var unixTimestamp;
			if(time < 9999999999){
				unixTimestamp = new Date( time * 1000 );
			}else{
				unixTimestamp = new Date( time );
			}
			return ''+ unixTimestamp.getFullYear() + "-" + (unixTimestamp.getMonth() + 1) + "-" + unixTimestamp.getDate() + " " + unixTimestamp.getHours() + ":" + unixTimestamp.getMinutes() + ":" + unixTimestamp.getSeconds();
			
		}
	}
	
	if(typeof Common.evidenceStatus !== 'function'){
		Common.evidenceStatus = function(state, language){
		// enum _eRegisReqState
		// {
			// DEFAULT_REGISREQ_STATE = 0,
			// RECV,													//已接收
			// SEND,													//已发送
			// STOP,													//中止
			// CONFIRMING,												//待确认
			// CONFIRMED,												//已确认
			// MYREFUSEOTHER,											//已拒绝
			// OTHERREFUSEME,											//被拒绝
			// ALLCONFIRMED,											//累计已确认
			// ALLOTHERREFUSEME,										//累计被拒绝
			// ALLMYREFUSEOTHER											//累计已拒绝
		// }
			var desc = '';
			if(language == 'undefined' || language == 1){
				switch(state){
					case 1: desc = "已接收"; break;
					case 2: desc = "已发送"; break;
					case 3: desc = "中止"; break;
					case 4: desc = "待确认"; break;
					case 5: desc = "已确认"; break;
					case 6: desc = "已拒绝"; break;
					case 7: desc = "被拒绝"; break;
					default: desc = '未知';
				}
			}else{
					switch(state){
					case 1: desc = "Received"; break;
					case 2: desc = "Send"; break;
					case 3: desc = "Stop"; break;
					case 4: desc = "Confirming"; break;
					case 5: desc = "Confirmed"; break;
					case 6: desc = "RefuseOther"; break;
					case 7: desc = "Refused"; break;
					default: desc = 'Unknow';
				}
			}
		
			return desc;
		}
	}

}());
