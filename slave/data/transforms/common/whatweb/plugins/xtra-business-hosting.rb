##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "XtraBusinessHosting" do
author "Andrew Horton"
version "0.1"
description "Hosting at Xtra.co.nz"
examples %w| 210.54.223.97 |

#<title>Xtra Business: Web Hosting</title>

matches [
{:name=>"title", 
:probability=>100,
:regexp=>/<title>Xtra Business: Web Hosting<\/title>/ }
]

end

