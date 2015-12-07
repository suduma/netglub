##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "HTTPServer" do
author "Andrew Horton"
version "0.1"
description "HTTP Server strings"
# identifying strings
#< Server: Apache/2.2.6 (Fedora)

def passive
	m=[]
	
	unless @meta.nil?
		server=nil	
		server=@meta["server"] if @meta.keys.include?("server")
		server=@meta["Server"] if @meta.keys.include?("Server")
	
		unless server.nil?		
			m << {:probability=>100,:name=>"server string",:string=>server}
		end
	end
	
	m
end

end


