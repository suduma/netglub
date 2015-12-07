##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "PoweredBy" do
author "Andrew Horton"
version "0.1"
description "Powered by xxx. This needs improvement to strip out <tags>."

def passive
	m=[]
 	if @body =~ /powered[ -]?by[ ]?([^ <>]+)/i
                title=@body.scan(/powered[ -]?by[ ]?([^ <>]+)/i)[0][0]
                m << {:probability=>100,:name=>"powered by xxx",:string=>title}
    end
    m
end

end

