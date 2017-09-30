'use strict';

var recordApp = angular.module('RecordApp', []);

recordApp.controller('recordCtrl', ['$scope', function($scope) {
		
	$scope.record = {  
						"blockNum": 0,
						"cFileName": "",
						"cCustomInfo":"",
						"cRightOwner":"",
						"cFileHash": "",
						"iFileState": 4,
						"tRegisTime": 0,
						"iFileSize": 0
					};
					
	$scope.language = 1;
	
	$scope.updateRecord = function(obj){
		$scope.record.blockNum = obj.blockNum;
		$scope.record.cFileName = obj.cFileName;
		$scope.record.cCustomInfo = obj.cCustomInfo;
		$scope.record.cRightOwner = obj.cRightOwner;
		$scope.record.cFileHash = obj.cFileHash;
		$scope.record.iFileState = obj.iFileState;
		$scope.record.tRegisTime = obj.tRegisTime;
		$scope.record.iFileSize = obj.iFileSize;
	};
	
	$scope.showHistory = function(){
		qRecord.showHistory();
	}
	
	$scope.getFileSizeDesc = function(){
		var size = $scope.record.iFileSize;
		
		if (size > 1024 * 1024)
			return '' + (size / (1024 * 1024)).toFixed(2) + 'M';
		if (size > 1024)
			return '' + (size / 1024).toFixed(2) + 'K';
		
		return size;
	};
	
	$scope.getEvidenceStatusDesc = function(){
        return Common.evidenceStatus($scope.record.iFileState, $scope.language);
	};
	
	$scope.getTimestampDesc = function(){
		return Common.timeStamp($scope.record.tRegisTime);
	};
	
	$scope.verify = function(){
		qRecord.verify($scope.record);
	};
	
}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qRecord = channel.objects.qRecord;
		
		qRecord.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("page")).scope();
			scope.$apply(function () {
				scope.language = lang;
			});
		});
		
		//绑定c++的回调函数
		qRecord.sigEvidenceChange.connect(function(obj){
			var scope = angular.element(document.getElementById("page")).scope();
			
			scope.$apply(function () {
				scope.updateRecord(obj);
			});
			
		});
		
		qRecord.onHtmlReady();
	});
});