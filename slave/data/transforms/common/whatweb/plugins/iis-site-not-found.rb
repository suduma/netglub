##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "IIS-SiteNotFound" do
author "Andrew Horton"
version "0.1"
description "Microsoft/IIS 5.0 default"
examples %w| webext.ccc.govt.nz |

matches [
{:name=>"Site Not Found", 
:probability=>100,
:version=>5,
:text=>"<html><head><title>Site Not Found</title></head>.<body>No web site is configured at this address.</body></html>"}
]


end
