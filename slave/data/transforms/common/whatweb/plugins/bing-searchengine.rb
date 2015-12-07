##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##

Plugin.define "Bing-SearchEngine" do
author "Andrew Horton"
version "0.1"
description "Bing.com is Microsoft's search engine"
examples %w| www.bing.com |

#

matches [
{:name=>"curUrl=", 
:probability=>100,
:regexp=>/var curUrl="http:\\\/\\\/www.bing.com\\\/"/ },

{:name=>"meta content", 
:probability=>100,
:text=>'<meta content="Bing is a search engine that finds' }

]

end

