'use strict';

var devNodeInfoApp = angular.module('DevNodeInfoApp', []);

devNodeInfoApp.controller('devNodeInfoCtrl', ['$scope', function($scope) {
			
	$scope.language = 1;
	$scope.hclog = '';
	
	$scope.title = '开发者模式-节点信息';
	$scope.myReg = '我的登记请求';
	$scope.recvRegReq = '收到的登记请求';
	$scope.noderun = '节点运行';
	$scope.dataTrans = '数据传输';
	$scope.storage = '存储';
	
	$scope.check = '查看';
	$scope.confirming = '待确认';
	$scope.confirmed = '已确认';
	
	$scope.total = '累计';
	$scope.refused = '被拒绝';
	
	//单独设置
	//节点信息
	$scope.connectedNodeCount = '连接节点数';		//
	$scope.totalRuntime = '累计运行时间';			//
	$scope.recvReq = '收到的请求';		//
	
	//数据传输
	$scope.sendRate = '发送速率';
	$scope.sendSize = '已发送';
	$scope.recvRate = '接收速率';
	$scope.recvSize = '已接收';
	
	//存储
	$scope.chainStorage = '链存储';
	$scope.cache = '缓冲区';
	$scope.clearCache = '清理缓存';
	$scope.memory = '内存消耗';
	
	$scope.onLanguageChange = function(){
		if($scope.language == 0){
			$scope.title = 'DevMode-NodeInfo';
			
			$scope.myReg = 'My Reg Req';			//我的登记请求
			$scope.recvRegReq = 'Recv Reg Req';		//收到的登记请求
			$scope.noderun = 'Node Run Info';		//节点运行
			$scope.dataTrans = 'Data Transfer';		//数据传输
			$scope.storage = 'Storage';				//存储
			
			
			$scope.check = 'Check';
			
			$scope.confirming = 'Confirming';
			$scope.confirmed = 'Confirmed';
			
			$scope.total = 'Total';
			$scope.refused = 'Refused';
			
			//单独设置
			//节点信息
			$scope.connectedNodeCount = 'CNC';		//连接节点数
			$scope.totalRuntime = 'TRT';			//累计运行时间
			$scope.recvReq = 'Recved Req';		//收到的请求
			
			//数据传输
			$scope.sendRate = 'SendRate';
			$scope.sendSize = 'SendSize';
			$scope.recvRate = 'RecvRate';
			$scope.recvSize = 'RecvSzie';
			
			//存储
			$scope.chainStorage = 'ChainStorage';
			$scope.cache = 'Cache';
			$scope.clearCache = 'ClearCache';
			$scope.memory = 'Memory';
		}else{
			$scope.title = '开发者模式-节点信息';
			
			$scope.myReg = '我的登记请求';
			$scope.recvRegReq = '收到的登记请求';
			$scope.noderun = '节点运行';
			$scope.dataTrans = '数据传输';
			$scope.storage = '存储';
			
			$scope.check = '查看';
			$scope.confirming = '待确认';
			$scope.confirmed = '已确认';
			
			$scope.total = '累计';
			$scope.refused = '被拒绝';
			
			//单独设置
			//节点信息
			$scope.connectedNodeCount = '连接节点数';		//
			$scope.totalRuntime = '累计运行时间';			//
			$scope.recvReq = '收到的请求';		//
			
			//数据传输
			$scope.sendRate = '发送速率';
			$scope.sendSize = '已发送';
			$scope.recvRate = '接收速率';
			$scope.recvSize = '已接收';
			
			//存储
			$scope.chainStorage = '链存储';
			$scope.cache = '缓冲区';
			$scope.clearCache = '清理缓存';
			$scope.memory = '内存消耗';
		}
	}
	
	$scope.testLog = function(){
		qDevMode.addLog('hello world');
	}
	
}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qDevMode = channel.objects.qDevMode;
		
		qDevMode.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("container")).scope();
			scope.$apply(function () {
				scope.language = lang;
				scope.onLanguageChange();
			});
		});
		
		qDevMode.sigAddLog.connect( function(log){
			var scope = angular.element(document.getElementById("container")).scope();
			scope.$apply(function () {
				scope.hclog += log + '\n';
				
				var obj = document.getElementById("hclog");  
				obj.scrollTop = obj.scrollHeight; // good  
			});
		});
		
		//qDevMode.onHtmlReady();
	});
});