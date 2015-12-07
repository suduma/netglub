##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "X-Powered-By" do
author "Andrew Horton"
version "0.1"
description "HTTP header, x-powered-by"
# identifying strings
# X-Powered-By: PHP/5.1.6

def passive
	m=[]
	
	unless @meta.nil?
		serverkey= %w|x-powered-by X-Powered-By|.map {|x| x if @meta.keys.include?(x) }.compact.first
		
		unless serverkey.nil?
			m << {:probability=>100,:name=>"x-powered-by string",:string=>@meta[serverkey]}
		end
	end
	m
end

end


