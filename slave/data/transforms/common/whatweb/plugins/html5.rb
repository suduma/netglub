##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "HTML5" do
author "Andrew Horton"
version "0.1"
description "HTML version 5, detected by the doctype declaration"


examples %w| www.rubious.co.uk phatdish.tumblr.com|


matches [
{:name=>"html5 doctype declaration",
:probability=>100,
:regexp=>/<!DOCTYPE html>/i}
]

end


