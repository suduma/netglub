##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "OkiPBX" do
author "Andrew Horton"
version "0.1"
description "OKI PBX (phone exchange) http://www.oki.com/en/iptel/products/mxsx/maintenance.html"
examples %w| 125.236.233.225 |
#
# OKI PBX
# <APPLET CODE="DavisBar.class" ARCHIVE="ipstage.jar"
#   <PARAM NAME="servertype" value="ONLINE">
#    <PARAM NAME="systype"    value="OKI">


matches [
{:name=>"title", 
:probability=>100,
:regexp=>/<title>IPstageMaintenanceConsole(PBX)<\/title>/ },

{:name=>"applet code", 
:probability=>100,
:regexp=>/<APPLET CODE="DavisBar.class" ARCHIVE="ipstage.jar"/ },

{:name=>"param systype = OKI",
:probability=>100,
:regexp=>/<PARAM NAME="systype"    value="OKI">/ }
]

end



