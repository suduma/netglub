##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "MikroTik" do
author "Andrew Horton"
version "0.1"
description "MikroTik router. Homepage: http://www.mikrotik.com"

# identifying strings
# <title>mikrotik routeros > administration</title>
# <div class="top">mikrotik routeros 3.20 configuration page</div>
# <div class="top">mikrotik routeros 2.9.27 configuration page</div>


matches [
{:name=>"title", 
:probability=>100,
:text=>"<title>mikrotik routeros > administration</title>"}
]


def passive
	m=[]
	
	if @body =~ /<div class="top">mikrotik routeros ([^ ]+) configuration page</
		version=@body.scan(/<div class="top">mikrotik routeros ([^ ]+) configuration page</)[0][0]
		m << {:probability=>100,:name=>"version",:version=>version} 
	end
	
	m
end

end


