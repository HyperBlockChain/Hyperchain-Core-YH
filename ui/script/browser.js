'use strict';

var browserApp = angular.module('BrowserApp', []);

browserApp.controller('browserCtrl', ['$scope', function($scope) {
		
	 // {  
		// "iBlockNum": 0,
		// "iUseNodes": 0,
		// "iDataChainNum": 0,
		// "iChainNum": 0,
		// "iLongestChain": 0,
		// "tTime": 1234
		// //,"tFileInfo": {},
	// };
	$scope.sea = {"searchVal": ""};
	$scope.searchResult = [];
	
	$scope.showMore = false;
	
	$scope.confirmingNum = 0;
	
	$scope.language = 1;
	
	$scope.generalInfo = {
		"chainDataSize": "",					//链数据大小.这里为什么是string呢
		"tradesTotal": 0,					//交易总数
		"nodeCount": 0,						//全网节点数
		"unConfirmedReq": 0,					//未确认请求
		"myConnectedNode": 0,				//我链接的节点数
		"alternativeConsensusBlockTime": 0 	//备选共识块时间
	};
	
	//按钮相关的函数
	$scope.search = function(){
		//清空数据
		$scope.searchResult.splice(0, $scope.searchResult.length);
		
		//隐藏“更多”显示结果
		$scope.showMore = false;
		
		if($scope.sea.searchVal.length == 0){
			return;
		}

		qBrowser.search($scope.sea.searchVal);
	};
	
	$scope.showDetail = function(item){
		qBrowser.showDetail(item);
	};
	
	$scope.showMoreItem = function(){
		$scope.showMore = true;
	};
	
	$scope.checkAllNode = function(){
		
	};
	
	$scope.checkMyConnectNode = function(){
		
	};
	
	$scope.checkUnConfirmedReq = function(){
		
	};
	
	///////////////////////////////////////
	$scope.updateResult = function(obj){
		
	};
	
	$scope.updateGeneral = function(generalInfo){
		$scope.generalInfo = generalInfo;
		return;

		$scope.generalInfo.chainDataSize = generalInfo.chainDataSize;
		$scope.generalInfo.tradesTotal = generalInfo.tradesTotal;
		$scope.generalInfo.nodeCount = generalInfo.nodeCount;
		$scope.generalInfo.unConfirmedReq = generalInfo.unConfirmedReq;
		$scope.generalInfo.myConnectedNode = generalInfo.myConnectedNode;
		$scope.generalInfo.alternativeConsensusBlockTime = generalInfo.alternativeConsensusBlockTime;
	};
	
	$scope.getTimestampDesc = function(t){
		return Common.timeStamp(t);
	};
	
}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qBrowser = channel.objects.qBrowser;
		
		qBrowser.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("page")).scope();
			scope.$apply(function () {
				scope.language = lang;
			});
		});
		
		//绑定c++的回调函数
		qBrowser.sigUpdateGeneralInfo.connect(function(generalInfo){
			var scope = angular.element(document.getElementById("page")).scope();
			
            scope.$apply(function () {
                scope.updateGeneral(generalInfo);
            });
			
		});
		
		qBrowser.sigSearchResult.connect(function(arr){
			var scope = angular.element(document.getElementById("page")).scope();
			
			//删除测试数据
            // var o1 = {
            //     "iBlockNum": 1,
            //     "iUseNodes": 1,
            //     "iDataChainNum": 1,
            //     "iChainNum": 1,
            //     "iLongestChain": 1,
            //     "tTime": 1234
            // };
			
            // var tf = {
            //     "cFileName": 'xx.txt',
            //     "cCustomInfo": 'xxx',
            //     "cRightOwner": 'xxx',
            //     "cFileHash": 'xxx',
            //     "iFileState": 2,
            //     "tRegisTime": 123,
            //     "iFileSize": 123455
            // };
            // o1.tFileInfo = tf;
        			
            // var o2 = {
            //     "iBlockNum": 2,
            //     "iUseNodes": 2,
            //     "iDataChainNum": 2,
            //     "iChainNum": 2,
            //     "iLongestChain": 2,
            //     "tTime": 5678
            // };
            // o2.tFileInfo = tf;
			
            // arr.push(o1);
            // arr.push(o2);
			
			if(arr.length > 0){
				 scope.$apply(function () {
					for(var i = 0; i < arr.length; i++){
						scope.searchResult.push(arr[i]);
					}
				});

			}
		});
		
		qBrowser.onHtmlReady();
	});
});