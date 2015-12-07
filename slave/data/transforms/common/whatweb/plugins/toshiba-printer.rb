##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "ToshibaPrinter" do
author "Andrew Horton"
version "0.1"
description "Toshiba printer Top Access"
examples %w| http://203.175.112.6/ |

# <TITLE CLASS="clsTitle1">TopAccess</title>
# Server: TOSHIBA TEC CORPORATION

matches [
{:name=>"title", 
:probability=>100,
:regexp=>/<TITLE CLASS="clsTitle1">TopAccess<\/title>/ }
]

end

