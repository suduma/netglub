##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "MD5" do
author "Andrew Horton"
version "0.1"
description "MD5 sum of html body. Useful to find matching pages"

#

def passive
	m=[]
	
	unless @body.nil?
#	    m << {:probability=>100,:name=>"page title",:string=>Digest::SHA256.hexdigest(@body)}
	    m << {:probability=>100,:name=>"page title",:string=>Digest::MD5.hexdigest(@body)}
	end
 	m
end

end

