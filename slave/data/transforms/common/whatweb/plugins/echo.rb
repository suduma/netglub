##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Echo" do
author "Andrew Horton"
version "0.1"
description "CMS - www.helloecho.com"
matches [
{:name=>"Powered by link",
:probability=>100,
:regexp=>/<a href="http:\/\/www.helloecho.com\/go\/?.*" target="_blank">powered by echo<\/a>/},
]
# identifying strings

end


