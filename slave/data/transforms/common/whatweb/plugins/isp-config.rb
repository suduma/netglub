##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "ISPConfig" do
author "Andrew Horton"
version "0.1"
description "ISPConfig is a free, opensource hosting control panel"
examples %w| http://118.93.56.121/ |

#powered by <a HREF="http://www.ispconfig.org">ISPConfig</a>


matches [
{:name=>"powered by link", 
:probability=>100,
:text=>'powered by <a HREF="http://www.ispconfig.org">ISPConfig</a>' },

{:name=>"This IP address is shared...", 
:probability=>75,
:text=>'This IP address is shared. For access to the web site which you look for, enter its address instead of its IP.' }
]

end

