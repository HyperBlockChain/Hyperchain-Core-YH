'use strict';

(function($) {

    $(document).ready(function () {

        $("#reload2").click(function(){
            if(typeof WorkContractQt != 'undefined'){
                WorkContractQt.reload();    
            }else{
                alert("WorkContractQt not exist");
            }
        });
        
    });

})(jQuery);