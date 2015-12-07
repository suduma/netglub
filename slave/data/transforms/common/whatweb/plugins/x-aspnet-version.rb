##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "X-ASPNetVersion" do
author "Andrew Horton"
version "0.1"
description "HTTP header, x-aspnet-version"

def passive
	m=[]
	
	unless @meta.nil?
		serverkey= %w|x-aspnet-version X-AspNet-Version|.map {|x| x if @meta.keys.include?(x) }.compact.first
		
		unless serverkey.nil?
			m << {:probability=>100,:name=>"x-aspnet-version string",:version=>@meta[serverkey]}
		end
	end
	m
end

end


