/****************************************************************************
**Copyright 2016-2018 hyperchain.net (Hyperchain)
**Distributed under the MIT software license, see the accompanying
**file COPYING or https://opensource.org/licenses/MIT.

**Permission is hereby granted, free of charge, to any person obtaining a copy of this 
**software and associated documentation files (the "Software"), to deal in the Software
**without restriction, including without limitation the rights to use, copy, modify, merge,
**publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
**to whom the Software is furnished to do so, subject to the following conditions:
**The above copyright notice and this permission notice shall be included in all copies or
**substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
**INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
**PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
**FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
**OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
**DEALINGS IN THE SOFTWARE.
*/


function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();
	
	if (systemInfo.productType === "windows") {
		component.addOperation("Execute", "@TargetDir@/bin/vcredist_x86.exe", "/quiet", "/norestart");
		component.addOperation("CreateShortcut", "@TargetDir@/bin/hc.exe", "@DesktopDir@/hc.lnk",
            "workingDirectory=@TargetDir@/bin", "@TargetDir@/bin/hc.exe",
            "iconId=1", "description=Open HyperChain Application");
    }
	else {
		var hc_script = "@TargetDir@/bin/hyperchain" ;
		component.addOperation("Execute", "touch", hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo '#!/bin/sh' > " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo '#http://www.hyperchain.net' >> " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'appname=hc' >> " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'dirname=`dirname $0`' >> " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'tmp=${dirname#?}' >> " + hc_script);
		
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'if [ \"${dirname%$tmp}\" != \"/\" ]; then' >> " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo '\tdirname=$PWD/$dirname' >> " + hc_script);
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'fi' >> " + hc_script);
		
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'LD_LIBRARY_PATH=$dirname:$dirname/lib' >> " + hc_script);
		
		component.addOperation("Execute", "/bin/sh", "-c", "echo 'export LD_LIBRARY_PATH' >> " + hc_script);
		
		component.addOperation("Execute", "/bin/sh", "-c", "echo '$dirname/$appname' >> " + hc_script);
						
		component.addOperation("Execute", "chmod", "a+x", hc_script);
		
		//Create Shortcut, At first let any user can access these directories.	
		component.addOperation("Execute", "chmod", "a+x", "@TargetDir@/bin");
		component.addOperation("Execute", "chmod", "a+x", "@TargetDir@/bin/lib");
		component.addOperation("Execute", "chmod", "a+x", "@TargetDir@/bin/resources");
		component.addOperation("Execute", "chmod", "a+x", "@TargetDir@/bin/translations");
		component.addOperation("Execute", "chmod", "a+x", "@TargetDir@/bin/translations/qtwebengine_locales");
		
		component.addOperation("Execute", "chmod", "-R", "a+x", "@TargetDir@/bin/plugins");
		component.addOperation("CreateDesktopEntry", 
								"/usr/share/applications/hyperchain.desktop", 
								"Type=Application\nExec=@TargetDir@/bin/hyperchain\nName=Hyperchain\nGenericName=Hyperchain\nIcon=@TargetDir@/bin/logo.ico\nTerminal=false\nCategories=Development;"
							  );
		
		
    }
}

