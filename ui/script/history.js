'use strict';

var historyApp = angular.module('HistoryApp', []);

historyApp.controller('historyCtrl', ['$scope', '$interval', function($scope,$interval) {
	$interval(function(){
		var timestamp=new Date().getTime();
		
		$scope.currentTime = Common.timeStamp(timestamp);
	},1000, -1);
	
	$scope.curPage = 1;
	
	$scope.language = 1;
	
	var timestamp=new Date().getTime();	
	$scope.currentTime = Common.timeStamp(timestamp);
	
	//{  
	//  "blockNum": 1
	//	"cFileName": 20,
	//	"cCustomInfo":"xxxxxx",
	//  "cRightOwner":"sjc",
	//  "cFileHash": "aaaaaaa",
	//  "iFileState": 4
	//  "tRegisTime": 111111,
	//  "iFileSize": 1111111
	//}
	$scope.historys = [];

	$scope.getEvidence = function(page, pageSize){
		qHistory.getEvidence(page, pageSize);
	};
	
	$scope.updateHistroies = function(arr){
		$scope.historys.splice(0, $scope.historys.length);
		
		for(var i = 0; i < arr.length; i++){
			$scope.historys.push(arr[i]);
		}
	};

	$scope.addEvidece = function(obj){
		$scope.historys.push(obj);
	};

	$scope.updateEvidence = function(hash, evidence, type){
		for(var i = 0; i < $scope.historys.length; i++){
			if($scope.historys[i].cFileHash == hash){

				if(type == 1){
					$scope.historys[i].iFileState = evidence.iFileState;
					break;
				}
				
			}
		}
	};
	
	$scope.checkEvidenceInfo = function(evidence){
		qHistory.checkEvidenceInfo(evidence);
	};
	
	$scope.delMultiEvidence = function(){
		var arr = [1, 2, 3];
		qHistory.delMultiEvidence(arr);
	};
	
	$scope.exportAllEvidence = function(){
		qHistory.exportAllEvidence();
	};
	
	$scope.exportEvidence = function(){
		var arr = [1, 2,3];
		qHistory.exportEvidence(arr);
	};
	
	$scope.getStatusDesc = function(evidence){
		return Common.evidenceStatus(evidence.iFileState, $scope.language);
	};
	
	$scope.verifyEvidence = function(evidence){
		qHistory.verifyEvidence(evidence);
	};
	
	$scope.getTimeStampDesc = function(evidence){
		return Common.timeStamp(evidence.tRegisTime);
	};
	
	$scope.getFileSizeDesc = function(evidence){
		return Common.formatFileSize(evidence.iFileSize);
	};

}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qHistory = channel.objects.qHistory;
		
		qHistory.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("page")).scope();
			scope.$apply(function () {
				scope.language = lang;
			});
		});
		
		//绑定c++的回调函数
		qHistory.sigGetEvidence.connect(function(arr){
			var scope = angular.element(document.getElementById("page")).scope();
			
			scope.$apply(function () {
				scope.updateHistroies(arr);
			});
			
		});

		//添加存证记录
		qHistory.sigAddEvidence.connect(function(obj){
			var scope = angular.element(document.getElementById("page")).scope();
			
			scope.$apply(function () {
				scope.addEvidece(obj);
			});
		});

		//更新存证记录
		qHistory.sigUpdateEvidence.connect(function(hash, evidence, type){
			var scope = angular.element(document.getElementById("page")).scope();
				scope.$apply(function () {
					scope.updateEvidence(hash, evidence, type);
				});
		});
		
		
		qHistory.onHtmlReady();
	});
});