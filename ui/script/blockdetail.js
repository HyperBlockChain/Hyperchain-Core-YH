'use strict';

var blockDetailApp = angular.module('DetailApp', []);

blockDetailApp.controller('detailCtrl', ['$scope', function($scope) {
	$scope.language = 1;

	$scope.detail = {};
	$scope.detail.tFileInfo = {};
	
	$scope.refreshShow = function(obj){
		$scope.detail.iBlockNum = obj.iBlockNum;
		$scope.detail.iUseNodes = obj.iUseNodes ;
		$scope.detail.iDataChainNum = obj.iDataChainNum ;
		$scope.detail.iChainNum = obj.iChainNum ;
		$scope.detail.iLongestChain = obj.iLongestChain ;
		$scope.detail.tTime = obj.tTime;
		
		//存证文件
		$scope.detail.tFileInfo.cFileName = obj.tFileInfo.cFileName;
		$scope.detail.tFileInfo.cCustomInfo = obj.tFileInfo.cCustomInfo;
		$scope.detail.tFileInfo.cRightOwner = obj.tFileInfo.cRightOwner;
		$scope.detail.tFileInfo.cFileHash = obj.tFileInfo.cFileHash;
		$scope.detail.tFileInfo.iFileState = obj.tFileInfo.iFileState;
		$scope.detail.tFileInfo.tRegisTime = obj.tFileInfo.tRegisTime;
		$scope.detail.tFileInfo.iFileSize = obj.tFileInfo.iFileSize;
	};
	
	$scope.getFileState = function(state){
		return Common.evidenceStatus(state, $scope.language);
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

		window.qBlockDetail = channel.objects.qBlockDetail;

		qBlockDetail.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("container")).scope();
			scope.$apply(function () {
				scope.language = lang;
			});
		});
		
		qBlockDetail.sigNewBlockDetail.connect(function(obj){
			refreshShow(obj);
		});
	});
});


function refreshShow(obj){
    var scope = angular.element(document.getElementById("container")).scope();

    scope.$apply(function () {
		
		scope.refreshShow(obj);

    });
}