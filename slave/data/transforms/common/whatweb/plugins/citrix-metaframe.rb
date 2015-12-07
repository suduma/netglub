##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Citrix-Metaframe" do
author "Andrew Horton"
version "0.1"
description ""
examples %w|http://203.97.79.96|

=begin
<!--
---- WebInterface.htm
---- Copyright (c) 2000 - 2005 Citrix Systems, Inc. All Rights Reserved.
---- Web Interface (Build 45083)
-->
<SCRIPT LANGUAGE="JavaScript" TYPE="text/javascript">
<!--
window.location="/Citrix/MetaFrame";
// -->
=end



#


matches [
{:name=>"copyright", 
:probability=>100,
:regexp=>/Copyright \(c\) [\d]+ - [\d]+ Citrix Systems, Inc. All Rights Reserved./ },

{:name=>"window.location", 
:probability=>100,
:text=>'window.location="/Citrix/MetaFrame";' },

{:name=>"title", 
:probability=>100,
:text=>"<title>MetaFrame Presentation Server Log In</title>" },

]



end




