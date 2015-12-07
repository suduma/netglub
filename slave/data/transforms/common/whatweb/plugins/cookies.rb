##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Cookies" do
author "Andrew Horton"
version "0.1"
description "Display the names of cookies in the HTTP headers. I have chosen not to record cookie values to save on space."

def passive
	m=[]
	found=[]
	
	unless @cookies.nil? or @cookies.empty?
		found=@cookies.map {|x| x.split("=")[0] }.sort.uniq		
		m << {:name=>"cookie names", :string=>found, :probability=>100}
	end

	m
end

end


