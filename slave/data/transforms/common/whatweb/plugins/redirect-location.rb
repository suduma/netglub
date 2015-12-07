##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "RedirectLocation" do
author "Andrew Horton"
version "0.1"
description "HTTP Server string location. used with http-status 301 and 302"

def passive
	m=[]
	
	unless @meta.nil?
		location=nil	
		location=@meta["location"] if @meta.keys.include?("location")
	
		unless location.nil?
			m << {:probability=>100,:name=>"location",:string=>location}
		end
	end
	
	m
end

end


