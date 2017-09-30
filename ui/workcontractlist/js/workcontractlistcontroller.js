'use strict';

var stInfoApp = angular.module('WorkContractListApp', []);

stInfoApp.controller('workListCtrl', ['$scope', function($scope) {
    $scope.contracts = [];

    $scope.appendContract = function(obj){
        $scope.contracts.push(obj);
    };

    $scope.clear = function(){
        $scope.contracts.splice(0, $scope.contracts.length);
    };

    $scope.grab = function (id, rate) {
        if($scope.ifContractGrabbing(id)){
            return;
        }

        WorkContractQt.grab(id, rate);
    };

    $scope.GetLogoURI = function (job) {
        //return "images/logo144.png";
        var obj = null;
        obj = job.EmployerType == 1 ? job.EmployerUser : job.EmployerTeam;
        return Common.formatURIWithStamp(obj.LogoPicture, obj.LogoStamp);
    };

    //计算保障金可工作小时数
    $scope.calDepositeOfHours = function (job) {
        return Math.floor(job.EmployerDeposite / job.HourSalary);
    };

    //计算完成进度
    $scope.calExcuteRate = function (job) {
        //return 10.1;
        var localWorkSecs = WorkContractQt.getLocalWorkSecs(job.ID, job.InvalidTimeLength, job.MinWorkHoursUnitType);
        //var ret = job.EstimateExcuteHours * 100.0 / job.Hours;
        var ret = (job.EstimateExcuteHours * 3600 + localWorkSecs) * 100.0 / (job.Hours * 3600);
        
        return ret.toFixed(1);
    };

    $scope.calSumSalary = function(job){
        var sumSalary =  job.EstimateAmount;
        var ret = '' + sumSalary + '¥';
        return ret;
    };

    $scope.getSignDateTime = function(job){

        return job.SignContractTime;

        var dt = job.SignContractTime.split('-');
        if(dt instanceof Array && dt.length == 3){
            var year = dt[0];
            var month= dt[1];
            var day  = dt[2];
        }

        var str = '' + year + '年' + month + '月' + day;

        return str;
    };

    //计算当前单位时间内工作是否已完成
    $scope.getCurStatusDesc = function(job){
        var desc = '';

        var needwork = $scope.getNeedWorktime(job);
        if(needwork > 0){
            desc = '工作未完成';
        }else{
            desc = '工作已经完成';
        }

        var pre = '';
        pre = (job.MinWorkHoursUnitType == 1 ? '今天' : '本周');

        var ret = pre + desc;

        return ret; 
    };

    $scope.ifContractGrabbing = function(contractID){
        return WorkContractQt.ifContractGrabbing(contractID);
    };

    $scope.getAllWorktime = function(job){
        
        var type = (job.MinWorkHoursUnitType == 1 ? 1 : 2);
        var localSecs = WorkContractQt.getLocalWorkSecs(job.ID, job.InvalidTimeLength, type);

        var hours = localSecs / 3600;
        hours = hours + job.EstimateExcuteHoursCurrentUnit;
        hours = hours.toFixed(1);

        return hours;
    };

    $scope.getNeedWorktime = function(job){
        var worktimeOfHours = $scope.getAllWorktime(job);

        return job.MinWorkHoursPerUnit - worktimeOfHours;
    };


    $scope.showNoContractTip = false;

    $scope.calTotalTime = function(job){
        var totalSecs = job.TotalMinutes * 60 + WorkContractQt.getLocalTotalTime(job.ID);

        var ret = Common.formatSecond2String(totalSecs);

        return ret;
    };

    $scope.calTotalTimeToday = function(job){
        var serSecs = job.TotalMinutesInToday * 60;
        var locSecs = WorkContractQt.getLocalDurationTime(job.ID, 1);
        var ret = Common.formatSecond2String(serSecs + locSecs);

        return ret;
    };

    $scope.calTotalTimeYesterday = function(job){
        var serSecs = job.TotalMinutesInYesterday * 60;
        var locSecs = WorkContractQt.getLocalDurationTime(job.ID, 2);

        var ret = Common.formatSecond2String(serSecs + locSecs);

        return ret;
    };
    
}]);

//////////////////////////////
//qt 调用函数
function setShowNoContractTip(ifShow){
    var scope = angular.element(document.getElementById("news_box")).scope();
        scope.$apply(function () {
            scope.showNoContractTip = ifShow;
        });
}

function appendContract(contract) {
    var scope = angular.element(document.getElementById("news_box")).scope();
    scope.$apply(function () {
        scope.appendContract(contract);
    });
}

function clearContracts() {
    var scope = angular.element(document.getElementById("news_box")).scope();
    scope.$apply(function () {
        scope.clear();
    });
}

function appendContracts(json){
    var arr = $.parseJSON(json);
    if(arr instanceof  Array){
        arr.forEach(function(item){
            appendContract(item);
        });
    }
}

function refreshShow(){
    var scope = angular.element(document.getElementById("news_box")).scope();

    scope.$apply(function () {
        scope.$apply();
    });
}