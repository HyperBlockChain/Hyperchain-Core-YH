'use strict';

var blockInfoApp = angular.module('BlockInfoApp', []);

blockInfoApp.controller('blockInfoCtrl', ['$scope', function($scope) {
	$scope.blockInfo = {};
	$scope.blockInfo.blockNum = 0;
	$scope.blockInfo.fileName = "";
	$scope.blockInfo.customInfo = null;
	$scope.blockInfo.rightOwner = "";
	$scope.blockInfo.fileHash = "";
	$scope.blockInfo.regTime = 0;
	$scope.blockInfo.fileSize = 0;
	$scope.blockInfo.fileState = 0;
	
	//lang =  0：英文, 1中文
	$scope.content_lang = 1;
	$scope.setLang = function(lang){
		$scope.content_lang = lang;
	}

    $scope.update = function(obj){
        $scope.blockInfo.blockNum = obj.blockNum;
		$scope.blockInfo.fileName = obj.fileName;
		$scope.blockInfo.customInfo = obj.customInfo;
		$scope.blockInfo.rightOwner = obj.rightOwner;
		$scope.blockInfo.fileHash = obj.fileHash;
		$scope.blockInfo.regTime = obj.regTime;
		$scope.blockInfo.fileSize = obj.fileSize;
		$scope.blockInfo.fileState = obj.fileState;
    };
	
	$scope.getFileHash = function(){
		var ret = "";
		
		var obj = $scope.blockInfo;
		
		if(obj.fileHash){
			// ret = obj.fileHash.slice(0, 32);
			// ret += "...";
			
			// if(obj.fileHash.length == 128){
			// 	ret += obj.fileHash.slice(128-32, 32)
			// }
			ret = obj.fileHash;
		}
		
		return ret;
	};
	
	$scope.getFileState = function(state){
		return Common.evidenceStatus(state, $scope.content_lang);
	};
	
	$scope.getTimestampDesc = function(stamp){
		return Common.timeStamp(stamp);
	};
	
	$scope.getFileSizeDesc = function(size){
		return Common.formatFileSize(size);
	};

}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qBlockInfo = channel.objects.qBlockInfo;
		
		qBlockInfo.sigNewBlockInfo.connect(function(obj){
			refreshShow(obj);
		});
		
		qBlockInfo.sigChangeLang.connect(function(lang){
			changLang(lang);
		});
	});
});


function refreshShow(obj){
    var scope = angular.element(document.getElementById("container")).scope();

    scope.$apply(function () {
		//这里本来也可以直接修改blockNum,为了扩展，还是调用update()函数吧
		//scope.blockNum = obj.blockNum;
		
		scope.update(obj);
        //scope.$apply();
    });
};


function changLang(lang){
	var scope = angular.element(document.getElementById("container")).scope();

    scope.$apply(function () {
		scope.setLang(lang);
    });
}