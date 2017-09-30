'use strict';

var setNodeApp = angular.module('SetNodeApp', []);

setNodeApp.controller('setNodeCtrl', ['$scope', function($scope) {
	$scope.nodeFlag = {"flag": '节点标记', "ip":"192.1.1.1", "port":8888};
    $scope.servers = [];
	
	$scope.language = 1;
	
	//翻译初始化
	$scope.nodeFlagTitle = '节点标记';
	$scope.flag = '标记';
	$scope.addr = '地址';
	$scope.server = '超块链 BootStrap 服务器';
	$scope.pluginRight = '插件授权';
	$scope.popen = '开';
	$scope.pclose = '关';
	$scope.cacheSet = '缓存设置';
	$scope.chaindataCachePath = '链数据缓存路径';
	$scope.modify = '修改';
	$scope.backupAndRecover = '备份和恢复';
	$scope.backupSet = '备份设置';
	$scope.recoverSet = '恢复设置';
	$scope.devMode = '开发者模式';
	
	$scope.servers.push({"name":"server1", "ip":"192.168.0.1", "port": 433});
	$scope.servers.push({"name":"server2", "ip":"192.168.0.2", "port": 434});
	$scope.servers.push({"name":"server3", "ip":"192.168.0.3", "port": 435});
	$scope.servers.push({"name":"server4", "ip":"192.168.0.4", "port": 436});
	$scope.servers.push({"name":"server5", "ip":"192.168.0.5", "port": 437});
	
    $scope.appendServer = function(obj){
        $scope.servers.push(obj);
    };

    $scope.clear = function(){
        $scope.servers.splice(0, $scope.servers.length);
    };
	
	$scope.myAppOpen = 1;
	$scope.myApptoggle = function(){
		if($scope.myAppOpen == 0){
			$scope.myAppOpen = 1;
		}else{
			$scope.myAppOpen = 0;
		}
		
		qNodeSet.myAppOpen( $scope.myAppOpen == 1 ? true : false);
	}
	
	$scope.onLanguageChange = function(){
		if($scope.language == 0){
			$scope.nodeFlagTitle = 'NodeFlag';
			$scope.flag = 'Flag';
			$scope.addr = 'Address';
			$scope.server = 'HyperChain BootStrap Servers';
			$scope.pluginRight = 'PluginRight';
			$scope.popen = 'Open';
			$scope.pclose = 'Close';
			$scope.cacheSet = 'CacheSet';
			$scope.chaindataCachePath = 'ChaindataCachePath';
			$scope.modify = 'Modify';
			$scope.backupAndRecover = 'BackupAndRecover';
			$scope.backupSet = 'BackupSet';
			$scope.recoverSet = 'RecoverSet';
			$scope.devMode = 'DevMode';
		}else{
			$scope.nodeFlagTitle = '节点标记';
			$scope.flag = '标记';
			$scope.addr = '地址';
			$scope.server = '超块链 BootStrap 服务器';
			$scope.pluginRight = '插件授权';
			$scope.popen = '开';
			$scope.pclose = '关';
			$scope.cacheSet = '缓存设置';
			$scope.chaindataCachePath = '链数据缓存路径';
			$scope.modify = '修改';
			$scope.backupAndRecover = '备份和恢复';
			$scope.backupSet = '备份设置';
			$scope.recoverSet = '恢复设置';
			$scope.devMode = '开发者模式';
		}
	};
}]);

$(function(){
	//注册qt全局对象及回调
	new QWebChannel(qt.webChannelTransport, function(channel) {

		window.qNodeSet = channel.objects.qNodeSet;
		
		qNodeSet.sigTest.connect(function(param){
			alert(param);
		});
		
		qNodeSet.sigChangeLang.connect(function(lang){
			var scope = angular.element(document.getElementById("container")).scope();
			scope.$apply(function () {
				scope.language = lang;
				scope.onLanguageChange();
			});
		});
		
		qNodeSet.getLocalInfo(function(obj){
			var scope = angular.element(document.getElementById("container")).scope();
			scope.$apply(function () {
				scope.nodeFlag.flag = obj.info;
				scope.nodeFlag.ip = obj.ip;
				scope.nodeFlag.port = obj.port;
			});
		});
	});
	
	
	$(document.body).css({
	   "overflow-x":"hidden",
	   "overflow-y":"auto"
	 });
});

function callQTFunc(){
	qNodeSet.callQTFunc("hell qt, i am from html");
}

function showFramelessWnd(){
	qNodeSet.showFramelessWnd();
}