##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "WordPress" do
author "Andrew Horton"
version "0.2"
# identifying strings
# <meta name="generator" content="WordPress 2.5.1" />
# <meta name="generator" content="WordPress 2.6.5" />
# <meta name="generator" content="WordPress.com" />

matches [
{:name=>"meta generator tag with WordPress.com", 
:probability=>100,
:text=>"<meta name=\"generator\" content=\"WordPress.com\" />"},

{:name=>"powered by link",
:probability=>100,
:text=>"<a href=\"http://www.wordpress.com\">Powered by WordPress</a>"},

{:name=>"wp-content",
:probability=>75,
:regexp=>/"[^"]+\/wp-content\/[^"]+"/}
]


def passive
	m=[]
	
	if @body =~ /<meta name=\"generator\" content=\"WordPress[ ]?[0-9\.]+\"/
		version=@body.scan(/<meta name=\"generator\" content=\"(WordPress)[ ]?([0-9\.]+)\"/)[0][1]
		m << {:probability=>100,:name=>"meta generator tag",:version=>version} 
	end	
	m
end

# http://www.morningstarsecurity.com/readme.html
# aggressive plugin should get this file for the version


end

