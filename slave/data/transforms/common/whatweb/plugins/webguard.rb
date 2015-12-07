##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "WebGuard" do
author "Andrew Horton"
version "0.1"
description "security surveillance  homepage: http://novuscctv.com/"
examples %w| 124.198.140.159 144.131.54.78 www.dvronline.net |

# product is: © NOVUS Security
# NV-DVR5816/DVD

# <title>WebGuard Login</title>
# newWindow = window.open(addr + '.htm', 'WEBGUARD', 'width=1000,
# watchPort = 8016;


matches [
{:name=>"title", 
:probability=>100,
:regexp=>/<title>WebGuard Login<\/title>/ },

{:name=>"watchport", 
:probability=>75,
:regexp=>/watchPort = 8016;/ },

{:name=>"javascript", 
:probability=>75,
:regexp=>/newWindow = window.open\(addr \+ '.htm', 'WEBGUARD', 'width=1000,/ }



]

end


