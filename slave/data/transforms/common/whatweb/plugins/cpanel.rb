##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "CPanel" do
author "Andrew Horton"
version "0.1"
description "homepage:www.cpanel.net"
examples %w| 202.191.40.29 |

# default apache + cpanel

# <title>cPanel&reg;</title>
# <div id="footer_images"><img src="sys_cpanel/images/powered_by.gif" />
# Apache is working on your cPanel<sup>&reg;</sup> and WHM&#8482; Server
# 

matches [
{:name=>"default title", 
:probability=>100,
:text=>"<title>cPanel&reg;</title>" },

{:name=>"footer images", 
:probability=>100,
:text=>'<div id="footer_images"><img src="sys_cpanel/images/powered_by.gif" />' },

{:name=>"is working", 
:probability=>100,
:text=>"Apache is working on your cPanel<sup>&reg;</sup> and WHM&#8482; Server" }
]



end



